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
  gauss_legendre.nodes = GaussLegendreNodes(n_points);
  gauss_legendre.weights = GaussLegendreWeights(gauss_legendre.nodes);
  return gauss_legendre;
}

mfem::Vector GaussLegendreNodes(const int n_points) {
  KINGFISHER_ASSERT(
      n_points > 0,
      "Number of points passed to GaussLegendreNodes must be > 0.");
  return AllLegendreRoots(n_points);
}

mfem::Vector GaussLegendreWeights(const mfem::Vector& nodes) {
  const int n_points = nodes.Size();
  mfem::Vector weights(n_points);
  for (auto k = 0; k < n_points; k++) {
    double l_prime = LegendrePolynomialPrime(n_points, nodes[k]);
    weights[k] = 2.0 / (1 - nodes[k] * nodes[k]) / l_prime / l_prime;
  }
  return weights;
}
}  // namespace kingfisher
