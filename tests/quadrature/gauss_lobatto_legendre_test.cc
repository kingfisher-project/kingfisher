// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/gauss_lobatto_legendre.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>

#include "helpers.h"
#include "quadrature/quadrature.h"
#include "utils/errors.h"
#include "utils/legendre_polynomials.h"
#include "utils/numerics.h"

namespace kingfisher {

// =======================================================================
// Properties that must hold for every number of points (n >= 2)
// =======================================================================

class GaussLobattoLegendreTest : public ::testing::TestWithParam<int> {};

INSTANTIATE_TEST_SUITE_P(PointCounts, GaussLobattoLegendreTest,
                         ::testing::Values(2, 3, 4, 5, 6, 8, 10, 20));

TEST_P(GaussLobattoLegendreTest, NodesAndWeightsHaveNPoints) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLobattoLegendre(n);
  EXPECT_EQ(quad_set.nodes.Size(), n);
  EXPECT_EQ(quad_set.weights.Size(), n);
}

// Two degrees of freedom are spent on the fixed endpoints, so an n-point
// Lobatto rule is exact only up to degree 2n - 3.
TEST_P(GaussLobattoLegendreTest, ExactForMonomialsUpToDegree2NMinus3) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLobattoLegendre(n);
  for (int k = 0; k <= 2 * n - 3; k++) {
    const double result =
        IntegrateFunction([k](double x) { return std::pow(x, k); }, quad_set);
    EXPECT_NEAR(result, AnalyticMonomialIntegral(k), EXP_NEAR_TOLERANCE)
        << "n=" << n << ", k=" << k;
  }
}

// Degree 2n - 2 is the first monomial the rule cannot integrate exactly.
TEST_P(GaussLobattoLegendreTest, NotExactForMonomialOfDegree2NMinus2) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLobattoLegendre(n);
  const int k = 2 * n - 2;
  const double result =
      IntegrateFunction([k](double x) { return std::pow(x, k); }, quad_set);
  const double error = std::abs(result - AnalyticMonomialIntegral(k));
  // The error shrinks rapidly with n (roughly 4^{-n}); require it to be far
  // above round-off.
  EXPECT_GT(error, 1e-3 * std::pow(0.25, n)) << "n=" << n;
}

TEST_P(GaussLobattoLegendreTest, WeightsArePositiveAndSumToTwo) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLobattoLegendre(n);
  double sum = 0.0;
  for (int i = 0; i < n; i++) {
    EXPECT_GT(quad_set.weights[i], 0.0) << "n=" << n << ", i=" << i;
    sum += quad_set.weights[i];
  }
  EXPECT_NEAR(sum, 2.0, EXP_NEAR_TOLERANCE) << "n=" << n;
}

// Lobatto sets are *closed*: both endpoints are nodes.
TEST_P(GaussLobattoLegendreTest, EndpointsAreExactlyMinusOneAndOne) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLobattoLegendre(n);
  EXPECT_EQ(quad_set.nodes[0], -1.0);
  EXPECT_EQ(quad_set.nodes[n - 1], 1.0);
}

TEST_P(GaussLobattoLegendreTest, NodesAreOrderedAndSymmetric) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLobattoLegendre(n);
  for (int i = 0; i < n; i++) {
    if (i > 0) EXPECT_LT(quad_set.nodes[i - 1], quad_set.nodes[i]) << "i=" << i;
    EXPECT_NEAR(quad_set.nodes[i], -quad_set.nodes[n - 1 - i],
                EXP_NEAR_TOLERANCE)
        << "i=" << i;
    EXPECT_NEAR(quad_set.weights[i], quad_set.weights[n - 1 - i],
                EXP_NEAR_TOLERANCE)
        << "i=" << i;
  }
  if (n % 2 == 1) EXPECT_EQ(quad_set.nodes[n / 2], 0.0);
}

// The interior nodes are the roots of P'_{n-1}.
TEST_P(GaussLobattoLegendreTest, InteriorNodesAreRootsOfLegendrePrime) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLobattoLegendre(n);
  for (int i = 1; i < n - 1; i++) {
    EXPECT_NEAR(LegendrePolynomialPrime(n - 1, quad_set.nodes[i]), 0.0,
                EXP_NEAR_TOLERANCE * n * n)
        << "n=" << n << ", i=" << i;
  }
}

TEST_P(GaussLobattoLegendreTest, GaussLobattoLegendreMatchesInPlaceFunctions) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLobattoLegendre(n);
  QuadSet1D expected;
  GaussLobattoLegendreNodes(expected, n);
  GaussLobattoLegendreWeights(expected);
  ASSERT_EQ(quad_set.nodes.Size(), expected.nodes.Size());
  ASSERT_EQ(quad_set.weights.Size(), expected.weights.Size());
  for (int i = 0; i < n; i++) {
    EXPECT_EQ(quad_set.nodes[i], expected.nodes[i]);
    EXPECT_EQ(quad_set.weights[i], expected.weights[i]);
  }
}

