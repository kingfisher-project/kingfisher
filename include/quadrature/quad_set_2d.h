// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_QUADRATURE_QUAD_SET_2D_H_
#define KINGFISHER_QUADRATURE_QUAD_SET_2D_H_

#include <mfem/linalg/vector.hpp>

#include "quadrature/quad_set_1d.h"

namespace kingfisher {
struct QuadSet2D {
  mfem::Vector weights;
  mfem::Vector first_axis_abscissas;
  mfem::Vector second_axis_abscissas;
};

/**
 * @brief Creates a 2D quadrature by taking a tensor product of two 1D
 * quadrature sets.
 *
 * @param first_axis First axis quadrature set.
 * @param second_axis Second axis quadrature set.
 * @return QuadSet2D
 */
QuadSet2D Create2DQuadSet(const QuadSet1D& first_axis,
                          const QuadSet1D& second_axis);
/**
 * @brief Integrates a grid function in 2D using the supplied quadrature set.
 *
 * @param grid_values Grid function values, which are assumed to be evaluated at
 * the expected abscissas in 2D.
 * @param quad_set 2D quadrature set.
 * @return double
 * @todo Add more information on HOW the integral is evaluated, notably on the
 * "fast" iterator.
 */
double Integrate2D(const mfem::Vector& grid_values, const QuadSet2D& quad_set);

}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_QUAD_SET_2D_H_
