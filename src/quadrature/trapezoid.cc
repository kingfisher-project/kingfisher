// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/trapezoid.h"

#include "utils/errors.h"

namespace kingfisher {
QuadSet1D Trapezoid(const int n_points, const double a, const double b) {
  QuadSet1D quad_set;
  TrapezoidNodesAndWeights(quad_set, n_points, a, b);
  return quad_set;
}

void TrapezoidNodesAndWeights(QuadSet1D& quad_set, const int n_points,
                              const double a, const double b) {
  KINGFISHER_ASSERT(n_points >= 2,
                    "Number of points passed to TrapezoidNodesAndWeights must "
                    "be at least 2.");
  KINGFISHER_ASSERT(b > a,
                    "Upper bound b passed to TrapezoidNodesAndWeights must be "
                    "greater than lower bound a.");
  double delta = (b - a) / static_cast<double>(n_points - 1);
  quad_set.nodes.SetSize(n_points);
  quad_set.weights.SetSize(n_points);
  for (auto k = 0; k < n_points; k++) {
    quad_set.nodes[k] = a + delta * static_cast<double>(k);
    quad_set.weights[k] = delta;
  }
  quad_set.weights[0] /= 2.0;
  quad_set.weights[n_points - 1] /= 2.0;
}

}  // namespace kingfisher
