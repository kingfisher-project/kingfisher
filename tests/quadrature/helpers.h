// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_TESTS_QUADRATURE_HELPERS_H_
#define KINGFISHER_TESTS_QUADRATURE_HELPERS_H_

#include <functional>
#include <mfem.hpp>

#include "quadrature/quadrature.h"

namespace kingfisher {

/// @brief Exact value of \f$\int_{-1}^{1} x^k dx\f$.
inline double AnalyticMonomialIntegral(const int k) {
  return k % 2 == 0 ? 2.0 / (k + 1) : 0.0;
}

/// @brief Evaluate `f` at each of `nodes`.
inline mfem::Vector Evaluate(const std::function<double(double)>& f,
                             const mfem::Vector& nodes) {
  mfem::Vector values(nodes.Size());
  for (int i = 0; i < nodes.Size(); i++) values[i] = f(nodes[i]);
  return values;
}

/// @brief Integrate `f` over [-1, 1] with the given 1D quadrature set.
inline double IntegrateFunction(const std::function<double(double)>& f,
                                const QuadSet1D& quad_set) {
  return Integrate(Evaluate(f, quad_set.nodes), quad_set.weights);
}

}  // namespace kingfisher

#endif  // KINGFISHER_TESTS_QUADRATURE_HELPERS_H_
