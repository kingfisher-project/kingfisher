// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_UTILS_ENUMS_H_
#define KINGFISHER_UTILS_ENUMS_H_

namespace kingfisher {
/**
 * @brief Available run modes.
 *
 */
enum class RunMode { FixedSource };

/**
 * @brief Available spectral element ("SE") node placement choices.
 *
 */
enum class SENodePlacement { GaussLobattoLegendre };

/**
 * @brief Available output file formats.
 *
 */
enum class OutputFormat { VTKHDF };

/**
 * @brief Available values to be exported from the simulation.
 *
 */
enum class ExportValues { ScalarFlux, AngularFlux };

/**
 * @brief Available boundary condition types.
 *
 */
enum class BoundaryType { Vacuum, Reflective };

/**
 * @brief Available independent source types.
 *
 */
enum class SourceType { Volumetric };

/**
 * @brief Available angular treatment models.
 *
 */
enum class AngularModel { DiscreteOrdinates };

/**
 * @brief Available spherical quadrature types.
 *
 */
enum class SphericalQuadType { TensorProduct };

/**
 * @brief Available generic quadrature sets.
 *
 */
enum class QuadSetType { GaussLegendre, Trapezoid };
}  // namespace kingfisher

#endif  // KINGFISHER_UTILS_ENUMS_H_
