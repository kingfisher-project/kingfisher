// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/quadrature.h"

namespace kingfisher {
QuadSet2D Create2DQuadSet(const QuadSet1D& first_axis,
                          const QuadSet1D& second_axis) {
  QuadSet2D quad_set;
  quad_set.weights.Reserve(first_axis.weights.Size() *
                           second_axis.weights.Size());
  for (auto j = 0; j < second_axis.weights.Size(); j++) {
    for (auto i = 0; i < first_axis.weights.Size(); i++) {
      quad_set.weights[j * first_axis.weights.Size() + i] =
          first_axis.weights[i] * second_axis.weights[j];
    }
  }
  quad_set.first_axis_nodes = first_axis.nodes;
  quad_set.first_axis_nodes = second_axis.nodes;

  return quad_set;
}
}  // namespace kingfisher
