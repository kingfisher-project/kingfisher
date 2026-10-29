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
 * @brief `angular` block parameters.
 *
 */
struct AngularParams {
  /// @brief Angular treatment model
  AngularModel model;

  /// @brief Type of spherical quadrature set
  SphericalQuadType quad_type;

  /// @brief Polar quadrature set
  QuadSetParams polar_quad_set;

  /// @brief Azimuthal quadrature set
  QuadSetParams azimuthal_quad_set;
};

/**
 * @brief Quadrature set parameters
 *
 */
struct QuadSetParams {
  /// @brief Type of quadrature set
  QuadSet type;

  /// @brief Number of points in set
  size_t n_points;

  /// @brief Minimum value (integrate from value)
  double min;

  /// @brief Maximum value (integrate to value)
  double max;
};

/**
 * @brief `spatial` block parameters.
 *
 */
struct SpatialParams {
  /// @brief Node placement strategy within spectral elements
  SENodePlacement se_node_placement = SENodePlacement::GaussLobattoLegendre;

  /// @brief Number of nodes along each axis of an element (at least 1).
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

}  // namespace kingfisher

#endif  // KINGFISHER_UTILS_INPUT_PARAMETERS_H_
