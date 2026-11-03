// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/gauss_legendre.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>

#include "../tests/constants.h"
#include "helpers.h"
#include "quadrature/quadrature.h"
#include "utils/errors.h"
#include "utils/legendre_polynomials.h"
#include "utils/numerics.h"

namespace kingfisher {

// =======================================================================
// Properties that must hold for every number of points
// =======================================================================

class GaussLegendreTest : public ::testing::TestWithParam<int> {};

INSTANTIATE_TEST_SUITE_P(PointCounts, GaussLegendreTest,
                         ::testing::Values(1, 2, 3, 4, 5, 8, 10, 20));

TEST_P(GaussLegendreTest, NodesAndWeightsHaveNPoints) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLegendre(n);
  EXPECT_EQ(quad_set.nodes.Size(), n);
  EXPECT_EQ(quad_set.weights.Size(), n);
}

// An n-point rule is exact for all polynomials of degree <= 2n - 1.
TEST_P(GaussLegendreTest, ExactForMonomialsUpToDegree2NMinus1) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLegendre(n);
  for (int k = 0; k <= 2 * n - 1; k++) {
    const double result =
        IntegrateFunction([k](double x) { return std::pow(x, k); }, quad_set);
    EXPECT_NEAR(result, AnalyticMonomialIntegral(k), kExpNearTolerance)
        << "n=" << n << ", k=" << k;
  }
}

// Degree 2n is the first monomial the rule cannot integrate exactly. This
// guards against a rule that is secretly higher order, or whose weights come
// from a different n than its nodes.
TEST_P(GaussLegendreTest, NotExactForMonomialOfDegree2N) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLegendre(n);
  const int k = 2 * n;
  const double result =
      IntegrateFunction([k](double x) { return std::pow(x, k); }, quad_set);
  // The error of the n-point rule on x^{2n} is positive and shrinks rapidly
  // with n (roughly 4^{-n}); require it to be far above round-off.
  const double error = std::abs(result - AnalyticMonomialIntegral(k));
  EXPECT_GT(error, 1e-3 * std::pow(0.25, n)) << "n=" << n;
}

TEST_P(GaussLegendreTest, WeightsArePositiveAndSumToTwo) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLegendre(n);
  double sum = 0.0;
  for (int i = 0; i < n; i++) {
    EXPECT_GT(quad_set.weights[i], 0.0) << "n=" << n << ", i=" << i;
    sum += quad_set.weights[i];
  }
  EXPECT_NEAR(sum, 2.0, kExpNearTolerance) << "n=" << n;
}

TEST_P(GaussLegendreTest, NodesAreOrderedSymmetricAndInsideTheInterval) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLegendre(n);
  for (int i = 0; i < n; i++) {
    EXPECT_GT(quad_set.nodes[i], -1.0);
    EXPECT_LT(quad_set.nodes[i], 1.0);
    if (i > 0) EXPECT_LT(quad_set.nodes[i - 1], quad_set.nodes[i]);
    // Antisymmetric nodes, symmetric weights. Exact: the implementation
    // mirrors the nodes, and the weights are even in x.
    EXPECT_NEAR(quad_set.nodes[i], -quad_set.nodes[n - 1 - i],
                kExpNearTolerance);
    EXPECT_NEAR(quad_set.weights[i], quad_set.weights[n - 1 - i],
                kExpNearTolerance);
  }
  if (n % 2 == 1) EXPECT_EQ(quad_set.nodes[n / 2], 0.0);
}

TEST_P(GaussLegendreTest, NodesAreRootsOfLegendrePolynomial) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLegendre(n);
  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(LegendrePolynomial(n, quad_set.nodes[i]), 0.0,
                kExpNearTolerance)
        << "n=" << n << ", i=" << i;
  }
}

TEST_P(GaussLegendreTest, GaussLegendreMatchesNodesAndWeightsFunctions) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLegendre(n);
  QuadSet1D expected;
  GaussLegendreNodes(expected, n);
  GaussLegendreWeights(expected);
  ASSERT_EQ(quad_set.nodes.Size(), expected.nodes.Size());
  ASSERT_EQ(quad_set.weights.Size(), expected.weights.Size());
  for (int i = 0; i < n; i++) {
    EXPECT_EQ(quad_set.nodes[i], expected.nodes[i]);
    EXPECT_EQ(quad_set.weights[i], expected.weights[i]);
  }
}

