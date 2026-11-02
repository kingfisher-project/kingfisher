// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/trapezoid.h"

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

#include "../tests/constants.h"
#include "helpers.h"
#include "quadrature/quadrature.h"
#include "utils/errors.h"
#include "utils/numerics.h"

namespace kingfisher {

// =======================================================================
// Properties that must hold for every number of points (n >= 2) on the
// default interval [-1, 1]
// =======================================================================

class TrapezoidTest : public ::testing::TestWithParam<int> {};

INSTANTIATE_TEST_SUITE_P(PointCounts, TrapezoidTest,
                         ::testing::Values(2, 3, 4, 5, 10, 11, 50));

TEST_P(TrapezoidTest, NodesAndWeightsHaveNPoints) {
  const int n = GetParam();
  const QuadSet1D quad_set = Trapezoid(n);
  EXPECT_EQ(quad_set.nodes.Size(), n);
  EXPECT_EQ(quad_set.weights.Size(), n);
}

TEST_P(TrapezoidTest, NodesAreUniformlySpacedFromMinusOneToOne) {
  const int n = GetParam();
  const QuadSet1D quad_set = Trapezoid(n);
  const double h = 2.0 / (n - 1);
  EXPECT_EQ(quad_set.nodes[0], -1.0);
  EXPECT_NEAR(quad_set.nodes[n - 1], 1.0, kExpNearTolerance);
  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(quad_set.nodes[i], -1.0 + h * i, kExpNearTolerance)
        << "i=" << i;
    EXPECT_NEAR(quad_set.nodes[i], -quad_set.nodes[n - 1 - i],
                kExpNearTolerance)
        << "i=" << i;
    if (i > 0) EXPECT_LT(quad_set.nodes[i - 1], quad_set.nodes[i]);
  }
}

// Interior weights are h; the two end weights are h / 2.
TEST_P(TrapezoidTest, WeightsArePositiveAndHalvedAtTheEnds) {
  const int n = GetParam();
  const QuadSet1D quad_set = Trapezoid(n);
  const double h = 2.0 / (n - 1);
  double sum = 0.0;
  for (int i = 0; i < n; i++) {
    const double expected = (i == 0 || i == n - 1) ? 0.5 * h : h;
    EXPECT_GT(quad_set.weights[i], 0.0) << "i=" << i;
    EXPECT_NEAR(quad_set.weights[i], expected, kExpNearTolerance)
        << "n=" << n << ", i=" << i;
    sum += quad_set.weights[i];
  }
  EXPECT_NEAR(sum, 2.0, kExpNearTolerance) << "n=" << n;
}

// Degree of exactness is 1: constants and linear functions are integrated
// exactly, but x^2 is not (guards against the rule silently becoming Simpson).
TEST_P(TrapezoidTest, ExactForDegreeZeroAndOneButNotTwo) {
  const int n = GetParam();
  const QuadSet1D quad_set = Trapezoid(n);
  for (int k = 0; k <= 1; k++) {
    const double result =
        IntegrateFunction([k](double x) { return std::pow(x, k); }, quad_set);
    EXPECT_NEAR(result, AnalyticMonomialIntegral(k), kExpNearTolerance)
        << "n=" << n << ", k=" << k;
  }
  const double result =
      IntegrateFunction([](double x) { return x * x; }, quad_set);
  EXPECT_GT(std::abs(result - AnalyticMonomialIntegral(2)), 1e-3 / (n * n));
}

// For f = x^2 (f'' = 2) the composite trapezoid error on [a, b] is exactly
// (b - a) h^2 f'' / 12, which is h^2 / 3 on [-1, 1].
TEST_P(TrapezoidTest, QuadraticErrorMatchesTheory) {
  const int n = GetParam();
  const QuadSet1D quad_set = Trapezoid(n);
  const double h = 2.0 / (n - 1);
  const double result =
      IntegrateFunction([](double x) { return x * x; }, quad_set);
  EXPECT_NEAR(result - 2.0 / 3.0, h * h / 3.0, kExpNearTolerance) << "n=" << n;
}

TEST_P(TrapezoidTest, TrapezoidMatchesInPlaceFunction) {
  const int n = GetParam();
  const QuadSet1D quad_set = Trapezoid(n);
  QuadSet1D expected;
  TrapezoidNodesAndWeights(expected, n, -1.0, 1.0);
  ASSERT_EQ(quad_set.nodes.Size(), n);
  ASSERT_EQ(quad_set.weights.Size(), n);
  for (int i = 0; i < n; i++) {
    EXPECT_EQ(quad_set.nodes[i], expected.nodes[i]);
    EXPECT_EQ(quad_set.weights[i], expected.weights[i]);
  }
}

// The in-place function must fully replace whatever the set held before, even
// if it had a different size.
TEST_P(TrapezoidTest, InPlaceFunctionOverwritesExistingSet) {
  const int n = GetParam();
  const QuadSet1D expected = Trapezoid(n);

  QuadSet1D reused = Trapezoid(n + 3);
  TrapezoidNodesAndWeights(reused, n, -1.0, 1.0);

  ASSERT_EQ(reused.nodes.Size(), n);
  ASSERT_EQ(reused.weights.Size(), n);
  for (int i = 0; i < n; i++) {
    EXPECT_EQ(reused.nodes[i], expected.nodes[i]);
    EXPECT_EQ(reused.weights[i], expected.weights[i]);
  }
}

// =======================================================================
// Closed-form low-order rules
// =======================================================================

