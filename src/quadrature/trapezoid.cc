// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/trapezoid.h"

namespace kingfisher {
QuadSet1D Trapezoid(const double a, const double b, const int n_points) {
  QuadSet1D quad_set;
  TrapezoidNodesAndWeights(quad_set, a, b, n_points);
}

void TrapezoidNodesAndWeights(QuadSet1D& quad_set, const double a,
                              const double b, const int n_points) {
  double delta = (b - a) / static_cast<double>(n_points - 1);
  quad_set.nodes.SetSize(n_points);
  quad_set.weights.SetSize(n_points);
  for (double k = 0; k < n_points; k++) {
    quad_set.nodes[k] = a + delta * k;
    quad_set.weights[k] = delta;
  }
  quad_set.weights[0] /= 2.0;
  quad_set.weights[n_points - 1] /= 2.0;
}

}  // namespace kingfisher
