// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, Liam Pohlmann
// Copyright (c) 2026, kingfisher-project

#include "quadrature/gauss_legendre.h"

#include "utils/errors.h"
#include "utils/legendre_polynomials.h"
#include "utils/numerics.h"

namespace kingfisher {

QuadSet1D GaussLegendre(const int n_points) {
  QuadSet1D gauss_legendre;
  GaussLegendreNodes(gauss_legendre, n_points);
  GaussLegendreWeights(gauss_legendre);
  return gauss_legendre;
}

void GaussLegendreNodes(QuadSet1D& gl_quad_set, const int n_points) {
  KINGFISHER_ASSERT(
      n_points > 0,
      "Number of points passed to GaussLegendreNodes must be > 0.");
  gl_quad_set.nodes = AllLegendreRoots(n_points);
}

void GaussLegendreWeights(QuadSet1D& gl_quad_set) {
  const int n_points = gl_quad_set.nodes.Size();
  gl_quad_set.weights.SetSize(n_points);

  for (auto k = 0; k < n_points; k++) {
    double l_prime = LegendrePolynomialPrime(n_points, gl_quad_set.nodes[k]);
    gl_quad_set.weights[k] = 2.0 /
                             (1 - gl_quad_set.nodes[k] * gl_quad_set.nodes[k]) /
                             l_prime / l_prime;
  }
}
}  // namespace kingfisher
