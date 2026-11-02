// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_QUADRATURE_TRAPEZOID_H_
#define KINGFISHER_QUADRATURE_TRAPEZOID_H_

#include "quadrature/quadrature.h"

namespace kingfisher {
/**
 * @brief Create a 1D trapezoid quadrature rule which approximates the integral
 * from \f$a\f$ to \f$b\f$.
 *
 * \f[
 * \int_a^bf(x)dx\approx \frac{h}{2}(f(x_0)+f(x_{N-1}))+h\sum_{k=1}^{N-2}f(x_k)
 * \f]
 *
 * @param n_points Number of points desired. Requires at least 2.
 * @param a Integrate from
 * @param b Integrate to
 * @return QuadSet1D
 */
QuadSet1D Trapezoid(const int n_points, const double a = -1.0,
                    const double b = 1.0);

/**
 * @brief Compute and set the nodes and weights of a 1D trapezoid quadrature set
 * in place.
 *
 * @param quad_set Quadrature set to modify
 * @param n_points Number of points
 * @param a Integrate from
 * @param b Integrate to
 */
void TrapezoidNodesAndWeights(QuadSet1D& quad_set, const int n_points,
                              const double a, const double b);

}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_TRAPEZOID_H_
