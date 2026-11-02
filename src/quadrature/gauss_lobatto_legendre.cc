// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/gauss_lobatto_legendre.h"

#include "utils/legendre_polynomials.h"

namespace kingfisher {

QuadSet1D GaussLobattoLegendre(const int n_points) {
  QuadSet1D quad_set;
  quad_set.nodes = GaussLobattoLegendreNodes(n_points);
  quad_set.weights = GaussLobattoLegendreWeights(quad_set.nodes);
}

mfem::Vector GaussLobattoLegendreNodes(const int n_points) {
  mfem::Vector nodes(n_points);
  nodes[0] = -1.0;
  nodes[n_points - 1] = 1.0;
  mfem::Vector inner_nodes = AllLegendrePrimeRoots(n_points - 1);
  for (auto k = 1; k < n_points - 1; k++) nodes[k] = inner_nodes[k];
  return nodes;
}

mfem::Vector GaussLobattoLegendreWeights(const mfem::Vector& gll_nodes) {
  const int n_points = gll_nodes.Size();
  mfem::Vector weights(n_points);

  const double front_coeff =
      2.0 / static_cast<double>(n_points - 1) / static_cast<double>(n_points);
  for (auto k = 0; k < n_points; k++) {
    double legendre_poly = LegendrePolynomial(n_points - 1, gll_nodes[k]);
    weights[k] = front_coeff / legendre_poly / legendre_poly;
  }
  return weights;
}

}  // namespace kingfisher
