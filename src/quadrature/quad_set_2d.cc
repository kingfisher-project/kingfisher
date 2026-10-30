// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/quad_set_2d.h"

#include "utils/errors.h"
namespace kingfisher {
QuadSet2D Create2DQuadSet(const QuadSet1D& first_axis,
                          const QuadSet1D& second_axis) {
  QuadSet2D quad_set;
  quad_set.weights.Reserve(first_axis.weights.Size() *
                           second_axis.weights.Size());
  for (auto j = 0; j < second_axis.weights.Size(); j++) {
    for (auto i = 0; i < first_axis.weights.Size(); i++) {
      quad_set.weights[j * first_axis.weights.Size() + i] =
          first_axis.weights[i] * second_axis.weights[j];
    }
  }
  quad_set.first_axis_abscissas = first_axis.abscissas;
  quad_set.second_axis_abscissas = second_axis.abscissas;

  return quad_set;
}

double Integrate2D(const mfem::Vector& grid_values, const QuadSet2D& quad_set) {
  KINGFISHER_ASSERT(grid_values.Size() == quad_set.weights.Size(),
                    "Sive of grid function values passed to Integrate2D does "
                    "not match the number of weights in the quadrature set.");

  double sum = 0;
  for (auto i = 0; i < grid_values.Size(); i++)
    sum += grid_values[i] * quad_set.weights[i];
  return sum;
}
}  // namespace kingfisher
