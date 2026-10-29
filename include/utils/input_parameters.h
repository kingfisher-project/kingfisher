// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_UTILS_INPUT_PARAMETERS_H_
#define KINGFISHER_UTILS_INPUT_PARAMETERS_H_

#include <string>
#include <vector>

#include "utils/enums.h"

namespace kingfisher {

/**
 * @brief `problem` block parameters.
 *
 */
struct ProblemParams {
  std::string name;
  RunMode run_mode;
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
  std::string name;
  BoundaryType type;
};

/**
 * @brief Individual material properties.
 *
 */
struct MaterialParams {
  std::string name;
  std::vector<double> total_xs;
  std::vector<double> scatter_matrix;
};

/**
 * @brief Individual source properties.
 *
 */
struct SourceParams {
  std::string name;
  SourceType type;
  std::string expression;
};

/**
 * @brief `source_iterations` sub-block parameters.
 *
 */
struct SourceIterParams {
  double tolerance;
  unsigned int max_iterations;
};

/**
 * @brief `spatial` block parameters.
 *
 */
struct SpatialParams {
  SENodePlacement se_node_placement;
  size_t nodes_per_axis;
};

/**
 * @brief `solver` block parameters.
 *
 */
struct SolverParams {
  SourceIterParams source_iter_params;
};

/**
 * @brief `output` block parameters.
 *
 */
struct OutputParams {
  OutputFormat output_format;
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

void ReadInput(const std::string filename);
void CheckInputFileVersion();

}  // namespace kingfisher

#endif  // KINGFISHER_UTILS_INPUT_PARAMETERS_H_