// The in-place functions must fully replace whatever the set held before, even
// if it had a different size.
TEST_P(GaussLegendreTest, InPlaceFunctionsOverwriteExistingSet) {
  const int n = GetParam();
  const QuadSet1D expected = GaussLegendre(n);

  QuadSet1D reused = GaussLegendre(n + 3);
  GaussLegendreNodes(reused, n);
  GaussLegendreWeights(reused);

  ASSERT_EQ(reused.nodes.Size(), n);
  ASSERT_EQ(reused.weights.Size(), n);
  for (int i = 0; i < n; i++) {
    EXPECT_EQ(reused.nodes[i], expected.nodes[i]);
    EXPECT_EQ(reused.weights[i], expected.weights[i]);
  }
}

// =======================================================================
// Closed-form low-order rules (independent of the implementation)
// =======================================================================

TEST(GaussLegendre, OnePointRuleIsMidpoint) {
  const QuadSet1D quad_set = GaussLegendre(1);
  EXPECT_NEAR(quad_set.nodes[0], 0.0, kTolerance);
  EXPECT_NEAR(quad_set.weights[0], 2.0, kTolerance);
}

TEST(GaussLegendre, TwoPointRuleMatchesClosedForm) {
  const QuadSet1D quad_set = GaussLegendre(2);
  const double x = 1.0 / std::sqrt(3.0);
  EXPECT_NEAR(quad_set.nodes[0], -x, kExpNearTolerance);
  EXPECT_NEAR(quad_set.nodes[1], x, kExpNearTolerance);
  EXPECT_NEAR(quad_set.weights[0], 1.0, kExpNearTolerance);
  EXPECT_NEAR(quad_set.weights[1], 1.0, kExpNearTolerance);
}

TEST(GaussLegendre, ThreePointRuleMatchesClosedForm) {
  const QuadSet1D quad_set = GaussLegendre(3);
  const double x = std::sqrt(0.6);
  EXPECT_NEAR(quad_set.nodes[0], -x, kExpNearTolerance);
  EXPECT_NEAR(quad_set.nodes[1], 0.0, kExpNearTolerance);
  EXPECT_NEAR(quad_set.nodes[2], x, kExpNearTolerance);
  EXPECT_NEAR(quad_set.weights[0], 5.0 / 9.0, kExpNearTolerance);
  EXPECT_NEAR(quad_set.weights[1], 8.0 / 9.0, kExpNearTolerance);
  EXPECT_NEAR(quad_set.weights[2], 5.0 / 9.0, kExpNearTolerance);
}

// =======================================================================
// Invalid input
// =======================================================================

// KINGFISHER_ASSERT is compiled out when NDEBUG is defined.
#ifndef NDEBUG
TEST(GaussLegendre, NonPositiveNumberOfPointsThrows) {
  QuadSet1D quad_set;
  EXPECT_THROW(GaussLegendreNodes(quad_set, 0), Error);
  EXPECT_THROW(GaussLegendreNodes(quad_set, -3), Error);
  EXPECT_THROW(GaussLegendre(0), Error);
  EXPECT_THROW(GaussLegendre(-3), Error);
}
#endif

// =======================================================================
// Non-polynomial integrand
// =======================================================================

// \int_{-1}^{1} e^x dx = e - 1/e. Gauss-Legendre converges geometrically for
// analytic integrands, so the error must fall at every step until it reaches
// round-off.
TEST(GaussLegendre, ExponentialIntegralConverges) {
  const double expected = std::exp(1.0) - std::exp(-1.0);
  const auto exponential = [](double x) { return std::exp(x); };

  double previous_error = std::numeric_limits<double>::max();
  for (const int n : {2, 4, 6, 8}) {
    const double error =
        std::abs(IntegrateFunction(exponential, GaussLegendre(n)) - expected);
    EXPECT_LT(error, previous_error) << "n=" << n;
    previous_error = error;
  }

  EXPECT_NEAR(IntegrateFunction(exponential, GaussLegendre(10)), expected,
              1e-13);
}

}  // namespace kingfisher
