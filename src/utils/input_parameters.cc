// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "utils/input_parameters.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <numbers>
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "utils/errors.h"

namespace kingfisher {
namespace {

// ryml aborts by default. These handlers turn its errors into
// kingfisher::Error. They are installed on the Tree rather than globally, so
// nothing outside this file is affected.
std::string ToString(ryml::csubstr s) { return std::string(s.str, s.len); }

[[noreturn]] void ThrowBasic(ryml::csubstr msg, const ryml::ErrorDataBasic&,
                             void*) {
  throw Error("YAML error: " + ToString(msg));
}

[[noreturn]] void ThrowParse(ryml::csubstr msg,
                             const ryml::ErrorDataParse& data, void*) {
  // ryml counts lines from 0 and reports npos when the line is unknown.
  const std::string line =
      data.ymlloc.line == ryml::npos
          ? ""
          : " at line " + std::to_string(data.ymlloc.line + 1);
  throw Error("YAML parse error" + line + ": " + ToString(msg));
}

[[noreturn]] void ThrowVisit(ryml::csubstr msg, const ryml::ErrorDataVisit&,
                             void*) {
  throw Error("YAML input error (missing or malformed key): " + ToString(msg));
}

ryml::Callbacks ThrowingCallbacks() {
  ryml::Callbacks callbacks = ryml::get_callbacks();
  callbacks.set_error_basic(ThrowBasic);
  callbacks.set_error_parse(ThrowParse);
  callbacks.set_error_visit(ThrowVisit);
  return callbacks;
}

// YAML spellings of each enum. A table is added together with the block that
// reads it.
template <typename E>
using Entry = std::pair<std::string_view, E>;

constexpr std::array kRunModeNames = {
    Entry<RunMode>{"fixed_source", RunMode::FixedSource}};

constexpr std::array kBoundaryTypes = {
    Entry<BoundaryType>{"vacuum", BoundaryType::Vacuum},
    Entry<BoundaryType>{"reflective", BoundaryType::Reflective}};

constexpr std::array kSENodePlacementStrats = {Entry<SENodePlacement>{
    "gauss_lobatto_legendre", SENodePlacement::GaussLobattoLegendre}};

constexpr std::array kOutputFormats = {
    Entry<OutputFormat>{"vtkhdf", OutputFormat::VTKHDF}};

constexpr std::array kExportValues = {
    Entry<ExportValues>{"scalar_flux", ExportValues::ScalarFlux},
    Entry<ExportValues>{"angular_flux", ExportValues::AngularFlux}};

constexpr std::array kSourceTypes = {
    Entry<SourceType>{"volumetric", SourceType::Volumetric}};

constexpr std::array kAngularModels = {
    Entry<AngularModel>{"discrete_ordinates", AngularModel::DiscreteOrdinates}};

constexpr std::array kSphericalQuadTypes = {Entry<SphericalQuadType>{
    "tensor_product", SphericalQuadType::TensorProduct}};

constexpr std::array kQuadSets = {
    Entry<QuadSet>{"gauss_legendre", QuadSet::GaussLegendre},
    Entry<QuadSet>{"trapezoid", QuadSet::Trapezoid}};

// The name to use for `node` in messages: its key, or for a list entry the key
// of the list it is in.
std::string KeyName(const ryml::ConstNodeRef& node) {
  if (node.has_key()) return ToString(node.key());
  if (node.has_parent() && node.parent().has_key()) {
    return ToString(node.parent().key());
  }
  return "value";
}

// Returns the node at `path` below `node`, e.g. {"problem", "mode"}. ryml's
// operator[] does not fail for a missing key, it returns an unreadable node
// that is undefined behavior to use, so every lookup goes through here. Throws
// an Error naming the full path if a key is missing.
ryml::ConstNodeRef Require(const ryml::ConstNodeRef& node,
                           std::initializer_list<std::string_view> path) {
  ryml::ConstNodeRef current = node;
  std::string full_path;
  for (const std::string_view key : path) {
    full_path += (full_path.empty() ? "" : ".") + std::string(key);
    if (current.is_map()) {
      current = current.find_child(ryml::csubstr(key.data(), key.size()));
    } else {
      current = ryml::ConstNodeRef();
    }
    if (!current.readable()) {
      // A list entry has no key of its own: name the list it is in.
      const bool in_list =
          !node.has_key() && node.has_parent() && node.parent().has_key();
      throw Error(
          "Missing required key '" + full_path + "'" +
          (in_list ? " in '" + ToString(node.parent().key()) + "'" : "") + ".");
    }
  }
  return current;
}

// Throws unless `node` is a YAML list.
void RequireSequence(const ryml::ConstNodeRef& node) {
  if (!node.is_seq()) throw Error("'" + KeyName(node) + "' must be a list.");
}

// Returns the enumerator whose YAML spelling is the scalar in `node`.
// Throws an Error that lists the available spellings if there is no match.
template <typename E, std::size_t N>
E ParseEnum(const ryml::ConstNodeRef& node,
            const std::array<Entry<E>, N>& table) {
  const std::string key = KeyName(node);
  if (!node.has_val()) throw Error("'" + key + "' must be a single value.");

  const std::string_view text(node.val().str, node.val().len);
  for (const auto& [name, enumerator] : table) {
    if (name == text) return enumerator;
  }

  std::string available;
  for (const auto& entry : table) {
    available += (available.empty() ? "" : ", ") + std::string(entry.first);
  }
  throw Error("Unknown " + key + " '" + std::string(text) +
              "'. Available: " + available + ".");
}

void ReadProblem(const ryml::ConstNodeRef& root, ProblemParams& problem) {
  Require(root, {"problem", "name"}).load(&problem.name);
  problem.run_mode =
      ParseEnum(Require(root, {"problem", "mode"}), kRunModeNames);
}

void ReadMesh(const ryml::ConstNodeRef& root, MeshParams& mesh) {
  Require(root, {"mesh", "filename"}).load(&mesh.filename);
}

// Reads a quadrature set from `node`. The input file does not give the
// integration bounds, so the caller passes them in for the set's role.
void ReadQuadSet(const ryml::ConstNodeRef& node, double min, double max,
                 QuadSetParams& quad) {
  quad.type = ParseEnum(Require(node, {"type"}), kQuadSets);
  Require(node, {"n_directions"}).load(&quad.n_points);
  KingfisherError(quad.n_points == 0, "'n_directions' must be at least 1.");
  quad.min = min;
  quad.max = max;
}

void ReadAngular(const ryml::ConstNodeRef& root, AngularParams& angular) {
  angular.model =
      ParseEnum(Require(root, {"angular", "model"}), kAngularModels);
  angular.quad_type =
      ParseEnum(Require(root, {"angular", "type"}), kSphericalQuadTypes);

  // Which keys are required depends on the quadrature type. Each type gets its
  // own case, and the compiler flags a type that is not handled here.
  switch (angular.quad_type) {
    case SphericalQuadType::TensorProduct:
      ReadQuadSet(Require(root, {"angular", "polar_quad_set"}), -1.0, 1.0,
                  angular.polar_quad_set);
      ReadQuadSet(Require(root, {"angular", "azimuthal_quad_set"}), 0.0,
                  2.0 * M_PI, angular.azimuthal_quad_set);
      break;
  }
}

void ReadSpatial(const ryml::ConstNodeRef& root, SpatialParams& spatial) {
  spatial.se_node_placement = ParseEnum(
      Require(root, {"spatial", "node_placement"}), kSENodePlacementStrats);
  Require(root, {"spatial", "nodes_per_axis"}).load(&spatial.nodes_per_axis);
}

void ReadSourceIter(const ryml::ConstNodeRef& root,
                    SourceIterParams& si_params) {
  Require(root, {"solver", "source_iterations", "tolerance"})
      .load(&si_params.tolerance);
  Require(root, {"solver", "source_iterations", "max_iterations"})
      .load(&si_params.max_iterations);
}

void ReadSolver(const ryml::ConstNodeRef& root, SolverParams& solver) {
  ReadSourceIter(root, solver.source_iter_params);
}

void ReadOutput(const ryml::ConstNodeRef& root, OutputParams& output) {
  output.output_format =
      ParseEnum(Require(root, {"output", "format"}), kOutputFormats);

  const ryml::ConstNodeRef values = Require(root, {"output", "export_values"});
  RequireSequence(values);
  for (const ryml::ConstNodeRef value : values.children()) {
    output.export_values.push_back(ParseEnum(value, kExportValues));
  }
}

void ReadBoundaries(const ryml::ConstNodeRef& root,
                    std::vector<BoundaryParams>& boundaries) {
  const ryml::ConstNodeRef list = Require(root, {"boundary_conditions"});
  RequireSequence(list);
  for (const ryml::ConstNodeRef entry : list.children()) {
    BoundaryParams& boundary = boundaries.emplace_back();
    Require(entry, {"name"}).load(&boundary.name);
    boundary.type = ParseEnum(Require(entry, {"type"}), kBoundaryTypes);
  }
}

// Flattens the YAML `scatter_xs[moment][from][to]` into one vector in that
// order (see MaterialParams::scatter_matrix). Every moment must be a
// `num_groups` x `num_groups` matrix.
std::vector<double> FlattenScatter(
    const std::vector<std::vector<std::vector<double>>>& scatter,
    std::size_t num_groups, const std::string& material) {
  const std::string error =
      "Material '" + material + "': each scatter_xs moment must be a " +
      std::to_string(num_groups) + " x " + std::to_string(num_groups) +
      " matrix (one row per total_xs group).";
  std::vector<double> flat;
  flat.reserve(scatter.size() * num_groups * num_groups);
  for (const auto& moment : scatter) {
    KingfisherError(moment.size() != num_groups, error);
    for (const auto& from_group : moment) {
      KingfisherError(from_group.size() != num_groups, error);
      flat.insert(flat.end(), from_group.begin(), from_group.end());
    }
  }
  return flat;
}

void ReadMaterials(const ryml::ConstNodeRef& root,
                   std::vector<MaterialParams>& materials) {
  const ryml::ConstNodeRef list = Require(root, {"materials"});
  RequireSequence(list);
  for (const ryml::ConstNodeRef entry : list.children()) {
    MaterialParams& material = materials.emplace_back();
    Require(entry, {"name"}).load(&material.name);
    Require(entry, {"total_xs"}).load(&material.total_xs);

    std::vector<std::vector<std::vector<double>>> scatter;
    Require(entry, {"scatter_xs"}).load(&scatter);
    material.scatter_matrix =
        FlattenScatter(scatter, material.total_xs.size(), material.name);
  }
}

// The `sources` block is optional.
void ReadSources(const ryml::ConstNodeRef& root,
                 std::vector<SourceParams>& sources) {
  if (!root.has_child("sources")) return;
  const ryml::ConstNodeRef list = Require(root, {"sources"});
  RequireSequence(list);
  for (const ryml::ConstNodeRef entry : list.children()) {
    SourceParams& source = sources.emplace_back();
    Require(entry, {"name"}).load(&source.name);
    source.type = ParseEnum(Require(entry, {"type"}), kSourceTypes);
    Require(entry, {"expression"}).load(&source.expression);
  }
}

}  // namespace

InputParameters ParseInputString(std::string yaml) {
  // The tree holds views into `yaml`, so `yaml` must outlive it.
  ryml::Tree tree(ThrowingCallbacks());
  ryml::parse_in_place(ryml::to_substr(yaml), &tree);
  ryml::ConstNodeRef root = tree.crootref();
  if (root.is_stream()) {
    // The file starts with `---`, so the root holds the documents.
    KingfisherError(root.num_children() != 1,
                    "Input file must contain exactly one YAML document.");
    root = root.first_child();
  }

  int version = -1;
  Require(root, {"version"}).load(&version);
  KingfisherError(version != 0, "Input file must be version 0.");

  InputParameters input_params;
  ReadProblem(root, input_params.problem_params);
  ReadMesh(root, input_params.mesh_params);
  ReadBoundaries(root, input_params.boundaries);
  ReadMaterials(root, input_params.materials);
  ReadSources(root, input_params.sources);
  ReadAngular(root, input_params.angular_params);
  ReadSpatial(root, input_params.spatial_params);
  ReadSolver(root, input_params.solver_params);
  ReadOutput(root, input_params.output_params);

  CheckInputParameters(input_params);

  return input_params;
}

InputParameters ReadInputFile(const std::string& filename) {
  std::ifstream input_file(filename, std::ios::binary);
  KingfisherError(!input_file,
                  "Failed to open yaml input file '" + filename + "'.");

  std::stringstream buffer;
  buffer << input_file.rdbuf();

  try {
    return ParseInputString(std::move(buffer.str()));
  } catch (const Error& e) {
    throw Error(filename + ": " + e.what());
  }
}

namespace {

bool IsNameStart(char c) { return c >= 'a' && c <= 'z'; }
bool IsNameChar(char c) {
  return IsNameStart(c) || (c >= '0' && c <= '9') || c == '_';
}

// Names are stored in the mesh as 'boundary:<name>' and 'region:<name>', so
// they cannot contain a colon.
void CheckName(std::string_view block, const std::string& name) {
  bool valid = !name.empty() && IsNameStart(name.front());
  for (const char c : name) valid = valid && IsNameChar(c);
  KingfisherError(!valid, "Invalid name '" + name + "' in '" +
                              std::string(block) +
                              "'. Names must start with a lowercase letter and "
                              "contain only lowercase letters, digits, and "
                              "underscores.");
}

// Checks every name in `entries` and that none is repeated.
template <typename T>
void CheckNames(std::string_view block, const std::vector<T>& entries) {
  std::unordered_set<std::string_view> seen_names;
  for (const T& entry : entries) {
    CheckName(block, entry.name);
    KingfisherError(
        !seen_names.insert(entry.name).second,
        "Duplicate name '" + entry.name + "' in '" + std::string(block) + "'.");
  }
}

// NaN compares false against everything, so test for the valid range instead
// of the invalid one.
void CheckFiniteNonNegative(const std::string& what, double value) {
  KingfisherError(
      !(std::isfinite(value) && value >= 0.0),
      what + " = " + std::to_string(value) + " must be finite and >= 0.");
}

}  // namespace

void CheckInputParameters(const InputParameters& input_parameters) {
  CheckBoundaryParams(input_parameters.boundaries);
  CheckMaterialParams(input_parameters.materials);
  CheckSourceParams(input_parameters.sources);
  CheckSpatialParams(input_parameters.spatial_params);
  CheckSolverParams(input_parameters.solver_params);

  // The materials fix the number of energy groups: sources need one expression
  // for each.
  if (input_parameters.materials.empty()) return;
  const std::size_t num_groups =
      input_parameters.materials.front().total_xs.size();
  for (const SourceParams& source : input_parameters.sources) {
    KingfisherError(source.expression.size() != num_groups,
                    "Source '" + source.name + "' has " +
                        std::to_string(source.expression.size()) +
                        " expressions, but the materials have " +
                        std::to_string(num_groups) +
                        " energy groups. One expression per group is "
                        "required.");
  }
}

void CheckBoundaryParams(const std::vector<BoundaryParams>& boundary_params) {
  CheckNames("boundary_conditions", boundary_params);
}

void CheckMaterialParams(const std::vector<MaterialParams>& material_params) {
  CheckNames("materials", material_params);

  for (const MaterialParams& material : material_params) {
    const std::string prefix = "Material '" + material.name + "': ";
    const std::size_t num_groups = material.total_xs.size();
    KingfisherError(num_groups == 0,
                    prefix + "total_xs must have at least one group.");
    KingfisherError(material.scatter_matrix.empty(),
                    prefix + "scatter_xs must have at least one moment.");

    for (std::size_t group = 0; group < num_groups; ++group) {
      CheckFiniteNonNegative(prefix + "total_xs[" + std::to_string(group) + "]",
                             material.total_xs[group]);
    }

    // scatter_matrix is [moment][from][to], flattened (see MaterialParams).
    for (std::size_t i = 0; i < material.scatter_matrix.size(); ++i) {
      CheckFiniteNonNegative(prefix + "scatter_xs[" +
                                 std::to_string(i / (num_groups * num_groups)) +
                                 "][" +
                                 std::to_string((i / num_groups) % num_groups) +
                                 "][" + std::to_string(i % num_groups) + "]",
                             material.scatter_matrix[i]);
    }

    KingfisherError(
        num_groups != material_params.front().total_xs.size(),
        prefix + "has " + std::to_string(num_groups) + " energy groups, but '" +
            material_params.front().name + "' has " +
            std::to_string(material_params.front().total_xs.size()) +
            ". All materials must have the same number of groups.");
  }
}

void CheckSourceParams(const std::vector<SourceParams>& source_params) {
  CheckNames("sources", source_params);

  for (const SourceParams& source : source_params) {
    const std::string prefix = "Source '" + source.name + "': ";
    KingfisherError(source.expression.empty(),
                    prefix + "expression must have at least one group.");
    for (std::size_t group = 0; group < source.expression.size(); ++group) {
      KingfisherError(source.expression[group].empty(),
                      prefix + "expression[" + std::to_string(group) +
                          "] must not be empty.");
    }
  }
}

void CheckSpatialParams(const SpatialParams& spatial_params) {
  KingfisherError(
      spatial_params.nodes_per_axis < 2,
      "`spatial.nodes_per_axis` is " +
          std::to_string(spatial_params.nodes_per_axis) +
          ", but at least 2 nodes per axis are required per spectral element.");
}

void CheckSolverParams(const SolverParams& solver_params) {
  const SourceIterParams& source_iter = solver_params.source_iter_params;
  KingfisherError(
      !(std::isfinite(source_iter.tolerance) && source_iter.tolerance > 0.0),
      "`solver.source_iterations.tolerance` = " +
          std::to_string(source_iter.tolerance) +
          " must be finite and greater than 0.");
  KingfisherError(source_iter.max_iterations < 1,
                  "`solver.source_iterations.max_iterations` must be at "
                  "least 1.");
}

}  // namespace kingfisher
