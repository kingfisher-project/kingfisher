// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/quadrature.h"

#include <gtest/gtest.h>

#include <functional>

#include "../tests/constants.h"
#include "helpers.h"
#include "quadrature/gauss_legendre.h"
#include "quadrature/trapezoid.h"
#include "utils/errors.h"
#include "utils/numerics.h"

namespace kingfisher {

// =======================================================================
// Integrate
// =======================================================================

TEST(Integrate, IsTheDotProductOfValuesAndWeights) {
  const mfem::Vector values{1.0, 2.0, 3.0};
  const mfem::Vector weights{0.5, 0.25, 2.0};
  EXPECT_DOUBLE_EQ(Integrate(values, weights), 0.5 + 0.5 + 6.0);
}

TEST(Integrate, EmptyVectorsIntegrateToZero) {
  const mfem::Vector empty;
  EXPECT_EQ(Integrate(empty, empty), 0.0);
}

TEST(Integrate, IntegratesQuadraticWithTwoPointGaussLegendre) {
  EXPECT_NEAR(
      IntegrateFunction([](double x) { return x * x; }, GaussLegendre(2)),
      2.0 / 3.0, kExpNearTolerance);
}

// KINGFISHER_ASSERT is compiled out when NDEBUG is defined.
#ifndef NDEBUG
TEST(Integrate, MismatchedSizesThrow) {
  const mfem::Vector three{1.0, 2.0, 3.0};
  const mfem::Vector two{1.0, 2.0};
  EXPECT_THROW(Integrate(three, two), Error);
  EXPECT_THROW(Integrate(two, three), Error);
}
#endif

// =======================================================================
// Create2DQuadSet
// =======================================================================

namespace {

// Grid values of f(x, y) in the Create2DQuadSet ordering: the first axis is
// the fast index, so values[j * Nx + i] = f(x_i, y_j).
mfem::Vector Evaluate2D(const std::function<double(double, double)>& f,
                        const QuadSet1D& first_axis,
                        const QuadSet1D& second_axis) {
  const int nx = first_axis.nodes.Size();
  const int ny = second_axis.nodes.Size();
  mfem::Vector values(nx * ny);
  for (int j = 0; j < ny; j++) {
    for (int i = 0; i < nx; i++) {
      values[j * nx + i] = f(first_axis.nodes[i], second_axis.nodes[j]);
    }
  }
  return values;
}

}  // namespace

// Different rules and different sizes on each axis, so that a transposed or
// swapped construction cannot pass by symmetry.
class Create2DQuadSetTest : public ::testing::Test {
 protected:
  const QuadSet1D first_axis_ = GaussLegendre(3);
  const QuadSet1D second_axis_ = Trapezoid(5);
};

TEST_F(Create2DQuadSetTest, HasOneWeightPerPointPair) {
  const QuadSet2D quad_set = Create2DQuadSet(first_axis_, second_axis_);
  EXPECT_EQ(quad_set.weights.Size(), 3 * 5);
}

TEST_F(Create2DQuadSetTest, WeightsAreTheTensorProductWithFirstAxisFastest) {
  const QuadSet2D quad_set = Create2DQuadSet(first_axis_, second_axis_);
  ASSERT_EQ(quad_set.weights.Size(), 15);
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < 3; i++) {
      EXPECT_DOUBLE_EQ(quad_set.weights[j * 3 + i],
                       first_axis_.weights[i] * second_axis_.weights[j])
          << "i=" << i << ", j=" << j;
    }
  }
}

TEST_F(Create2DQuadSetTest, WeightsSumToProductOfAxisIntervals) {
  const QuadSet2D quad_set = Create2DQuadSet(first_axis_, second_axis_);
  double sum = 0.0;
  for (int k = 0; k < quad_set.weights.Size(); k++) sum += quad_set.weights[k];
  EXPECT_NEAR(sum, 4.0, kExpNearTolerance);
}

// x^2 y^0 is not symmetric under x <-> y, so this fails if the grid ordering
// or the axes are transposed.
TEST_F(Create2DQuadSetTest, IntegratesSeparablePolynomialsExactly) {
  const QuadSet2D quad_set = Create2DQuadSet(first_axis_, second_axis_);
  // Gauss-Legendre (3 points) is exact through x^5; the trapezoid axis is
  // exact only through y^1, so keep the y-degree at 0 or 1.
  for (int a = 0; a <= 5; a++) {
    for (int b = 0; b <= 1; b++) {
      const mfem::Vector values = Evaluate2D(
          [a, b](double x, double y) {
            return std::pow(x, a) * std::pow(y, b);
          },
          first_axis_, second_axis_);
      EXPECT_NEAR(Integrate(values, quad_set.weights),
                  AnalyticMonomialIntegral(a) * AnalyticMonomialIntegral(b),
                  kExpNearTolerance)
          << "a=" << a << ", b=" << b;
    }
  }
}

TEST_F(Create2DQuadSetTest, IntegratesNonSeparableFunctionExactly) {
  const QuadSet2D quad_set = Create2DQuadSet(first_axis_, second_axis_);
  // \int\int (x^2 + 3 x y + y + 1) = 2/3 * 2 + 0 + 0 + 4 = 16/3
  const mfem::Vector values = Evaluate2D(
      [](double x, double y) { return x * x + 3.0 * x * y + y + 1.0; },
      first_axis_, second_axis_);
  EXPECT_NEAR(Integrate(values, quad_set.weights), 16.0 / 3.0,
              kExpNearTolerance);
}

TEST_F(Create2DQuadSetTest, StoresEachAxisNodes) {
  const QuadSet2D quad_set = Create2DQuadSet(first_axis_, second_axis_);
  ASSERT_EQ(quad_set.first_axis_nodes.Size(), 3);
  ASSERT_EQ(quad_set.second_axis_nodes.Size(), 5);
  for (int i = 0; i < 3; i++) {
    EXPECT_EQ(quad_set.first_axis_nodes[i], first_axis_.nodes[i]);
  }
  for (int j = 0; j < 5; j++) {
    EXPECT_EQ(quad_set.second_axis_nodes[j], second_axis_.nodes[j]);
  }
}

TEST(Create2DQuadSet, SinglePointAxisGivesOneWeightPerPointOfOtherAxis) {
  const QuadSet1D line = GaussLegendre(4);
  const QuadSet1D point = GaussLegendre(1);
  const QuadSet2D quad_set = Create2DQuadSet(line, point);
  ASSERT_EQ(quad_set.weights.Size(), 4);
  for (int i = 0; i < 4; i++) {
    EXPECT_DOUBLE_EQ(quad_set.weights[i], line.weights[i] * point.weights[0]);
  }
}

}  // namespace kingfisher
