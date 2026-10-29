// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_UTILS_ENUMS_H_
#define KINGFISHER_UTILS_ENUMS_H_

namespace kingfisher {
/**
 * @brief Available run modes.
 *
 */
enum class RunMode { FIXED_SOURCE };

/**
 * @brief Available spectral element ("SE") node placement choices.
 *
 */
enum class SENodePlacement { GaussLegendreLobatto };

/**
 * @brief Available output file formats.
 *
 */
enum class OutputFormat { VTKHDF };

/**
 * @brief Available values to be exported from the simulation.
 *
 */
enum class ExportValues { SCALAR_FLUX, ANGULAR_FLUX };

/**
 * @brief Available boundary condition types.
 *
 */
enum class BoundaryType { VACUUM, REFLECTIVE };

/**
 * @brief Available independent source types.
 *
 */
enum class SOURCE_TYPE { VOLUMETRIC };
}  // namespace kingfisher

#endif  // KINGFISHER_UTILS_ENUMS_H_
