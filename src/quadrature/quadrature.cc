// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/quadrature.h"

#include "quadrature/gauss_legendre.h"
#include "quadrature/trapezoid.h"
#include "utils/errors.h"
#include "utils/numerics.h"

namespace kingfisher {
QuadSet1D Create1DQuadSet(const QuadSetType type, const int n_points,
                          const double a, const double b) {
  switch (type) {
    case QuadSetType::Trapezoid:
      return Trapezoid(n_points, a, b);
    case QuadSetType::GaussLegendre:
      KingfisherError(DoubleEqual(a, -1.0) && DoubleEqual(b, 1.0),
                      "GaussLegendre quadrature set can only be created on the "
                      "unit interval");
      return GaussLegendre(n_points);
    default:
      KingfisherError("QuadSetType passed to Create1DQuadSet not supported.");
      QuadSet1D empty;
      return empty;
  }
}

QuadSet2D Create2DQuadSet(const QuadSet1D& first_axis,
                          const QuadSet1D& second_axis) {
  QuadSet2D quad_set;
  quad_set.weights.SetSize(first_axis.weights.Size() *
                           second_axis.weights.Size());
  for (auto j = 0; j < second_axis.weights.Size(); j++) {
    for (auto i = 0; i < first_axis.weights.Size(); i++) {
      quad_set.weights[j * first_axis.weights.Size() + i] =
          first_axis.weights[i] * second_axis.weights[j];
    }
  }
  quad_set.first_axis_nodes = first_axis.nodes;
  quad_set.second_axis_nodes = second_axis.nodes;

  return quad_set;
}

double Integrate(const mfem::Vector& grid_function,
                 const mfem::Vector& weights) {
  KINGFISHER_ASSERT(grid_function.Size() == weights.Size(),
                    "Sizes of grid_function and weights passed to Integrate "
                    "are not the same.");
  return InnerProduct(grid_function, weights);
}
}  // namespace kingfisher
