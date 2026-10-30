// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/quad_set_1d.h"

#include "utils/errors.h"

namespace kingfisher {

double Integrate1D(const mfem::Vector& grid_values, const QuadSet1D& quad_set) {
  KINGFISHER_ASSERT(grid_values.Size() == quad_set.abscissas.Size(),
                    "Integrate1D grid_values size does not match the size of "
                    "the quadrature set.");
  double sum = 0;
  for (auto i = 0; i < grid_values.Size(); i++)
    sum += grid_values[i] * quad_set.weights[i];
  return sum;
}
}  // namespace kingfisher
