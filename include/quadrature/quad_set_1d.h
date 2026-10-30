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

  /// @brief Quadrature abscissas
  mfem::Vector abscissas;
};

}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_QUAD_SET_1D_H_
