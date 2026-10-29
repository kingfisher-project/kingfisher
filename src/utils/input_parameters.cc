// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "utils/input_parameters.h"

#include <array>
#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <sstream>
#include <string>
#include <string_view>
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
    Entry<RunMode>{"fixed_source", RunMode::kFixedSource}};

constexpr std::array kBoundaryTypes = {
    Entry<BoundaryType>{"vacuum", BoundaryType::kVacuum},
    Entry<BoundaryType>{"reflective", BoundaryType::kReflective}};

constexpr std::array kSENodePlacementStrats = {Entry<SENodePlacement>{
    "gauss_lobatto_legendre", SENodePlacement::kGaussLobattoLegendre}};

constexpr std::array kOutputFormats = {
    Entry<OutputFormat>{"vtkhdf", OutputFormat::kVTKHDF}};

constexpr std::array kExportValues = {
    Entry<ExportValues>{"scalar_flux", ExportValues::kScalarFlux},
    Entry<ExportValues>{"angular_flux", ExportValues::kAngularFlux}};

constexpr std::array kSourceTypes = {
    Entry<SourceType>{"volumetric", SourceType::kVolumetric}};

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
  ReadSpatial(root, input_params.spatial_params);
  ReadSolver(root, input_params.solver_params);
  ReadOutput(root, input_params.output_params);

  return input_params;
}

InputParameters ReadInputFile(const std::string& filename) {
  std::ifstream input_file(filename, std::ios::binary);
  KingfisherError(!input_file,
                  "Failed to open yaml input file '" + filename + "'.");

  std::stringstream buffer;
  buffer << input_file.rdbuf();

  try {
    return ParseInputString(buffer.str());
  } catch (const Error& e) {
    throw Error(filename + ": " + e.what());
  }
}

}  // namespace kingfisher