// The in-place functions must fully replace whatever the set held before, even
// if it had a different size.
TEST_P(GaussLobattoLegendreTest, InPlaceFunctionsOverwriteExistingSet) {
  const int n = GetParam();
  const QuadSet1D expected = GaussLobattoLegendre(n);

  QuadSet1D reused = GaussLobattoLegendre(n + 3);
  GaussLobattoLegendreNodes(reused, n);
  GaussLobattoLegendreWeights(reused);

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

TEST(GaussLobattoLegendre, TwoPointRuleIsTrapezoid) {
  const QuadSet1D quad_set = GaussLobattoLegendre(2);
  EXPECT_NEAR(quad_set.nodes[0], -1.0, TOLERANCE);
  EXPECT_NEAR(quad_set.nodes[1], 1.0, TOLERANCE);
  EXPECT_NEAR(quad_set.weights[0], 1.0, EXP_NEAR_TOLERANCE);
  EXPECT_NEAR(quad_set.weights[1], 1.0, EXP_NEAR_TOLERANCE);
}

TEST(GaussLobattoLegendre, ThreePointRuleIsSimpson) {
  const QuadSet1D quad_set = GaussLobattoLegendre(3);
  const double expected_nodes[] = {-1.0, 0.0, 1.0};
  const double expected_weights[] = {1.0 / 3.0, 4.0 / 3.0, 1.0 / 3.0};
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(quad_set.nodes[i], expected_nodes[i], EXP_NEAR_TOLERANCE);
    EXPECT_NEAR(quad_set.weights[i], expected_weights[i], EXP_NEAR_TOLERANCE);
  }
}

TEST(GaussLobattoLegendre, FourPointRuleMatchesClosedForm) {
  const QuadSet1D quad_set = GaussLobattoLegendre(4);
  const double x = 1.0 / std::sqrt(5.0);
  const double expected_nodes[] = {-1.0, -x, x, 1.0};
  const double expected_weights[] = {1.0 / 6.0, 5.0 / 6.0, 5.0 / 6.0,
                                     1.0 / 6.0};
  for (int i = 0; i < 4; i++) {
    EXPECT_NEAR(quad_set.nodes[i], expected_nodes[i], EXP_NEAR_TOLERANCE);
    EXPECT_NEAR(quad_set.weights[i], expected_weights[i], EXP_NEAR_TOLERANCE);
  }
}

TEST(GaussLobattoLegendre, FivePointRuleMatchesClosedForm) {
  const QuadSet1D quad_set = GaussLobattoLegendre(5);
  const double x = std::sqrt(3.0 / 7.0);
  const double expected_nodes[] = {-1.0, -x, 0.0, x, 1.0};
  const double expected_weights[] = {1.0 / 10.0, 49.0 / 90.0, 32.0 / 45.0,
                                     49.0 / 90.0, 1.0 / 10.0};
  for (int i = 0; i < 5; i++) {
    EXPECT_NEAR(quad_set.nodes[i], expected_nodes[i], EXP_NEAR_TOLERANCE);
    EXPECT_NEAR(quad_set.weights[i], expected_weights[i], EXP_NEAR_TOLERANCE);
  }
}

// =======================================================================
// Invalid input
// =======================================================================

// KINGFISHER_ASSERT is compiled out when NDEBUG is defined.
#ifndef NDEBUG
TEST(GaussLobattoLegendre, FewerThanTwoPointsThrows) {
  QuadSet1D quad_set;
  EXPECT_THROW(GaussLobattoLegendreNodes(quad_set, 1), Error);
  EXPECT_THROW(GaussLobattoLegendreNodes(quad_set, 0), Error);
  EXPECT_THROW(GaussLobattoLegendreNodes(quad_set, -3), Error);
  EXPECT_THROW(GaussLobattoLegendre(1), Error);
  EXPECT_THROW(GaussLobattoLegendre(0), Error);
}
#endif

// =======================================================================
// Non-polynomial integrand
// =======================================================================

// \int_{-1}^{1} e^x dx = e - 1/e. Lobatto rules converge geometrically for
// analytic integrands, a bit slower than Gauss-Legendre for the same n.
TEST(GaussLobattoLegendre, ExponentialIntegralConverges) {
  const double expected = std::exp(1.0) - std::exp(-1.0);
  const auto exponential = [](double x) { return std::exp(x); };

  double previous_error = std::numeric_limits<double>::max();
  for (const int n : {3, 5, 7, 9}) {
    const double error = std::abs(
        IntegrateFunction(exponential, GaussLobattoLegendre(n)) - expected);
    EXPECT_LT(error, previous_error) << "n=" << n;
    previous_error = error;
  }
  EXPECT_LT(previous_error, 1e-8);

  EXPECT_NEAR(IntegrateFunction(exponential, GaussLobattoLegendre(20)),
              expected, 1e-13);
}

}  // namespace kingfisher
