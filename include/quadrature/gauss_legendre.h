// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_QUADRATURE_GAUSS_LEGENDRE_H_
#define KINGFISHER_QUADRATURE_GAUSS_LEGENDRE_H_

#include "quadrature/quadrature.h"

namespace kingfisher {
/**
 * @brief Create the 1D Gauss-Legendre quadrature on the interval \f$\xi\in
 * [-1,1]\f$.
 *
 * @param n_points Number of points
 * @return QuadSet1D
 */
QuadSet1D GaussLegendre(const int n_points);

/**
 * @brief Sets the Gauss-Legendre nodes on the standard interval \f$\xi\in
 * [-1,1]\f$ in place.
 *
 * @param gl_quad_set Quadrature set
 * @param n_points Number of points
 */
void GaussLegendreNodes(QuadSet1D& gl_quad_set, const int n_points);

/**
 * @brief Set the Gauss-Legendre weights corresponding to the given nodes in the
 * standard interval in place.
 *
 * @param gl_quad_set Quadrature set
 */
void GaussLegendreWeights(QuadSet1D& gl_quad_set);

}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_GAUSS_LEGENDRE_H_
