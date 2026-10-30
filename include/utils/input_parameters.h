// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_UTILS_INPUT_PARAMETERS_H_
#define KINGFISHER_UTILS_INPUT_PARAMETERS_H_

#include <cstddef>
#include <string>
#include <vector>

#include "utils/enums.h"

namespace kingfisher {

/**
 * @brief `problem` block parameters.
 *
 */
struct ProblemParams {
  /// @brief Name of the problem
  std::string name;

  /// @brief Run mode of simulation
  RunMode run_mode = RunMode::FixedSource;
};

/**
 * @brief `mesh` block parameters.
 *
 */
struct MeshParams {
  std::string filename;
};

/**
 * @brief Individual boundaries.
 *
 */
struct BoundaryParams {
  /// @brief Must match the prefix-free boundary group name in the mesh.
  std::string name;

  /// @brief Boundary type
  BoundaryType type = BoundaryType::Vacuum;
};

/**
 * @brief Individual material properties.
 *
 */
struct MaterialParams {
  std::string name;

  /// @brief Total cross section per group in 1/cm, ordered highest energy to
  /// lowest.
  std::vector<double> total_xs;

  /// @brief Legendre scattering moments in 1/cm, flattened from the YAML
  /// `scatter_xs[moment][from_group][to_group]` in that order (row-major), so
  /// the entry is `scatter_matrix[(moment * G + from) * G + to]` with `G` =
  /// `total_xs.size()`. Holds `G * G` entries per listed moment.
  std::vector<double> scatter_matrix;
};

/**
 * @brief Individual source properties.
 *
 */
struct SourceParams {
  std::string name;
  SourceType type = SourceType::Volumetric;

  /// @brief Source strength per group in n/cm^3/s/str, as expressions in terms
  /// of position, ordered highest energy to lowest.
  std::vector<std::string> expression;
};

/**
 * @brief `source_iterations` sub-block parameters.
 *
 */
struct SourceIterParams {
  /// @brief Convergence tolerance for the source iteration.
  double tolerance = 1.0e-8;

  /// Maximum number of source iterations.
  unsigned int max_iterations = 100;
};

/**
 * @brief Parameters of a one-dimensional quadrature set.
 *
 */
struct QuadSetParams {
  /// @brief Type of quadrature set
  QuadSet type = QuadSet::GaussLegendre;

  /// @brief Number of points in set (`n_directions` in the YAML)
  std::size_t n_points = 0;

  /// @brief Minimum value (integrate from value). Set by the reader from the
  /// set's role, not read from the input file.
  double min = 0.0;

  /// @brief Maximum value (integrate to value). Set by the reader from the
  /// set's role, not read from the input file.
  double max = 0.0;
};

/**
 * @brief `angular` block parameters.
 *
 */
struct AngularParams {
  /// @brief Angular treatment model
  AngularModel model = AngularModel::DiscreteOrdinates;

  /// @brief Type of spherical quadrature set
  SphericalQuadType quad_type = SphericalQuadType::TensorProduct;

  /// @brief Polar quadrature set over the polar cosine, [-1, 1]. Only used when
  /// `quad_type` is `TensorProduct`.
  QuadSetParams polar_quad_set;

  /// @brief Azimuthal quadrature set over [0, 2*pi]. Only used when `quad_type`
  /// is `TensorProduct`.
  QuadSetParams azimuthal_quad_set;
};

/**
 * @brief `spatial` block parameters.
 *
 */
struct SpatialParams {
  /// @brief Node placement strategy within spectral elements
  SENodePlacement se_node_placement = SENodePlacement::GaussLobattoLegendre;

  /// @brief Number of nodes along each axis of an element (at least 2).
  std::size_t nodes_per_axis = 2;
};

/**
 * @brief `solver` block parameters.
 *
 */
struct SolverParams {
  /// @brief Source iteration parameters
  SourceIterParams source_iter_params;
};

/**
 * @brief `output` block parameters.
 *
 */
struct OutputParams {
  /// @brief Output format
  OutputFormat output_format = OutputFormat::VTKHDF;

