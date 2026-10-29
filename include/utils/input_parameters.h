// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_UTILS_INPUT_PARAMETERS_H_
#define KINGFISHER_UTILS_INPUT_PARAMETERS_H_

#include <string>
#include <vector>

#include "utils/enums.h"

namespace kingfisher {

/**
 * @brief Input parameters for simulation from the user.
 *
 */
struct InputParameters {
  ProblemParams problem_params;
  MeshParams mesh_params;
  SpatialParams spatial_params;
  SolverParams solver_params;
  OutputParams output_params;
};

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
 * @brief `source_iterations` sub-block parameters.
 *
 */
struct SourceIterParams {
  double tolerance;
  unsigned int max_iterations;
};

/**
 * @brief `output` block parameters.
 *
 */
struct OutputParams {
  OutputFormat output_format;
  std::vector<ExportValues> export_values;
};

void ReadInput(const std::string filename);
void CheckInputFileVersion();

}  // namespace kingfisher

#endif  // KINGFISHER_UTILS_INPUT_PARAMETERS_H_
