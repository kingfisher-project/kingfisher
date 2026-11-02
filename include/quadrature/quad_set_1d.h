// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_QUADRATURE_QUAD_SET_1D_H_
#define KINGFISHER_QUADRATURE_QUAD_SET_1D_H_

#include <mfem/linalg/vector.hpp>

namespace kingfisher {

/**
 * @brief Defines a quadrature set in 1D.
 *
 */
struct QuadSet1D {
  /// @brief Quadrature weights
  mfem::Vector weights;

  /// @brief Quadrature nodes
  mfem::Vector nodes;
};

/**
 * @brief Integrate a 1D grid function using the supplied quadrature set. The
 * grid function values are expected to be evaluated at the quadrature set's
 * nodes. We approximate:
 *
 * \f[
 * \int_{-1}^1 f(x)dx\approx \sum_{i=0}^{N}w_i f(x_i)
 * \f]
 *
 * where \f$ x_i\in \{x_0,x_1,\dots,x_N\}\f$ are the quadrature nodes, and \f$
 * w_i\in \{w_0,w_1,\dots,w_N\}\f$ are the quadrature weights at the nodes.
 *
 * @param grid_values Grid function values.
 * @param quad_set Quadrature set.
 * @return double
 */
double Integrate1D(const mfem::Vector& grid_values, const QuadSet1D& quad_set);

}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_QUAD_SET_1D_H_
