// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/quad_set_2d.h"

#include "utils/errors.h"
namespace kingfisher {
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
