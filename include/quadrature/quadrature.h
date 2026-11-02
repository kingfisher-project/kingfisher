// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_QUADRATURE_QUADRATURE_H_
#define KINGFISHER_QUADRATURE_QUADRATURE_H_

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

struct QuadSet2D {
  /// @brief Quadrature weights
  mfem::Vector weights;

  /// @brief First axis' nodes
  mfem::Vector first_axis_nodes;

  /// @brief Second axis' nodes
  mfem::Vector second_axis_nodes;
};

/**
 * @brief Creates a 2D quadrature by taking a tensor product of two 1D
 * quadrature sets. `first_axis` is treated as the "fast" axis (the inner loop
 * of the tensor product). If x is the first axis, and y is second axis, then:
 *
 * \f[
 * \int_{-1}^1 \int_{-1}^1 f(x,y)dx dy\approx
 * \sum_{j=0}^{N_y}\sum_{i=0}^{N_x}w_{ij}f(x_i,y_j)
 * \f]
 *
 * @param first_axis First axis quadrature set.
 * @param second_axis Second axis quadrature set.
 * @return QuadSet2D
 */
QuadSet2D Create2DQuadSet(const QuadSet1D& first_axis,
                          const QuadSet1D& second_axis);

/**
 * @brief Integrates a grid function that was evaluated on a quadrature set's
 * nodes using the quadrature's weights. Weights and grid function values are
 * passed as flattened vectors.
 *
 * @param grid_function Grid function values
 * @param weights Quadrature weights
 * @return double
 */
double Integrate(const mfem::Vector& grid_function,
                 const mfem::Vector& weights);
}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_QUADRATURE_H_
