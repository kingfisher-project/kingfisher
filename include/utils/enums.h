// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_UTILS_ENUMS_H_
#define KINGFISHER_UTILS_ENUMS_H_

namespace kingfisher {
/**
 * @brief Available run modes.
 *
 */
enum class RunMode { kFixedSource };

/**
 * @brief Available spectral element ("SE") node placement choices.
 *
 */
enum class SENodePlacement { kGaussLobattoLegendre };

/**
 * @brief Available output file formats.
 *
 */
enum class OutputFormat { kVTKHDF };

/**
 * @brief Available values to be exported from the simulation.
 *
 */
enum class ExportValues { kScalarFlux, kAngularFlux };

/**
 * @brief Available boundary condition types.
 *
 */
enum class BoundaryType { kVacuum, kReflective };

/**
 * @brief Available independent source types.
 *
 */
enum class SourceType { kVolumetric };
}  // namespace kingfisher

#endif  // KINGFISHER_UTILS_ENUMS_H_