  /// @brief Values to be exported at the end of the simulation
  std::vector<ExportValues> export_values;
};

/**
 * @brief Input parameters for simulation from the user.
 *
 */
struct InputParameters {
  /// @brief Problem parameters
  ProblemParams problem_params;

  /// @brief Mesh parameters
  MeshParams mesh_params;

  /// @brief Set of all materials in simulation
  std::vector<MaterialParams> materials;

  /// @brief Set of all sources in simulation
  std::vector<SourceParams> sources;

  /// @brief Set of all boundaries in simulation
  std::vector<BoundaryParams> boundaries;

  /// @brief Angular discretization parameters
  AngularParams angular_params;

  /// @brief Spatial spectral element parameters
  SpatialParams spatial_params;

  /// @brief Solver parameters
  SolverParams solver_params;

  /// @brief Output parameters
  OutputParams output_params;
};

/**
 * @brief Parses YAML input text.
 *
 * Only checks that required keys are present and enum spellings are known;
 * it does not validate the values.
 *
 * @param yaml The YAML input. Taken by value because the parser edits it.
 * @return The parsed input parameters.
 * @throws Error If the text is not valid YAML or is missing a required key.
 */
InputParameters ParseInputString(std::string yaml);

/**
 * @brief Reads and parses a YAML input file.
 *
 * @param filename Path to the YAML input file.
 * @return The parsed input parameters.
 * @throws Error If the file cannot be opened or ParseInputString() fails. The
 *   message is prefixed with `filename`.
 */
InputParameters ReadInputFile(const std::string& filename);

/**
 * @brief Checks the values of a parsed input.
 *
 * ParseInputString() calls this after reading every block, so a returned
 * InputParameters has already passed it. Run it again after editing an
 * InputParameters by hand.
 *
 * Checks each block (see the Check*Params() functions below), then checks that
 * every source has one expression for each energy group of the materials.
 *
 * @param input_parameters The parameters to check.
 * @throws Error If any value is invalid. The message names the offending block
 *   and entry.
 */
void CheckInputParameters(const InputParameters& input_parameters);

/**
 * @brief Checks the `boundary_conditions` entries.
 *
 * Names must be unique and start with a lowercase letter followed by lowercase
 * letters, digits, and underscores (the mesh stores them as
 * `boundary:<name>`).
 *
 * @param boundary_params The boundaries to check.
 * @throws Error If a name is invalid or repeated.
 */
void CheckBoundaryParams(const std::vector<BoundaryParams>& boundary_params);

/**
 * @brief Checks the `materials` entries.
 *
 * Names must be valid and unique (see CheckBoundaryParams()). Each material
 * needs at least one energy group and one scattering moment, and every cross
 * section must be finite and non-negative. All materials must have the same
 * number of energy groups.
 *
 * @param material_params The materials to check.
 * @throws Error If any of the above is violated.
 */
void CheckMaterialParams(const std::vector<MaterialParams>& material_params);

/**
 * @brief Checks the `sources` entries.
 *
 * Names must be valid and unique (see CheckBoundaryParams()). Each source needs
 * at least one expression and no expression may be empty. The expression count
 * is compared with the materials' group count by CheckInputParameters().
 *
 * @param source_params The sources to check.
 * @throws Error If any of the above is violated.
 */
void CheckSourceParams(const std::vector<SourceParams>& source_params);

/**
 * @brief Checks the `spatial` block.
 *
 * Gauss-Lobatto-Legendre elements need at least 2 nodes per axis, because the
 * element end points are nodes.
 *
 * @param spatial_params The spatial parameters to check.
 * @throws Error If `nodes_per_axis` is less than 2.
 */
void CheckSpatialParams(const SpatialParams& spatial_params);

/**
 * @brief Checks the `solver` block.
 *
 * The tolerance must be finite and positive, and at least one iteration must be
 * allowed.
 *
 * @param solver_params The solver parameters to check.
 * @throws Error If the tolerance or iteration limit is invalid.
 */
void CheckSolverParams(const SolverParams& solver_params);

}  // namespace kingfisher

#endif  // KINGFISHER_UTILS_INPUT_PARAMETERS_H_
