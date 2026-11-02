// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_QUADRATURE_GAUSS_LOBATTO_LEGENDRE_H_
#define KINGFISHER_QUADRATURE_GAUSS_LOBATTO_LEGENDRE_H_

#include "quadrature/quadrature.h"

namespace kingfisher {
/**
 * @brief Create a 1D Gauss-Lobatto-Legendre quadrature set on the standard
 * interval \f$\xi\in [-1,1]\f$.
 *
 * @param n_points Number of points. Must be >=2
 * @return QuadSet1D
 */
QuadSet1D GaussLobattoLegendre(const int n_points);

/**
 * @brief Modify a 1D quadrature set in place to create Gauss-Lobatto-Legendre
 * nodes.
 *
 * @param gll_quad_set QuadSet1D quadrature object to modify
 * @param n_points Number of points desired in set
 */
void GaussLobattoLegendreNodes(QuadSet1D& gll_quad_set, const int n_points);

/**
 * @brief Modify a 1D quadrature set in place to create Gauss-Lobatto-Legendre
 * weights.
 *
 * @param gll_quad_set QuadSet1D quadrature object to modify
 */
void GaussLobattoLegendreWeights(QuadSet1D& gll_quad_set);
}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_GAUSS_LOBATTO_LEGENDRE_H_