TEST(Trapezoid, TwoPointRule) {
  const QuadSet1D quad_set = Trapezoid(2);
  EXPECT_DOUBLE_EQ(quad_set.nodes[0], -1.0);
  EXPECT_DOUBLE_EQ(quad_set.nodes[1], 1.0);
  EXPECT_DOUBLE_EQ(quad_set.weights[0], 1.0);
  EXPECT_DOUBLE_EQ(quad_set.weights[1], 1.0);
}

TEST(Trapezoid, ThreePointRule) {
  const QuadSet1D quad_set = Trapezoid(3);
  const double expected_nodes[] = {-1.0, 0.0, 1.0};
  const double expected_weights[] = {0.5, 1.0, 0.5};
  for (int i = 0; i < 3; i++) {
    EXPECT_DOUBLE_EQ(quad_set.nodes[i], expected_nodes[i]);
    EXPECT_DOUBLE_EQ(quad_set.weights[i], expected_weights[i]);
  }
}

TEST(Trapezoid, FivePointRule) {
  const QuadSet1D quad_set = Trapezoid(5);
  const double expected_nodes[] = {-1.0, -0.5, 0.0, 0.5, 1.0};
  const double expected_weights[] = {0.25, 0.5, 0.5, 0.5, 0.25};
  for (int i = 0; i < 5; i++) {
    EXPECT_DOUBLE_EQ(quad_set.nodes[i], expected_nodes[i]);
    EXPECT_DOUBLE_EQ(quad_set.weights[i], expected_weights[i]);
  }
}

// =======================================================================
// Non-default intervals
// =======================================================================

// The azimuthal quadrature set lives on [0, 2*pi].
TEST(Trapezoid, AzimuthalIntervalEndpointsAndWeightSum) {
  const double two_pi = 2.0 * std::numbers::pi;
  const QuadSet1D quad_set = Trapezoid(9, 0.0, two_pi);
  EXPECT_EQ(quad_set.nodes[0], 0.0);
  EXPECT_NEAR(quad_set.nodes[8], two_pi, kExpNearTolerance);
  double sum = 0.0;
  for (int i = 0; i < 9; i++) sum += quad_set.weights[i];
  EXPECT_NEAR(sum, two_pi, kExpNearTolerance);
}

// For periodic integrands the trapezoid rule is exact for trigonometric
// polynomials up to degree n - 2 (here: n = 9 points, 8 intervals).
TEST(Trapezoid, ExactForPeriodicTrigonometricPolynomialsOnZeroToTwoPi) {
  const QuadSet1D quad_set = Trapezoid(9, 0.0, 2.0 * std::numbers::pi);
  EXPECT_NEAR(IntegrateFunction([](double x) { return std::sin(x); }, quad_set),
              0.0, kExpNearTolerance);
  EXPECT_NEAR(IntegrateFunction(
                  [](double x) { return std::cos(x) * std::cos(x); }, quad_set),
              std::numbers::pi, kExpNearTolerance);
  EXPECT_NEAR(
      IntegrateFunction([](double x) { return std::cos(3.0 * x); }, quad_set),
      0.0, kExpNearTolerance);
}

// Asymmetric interval that does not contain zero: catches swapped a/b or a
// wrong offset. The rule is exact for linear integrands.
TEST(Trapezoid, LinearFunctionOnAsymmetricInterval) {
  const QuadSet1D quad_set = Trapezoid(7, 2.0, 5.0);
  EXPECT_EQ(quad_set.nodes[0], 2.0);
  EXPECT_NEAR(quad_set.nodes[6], 5.0, kExpNearTolerance);
  // \int_2^5 (3x - 1) dx = 1.5 (25 - 4) - 3 = 28.5
  EXPECT_NEAR(
      IntegrateFunction([](double x) { return 3.0 * x - 1.0; }, quad_set), 28.5,
      kExpNearTolerance);
}

// =======================================================================
// Invalid input
// =======================================================================

// KINGFISHER_ASSERT is compiled out when NDEBUG is defined.
#ifndef NDEBUG
TEST(Trapezoid, FewerThanTwoPointsThrows) {
  QuadSet1D quad_set;
  EXPECT_THROW(TrapezoidNodesAndWeights(quad_set, 1, -1.0, 1.0), Error);
  EXPECT_THROW(TrapezoidNodesAndWeights(quad_set, 0, -1.0, 1.0), Error);
  EXPECT_THROW(TrapezoidNodesAndWeights(quad_set, -3, -1.0, 1.0), Error);
  EXPECT_THROW(Trapezoid(1), Error);
}

TEST(Trapezoid, EmptyOrReversedIntervalThrows) {
  EXPECT_THROW(Trapezoid(5, 1.0, 1.0), Error);
  EXPECT_THROW(Trapezoid(5, 1.0, -1.0), Error);
}
#endif

// =======================================================================
// Convergence
// =======================================================================

// \int_{-1}^{1} e^x dx = e - 1/e. The error is O(h^2), so halving h should
// divide it by about 4.
TEST(Trapezoid, ExponentialIntegralConvergesSecondOrder) {
  const double expected = std::exp(1.0) - std::exp(-1.0);
  const auto exponential = [](double x) { return std::exp(x); };

  double previous_error = 0.0;
  for (const int n : {5, 9, 17, 33}) {
    const double error =
        std::abs(IntegrateFunction(exponential, Trapezoid(n)) - expected);
    if (previous_error > 0.0) {
      EXPECT_NEAR(previous_error / error, 4.0, 0.2) << "n=" << n;
    }
    previous_error = error;
  }
}

}  // namespace kingfisher
