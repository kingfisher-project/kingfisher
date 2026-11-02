// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_QUADRATURE_TRAPEZOID_H_
#define KINGFISHER_QUADRATURE_TRAPEZOID_H_

#include "quadrature/quadrature.h"

namespace kingfisher {
QuadSet1D Trapezoid(const double a = -1.0, const double b = 1.0,
                    const int n_points);

void TrapezoidNodesAndWeights(QuadSet1D& quad_set, const double a,
                              const double b, const int n_points);

}  // namespace kingfisher

#endif  // KINGFISHER_QUADRATURE_TRAPEZOID_H_
