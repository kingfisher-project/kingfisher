// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_QUADRATURE_GAUSS_LOBATTO_LEGENDRE_H_
#define KINGFISHER_QUADRATURE_GAUSS_LOBATTO_LEGENDRE_H_

#include <mfem/linalg/vector.hpp>

#include "quadrature/quadrature.h"

namespace kingfisher {
QuadSet1D GaussLobattoLegendre(const int n_points);

void GaussLobattoLegendreNodes(QuadSet1D& gll_quad_set, const int n_points);

void GaussLobattoLegendreWeights(QuadSet1D& gll_quad_set);
}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_GAUSS_LOBATTO_LEGENDRE_H_
