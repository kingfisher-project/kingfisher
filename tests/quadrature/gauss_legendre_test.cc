// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/gauss_legendre.h"

#include <gtest/gtest.h>

#include <cmath>
#include <functional>
#include <limits>

#include "quadrature/quadrature.h"
#include "utils/errors.h"
#include "utils/legendre_polynomials.h"
#include "utils/numerics.h"

namespace kingfisher {
namespace {

// \int_{-1}^{1} x^k dx
double AnalyticMonomialIntegral(const int k) {
  return k % 2 == 0 ? 2.0 / (k + 1) : 0.0;
}

mfem::Vector Evaluate(const std::function<double(double)>& f,
                      const mfem::Vector& nodes) {
  mfem::Vector values(nodes.Size());
  for (int i = 0; i < nodes.Size(); i++) values[i] = f(nodes[i]);
  return values;
}

double IntegrateFunction(const std::function<double(double)>& f,
                         const QuadSet1D& quad_set) {
  return Integrate(Evaluate(f, quad_set.nodes), quad_set.weights);
}

}  // namespace

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
    EXPECT_NEAR(result, AnalyticMonomialIntegral(k), EXP_NEAR_TOLERANCE)
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
  EXPECT_NEAR(sum, 2.0, EXP_NEAR_TOLERANCE) << "n=" << n;
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
                EXP_NEAR_TOLERANCE);
    EXPECT_NEAR(quad_set.weights[i], quad_set.weights[n - 1 - i],
                EXP_NEAR_TOLERANCE);
  }
  if (n % 2 == 1) EXPECT_EQ(quad_set.nodes[n / 2], 0.0);
}

TEST_P(GaussLegendreTest, NodesAreRootsOfLegendrePolynomial) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLegendre(n);
  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(LegendrePolynomial(n, quad_set.nodes[i]), 0.0,
                EXP_NEAR_TOLERANCE)
        << "n=" << n << ", i=" << i;
  }
}

TEST_P(GaussLegendreTest, GaussLegendreMatchesNodesAndWeightsFunctions) {
  const int n = GetParam();
  const QuadSet1D quad_set = GaussLegendre(n);
  const mfem::Vector nodes = GaussLegendreNodes(n);
  const mfem::Vector weights = GaussLegendreWeights(nodes);
  ASSERT_EQ(quad_set.nodes.Size(), nodes.Size());
  ASSERT_EQ(quad_set.weights.Size(), weights.Size());
  for (int i = 0; i < n; i++) {
    EXPECT_EQ(quad_set.nodes[i], nodes[i]);
    EXPECT_EQ(quad_set.weights[i], weights[i]);
  }
}

// =======================================================================
// Closed-form low-order rules (independent of the implementation)
// =======================================================================

TEST(GaussLegendre, OnePointRuleIsMidpoint) {
  const QuadSet1D quad_set = GaussLegendre(1);
  EXPECT_NEAR(quad_set.nodes[0], 0.0, TOLERANCE);
  EXPECT_NEAR(quad_set.weights[0], 2.0, TOLERANCE);
}

TEST(GaussLegendre, TwoPointRuleMatchesClosedForm) {
  const QuadSet1D quad_set = GaussLegendre(2);
  const double x = 1.0 / std::sqrt(3.0);
  EXPECT_NEAR(quad_set.nodes[0], -x, EXP_NEAR_TOLERANCE);
  EXPECT_NEAR(quad_set.nodes[1], x, EXP_NEAR_TOLERANCE);
  EXPECT_NEAR(quad_set.weights[0], 1.0, EXP_NEAR_TOLERANCE);
  EXPECT_NEAR(quad_set.weights[1], 1.0, EXP_NEAR_TOLERANCE);
}

TEST(GaussLegendre, ThreePointRuleMatchesClosedForm) {
  const QuadSet1D quad_set = GaussLegendre(3);
  const double x = std::sqrt(0.6);
  EXPECT_NEAR(quad_set.nodes[0], -x, EXP_NEAR_TOLERANCE);
  EXPECT_NEAR(quad_set.nodes[1], 0.0, EXP_NEAR_TOLERANCE);
  EXPECT_NEAR(quad_set.nodes[2], x, EXP_NEAR_TOLERANCE);
  EXPECT_NEAR(quad_set.weights[0], 5.0 / 9.0, EXP_NEAR_TOLERANCE);
  EXPECT_NEAR(quad_set.weights[1], 8.0 / 9.0, EXP_NEAR_TOLERANCE);
  EXPECT_NEAR(quad_set.weights[2], 5.0 / 9.0, EXP_NEAR_TOLERANCE);
}

// =======================================================================
// Invalid input
// =======================================================================

// KINGFISHER_ASSERT is compiled out when NDEBUG is defined.
#ifndef NDEBUG
TEST(GaussLegendre, NonPositiveNumberOfPointsThrows) {
  EXPECT_THROW(GaussLegendreNodes(0), Error);
  EXPECT_THROW(GaussLegendreNodes(-3), Error);
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
