// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/gauss_lobatto_legendre.h"

#include "utils/legendre_polynomials.h"

namespace kingfisher {

QuadSet1D GaussLobattoLegendre(const int n_points) {
  QuadSet1D quad_set;
  GaussLobattoLegendreNodes(quad_set, n_points);
  GaussLobattoLegendreWeights(quad_set);
  return quad_set;
}

void GaussLobattoLegendreNodes(QuadSet1D& gll_quad_set, const int n_points) {
  gll_quad_set.nodes.SetSize(n_points);
  gll_quad_set.nodes[0] = -1.0;
  gll_quad_set.nodes[n_points - 1] = 1.0;
  mfem::Vector inner_nodes = AllLegendrePrimeRoots(n_points - 1);
  for (auto k = 1; k < n_points - 1; k++)
    gll_quad_set.nodes[k] = inner_nodes[k - 1];
}

void GaussLobattoLegendreWeights(QuadSet1D& gll_quad_set) {
  const int n_points = gll_quad_set.nodes.Size();
  gll_quad_set.weights.SetSize(n_points);

  const double front_coeff =
      2.0 / static_cast<double>(n_points - 1) / static_cast<double>(n_points);
  for (auto k = 0; k < n_points; k++) {
    double legendre_weight =
        LegendrePolynomial(n_points - 1, gll_quad_set.nodes[k]);
    gll_quad_set.weights[k] = front_coeff / legendre_weight / legendre_weight;
  }
}

}  // namespace kingfisher
