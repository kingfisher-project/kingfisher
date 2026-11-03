// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "quadrature/quadrature.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <numbers>
#include <string>
#include <utility>
#include <vector>

#include "../tests/constants.h"
#include "helpers.h"
#include "quadrature/gauss_legendre.h"
#include "quadrature/gauss_lobatto_legendre.h"
#include "quadrature/trapezoid.h"
#include "utils/errors.h"
#include "utils/input_parameters.h"
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

// =======================================================================
// Quadrature type traits
//
// Everything the generic tests below need to know about a QuadSetType lives in
// TraitsFor(). To add a quadrature set:
//   1. Add its enumerator to QuadSetType. The build then fails in TraitsFor()
//      (-Werror=switch) until step 2 is done.
//   2. Add a case to TraitsFor() and the type to AllQuadSetTypes().
// Every Create1DQuadSet and SphericalQuadSet test then runs on it.
// =======================================================================

namespace {

struct QuadTypeTraits {
  QuadSetType type;
  std::string name;
  /// Whether Create1DQuadSet accepts any interval. If false, only [-1, 1].
  bool any_interval;
  /// Smallest number of points the rule can be built with.
  int min_points;
  /// Whether the error for smooth integrands decreases geometrically in n.
  bool spectral;
  /// Highest monomial degree integrated exactly on [-1, 1] by n points.
  int (*exact_degree)(int n);
  /// Highest trigonometric frequency m for which n points integrate
  /// cos(m x) and sin(m x) exactly over a full period [0, 2 pi]. -1 if the
  /// rule is not built for periodic integrands.
  int (*periodic_frequency)(int n);
  /// The rule built directly, bypassing Create1DQuadSet.
  QuadSet1D (*reference)(int n, double min, double max);
};

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch"
#endif
QuadTypeTraits TraitsFor(const QuadSetType type) {
  switch (type) {
    case QuadSetType::GaussLegendre:
      return {type,
              "GaussLegendre",
              false,
              1,
              true,
              [](int n) { return 2 * n - 1; },
              [](int) { return -1; },
              [](int n, double, double) { return GaussLegendre(n); }};
    case QuadSetType::GaussLobattoLegendre:
      return {type,
              "GaussLobattoLegendre",
              false,
              2,
              true,
              [](int n) { return 2 * n - 3; },
              [](int) { return -1; },
              [](int n, double, double) { return GaussLobattoLegendre(n); }};
    case QuadSetType::Trapezoid:
      return {type,
              "Trapezoid",
              true,
              2,
              false,
              [](int) { return 1; },
              [](int n) { return n - 2; },
              [](int n, double a, double b) { return Trapezoid(n, a, b); }};
  }
  KingfisherError("Unknown QuadSetType passed to TraitsFor.");
}
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

std::vector<QuadSetType> AllQuadSetTypes() {
  return {QuadSetType::GaussLegendre, QuadSetType::GaussLobattoLegendre,
          QuadSetType::Trapezoid};
}

QuadSetParams MakeParams(const QuadSetType type, const int n, const double min,
                         const double max) {
  return {type, static_cast<std::size_t>(n), min, max};
}

constexpr double kTwoPi = 2.0 * std::numbers::pi;

/// Tolerance for a quantity of size `expected` built from a few hundred
/// floating-point operations.
double Tolerance(const double expected) {
  return 100.0 * kExpNearTolerance * (1.0 + std::abs(expected));
}

/// Exact value of \f$\int_a^b x^k dx\f$.
double AnalyticMonomialIntegral(const int k, const double a, const double b) {
  return (std::pow(b, k + 1) - std::pow(a, k + 1)) / (k + 1);
}

}  // namespace

TEST(QuadTypeTraits, AllQuadSetTypesListsEveryEnumerator) {
  const std::vector<QuadSetType> listed = AllQuadSetTypes();
  int handled = 0;
  for (int value = 0; value < 64; value++) {
    const auto type = static_cast<QuadSetType>(value);
    std::optional<QuadTypeTraits> maybe;
    try {
      maybe = TraitsFor(type);
    } catch (const Error&) {
      continue;  // past the last enumerator
    }
    const QuadTypeTraits& traits = *maybe;
    handled++;
    EXPECT_EQ(traits.type, type) << traits.name;
    EXPECT_NE(std::find(listed.begin(), listed.end(), type), listed.end())
        << traits.name << " is missing from AllQuadSetTypes()";
    EXPECT_GE(traits.min_points, 1) << traits.name;
    EXPECT_GE(traits.exact_degree(traits.min_points), 0) << traits.name;
  }
  EXPECT_EQ(handled, static_cast<int>(listed.size()));
}

// =======================================================================
// Create1DQuadSet, for every QuadSetType and number of points
// =======================================================================

namespace {

struct Case1D {
  QuadSetType type;
  int n;
};

std::vector<Case1D> Create1DCases() {
  std::vector<Case1D> cases;
  for (const QuadSetType type : AllQuadSetTypes()) {
    // Create1DQuadSet only dispatches; the rules themselves are tested in
    // their own files, so the smallest, a middle, and a large size suffice.
    for (const int n : {TraitsFor(type).min_points, 5, 12}) {
      cases.push_back({type, n});
    }
  }
  return cases;
}

std::string Case1DName(const ::testing::TestParamInfo<Case1D>& info) {
  return TraitsFor(info.param.type).name + "_" + std::to_string(info.param.n);
}

}  // namespace

class Create1DQuadSetTest : public ::testing::TestWithParam<Case1D> {
 protected:
  QuadTypeTraits traits_ = TraitsFor(GetParam().type);
  int n_ = GetParam().n;
};

INSTANTIATE_TEST_SUITE_P(AllTypes, Create1DQuadSetTest,
                         ::testing::ValuesIn(Create1DCases()), Case1DName);

TEST_P(Create1DQuadSetTest, MatchesTheRuleBuiltDirectly) {
  std::vector<std::pair<double, double>> intervals = {{-1.0, 1.0}};
  if (traits_.any_interval) intervals.emplace_back(0.0, kTwoPi);
  for (const auto& [min, max] : intervals) {
    const QuadSet1D quad_set =
        Create1DQuadSet(MakeParams(traits_.type, n_, min, max));
    const QuadSet1D expected = traits_.reference(n_, min, max);
    ASSERT_EQ(quad_set.nodes.Size(), expected.nodes.Size());
    for (int i = 0; i < n_; i++) {
      EXPECT_DOUBLE_EQ(quad_set.nodes[i], expected.nodes[i])
          << "i=" << i << ", interval [" << min << ", " << max << "]";
      EXPECT_DOUBLE_EQ(quad_set.weights[i], expected.weights[i])
          << "i=" << i << ", interval [" << min << ", " << max << "]";
    }
  }
}

// Types restricted to [-1, 1] must reject other intervals instead of silently
// returning nodes on the wrong one. The others must honour the interval.
TEST_P(Create1DQuadSetTest, HandlesAnIntervalOtherThanTheUnitInterval) {
  const QuadSetParams params = MakeParams(traits_.type, n_, 0.0, kTwoPi);
  if (!traits_.any_interval) {
    EXPECT_THROW(Create1DQuadSet(params), Error);
    return;
  }
  const QuadSet1D quad_set = Create1DQuadSet(params);
  EXPECT_NEAR(quad_set.nodes[0], 0.0, kExpNearTolerance);
  EXPECT_NEAR(quad_set.nodes[n_ - 1], kTwoPi, kExpNearTolerance);
  for (int k = 0; k <= traits_.exact_degree(n_); k++) {
    const double exact = AnalyticMonomialIntegral(k, 0.0, kTwoPi);
    EXPECT_NEAR(
        IntegrateFunction([k](double x) { return std::pow(x, k); }, quad_set),
        exact, Tolerance(exact))
        << "k=" << k;
  }
}

// The point-count checks live in the rules and are KINGFISHER_ASSERTs, so they
// are compiled out under NDEBUG. Create1DQuadSet narrows n_points (size_t) to
// int, so a count that does not fit must not slip through as a valid one.
#ifndef NDEBUG
TEST_P(Create1DQuadSetTest, TooFewPointsThrow) {
  EXPECT_THROW(
      Create1DQuadSet(MakeParams(traits_.type, traits_.min_points - 1, -1, 1)),
      Error);
}

TEST_P(Create1DQuadSetTest, PointCountTooLargeForIntThrows) {
  QuadSetParams params = MakeParams(traits_.type, n_, -1.0, 1.0);
  params.n_points = static_cast<std::size_t>(std::numeric_limits<int>::max()) +
                    static_cast<std::size_t>(n_) + 1;
  EXPECT_THROW(Create1DQuadSet(params), Error);
}

TEST(Create1DQuadSet, EmptyIntervalThrowsForTrapezoid) {
  EXPECT_THROW(Create1DQuadSet(MakeParams(QuadSetType::Trapezoid, 4, 1.0, 1.0)),
               Error);
  EXPECT_THROW(Create1DQuadSet(MakeParams(QuadSetType::Trapezoid, 4, 1.0, 0.0)),
               Error);
}
#endif

TEST(Create1DQuadSet, UnsupportedTypeThrows) {
  EXPECT_THROW(
      Create1DQuadSet(MakeParams(static_cast<QuadSetType>(99), 4, -1.0, 1.0)),
      Error);
}

// =======================================================================
// SphericalQuadSet
//
// A direction on the unit sphere is Omega = (s cos(theta), s sin(theta), mu)
// with s = sqrt(1 - mu^2). theta is the first (fast) axis and mu the second.
// =======================================================================

namespace {

struct SphericalCase {
  QuadSetType polar;
  QuadSetType azimuthal;
  int n_polar;
  int n_azimuthal;
};

AngularParams MakeAngularParams(const SphericalCase& c) {
  AngularParams params;
  params.quad_type = SphericalQuadType::TensorProduct;
  params.polar_quad_set = MakeParams(c.polar, c.n_polar, -1.0, 1.0);
  params.azimuthal_quad_set =
      MakeParams(c.azimuthal, c.n_azimuthal, 0.0, kTwoPi);
  return params;
}

/// Types the azimuthal axis can use: they must accept [0, 2 pi].
std::vector<QuadSetType> AzimuthalTypes() {
  std::vector<QuadSetType> types;
  for (const QuadSetType type : AllQuadSetTypes()) {
    if (TraitsFor(type).any_interval) types.push_back(type);
  }
  return types;
}

/// Highest total degree d such that every monomial
/// Omega_x^a Omega_y^b Omega_z^c with a + b <= `azimuthal_degree` and
/// a + b + c <= `polar_degree` is integrated exactly. The azimuthal integrand
/// is a trigonometric polynomial of frequency a + b; the polar one is a
/// polynomial of degree a + b + c in mu.
int PolarDegree(const SphericalCase& c) {
  return TraitsFor(c.polar).exact_degree(c.n_polar);
}
int AzimuthalFrequency(const SphericalCase& c) {
  return TraitsFor(c.azimuthal).periodic_frequency(c.n_azimuthal);
}

/// Different point counts on the two axes, so that swapped or transposed axes
/// cannot pass by symmetry.
std::vector<SphericalCase> SphericalCases(const int min_total_degree = 0) {
  const std::vector<std::pair<int, int>> counts = {
      {2, 5}, {3, 8}, {5, 9}, {8, 13}, {12, 21}};
  std::vector<SphericalCase> cases;
  for (const QuadSetType polar : AllQuadSetTypes()) {
    for (const QuadSetType azimuthal : AzimuthalTypes()) {
      for (const auto& [n_polar, n_azimuthal] : counts) {
        const SphericalCase c{polar, azimuthal, n_polar, n_azimuthal};
        if (n_polar < TraitsFor(polar).min_points ||
            n_azimuthal < TraitsFor(azimuthal).min_points) {
          continue;
        }
        if (PolarDegree(c) >= min_total_degree &&
            AzimuthalFrequency(c) >= min_total_degree) {
          cases.push_back(c);
        }
      }
    }
  }
  return cases;
}

std::string SphericalCaseName(
    const ::testing::TestParamInfo<SphericalCase>& info) {
  const SphericalCase& c = info.param;
  return "Polar" + TraitsFor(c.polar).name + std::to_string(c.n_polar) +
         "_Azimuthal" + TraitsFor(c.azimuthal).name +
         std::to_string(c.n_azimuthal);
}

/// Grid values of f(Omega_x, Omega_y, Omega_z) in the SphericalQuadSet
/// ordering: values[j * n_azimuthal + i] is at (theta_i, mu_j).
mfem::Vector EvaluateOnSphere(
    const std::function<double(double, double, double)>& f,
    const QuadSet2D& quad_set) {
  const int n_theta = quad_set.first_axis_nodes.Size();
  const int n_mu = quad_set.second_axis_nodes.Size();
  mfem::Vector values(n_theta * n_mu);
  for (int j = 0; j < n_mu; j++) {
    const double mu = quad_set.second_axis_nodes[j];
    const double s = std::sqrt(std::max(0.0, 1.0 - mu * mu));
    for (int i = 0; i < n_theta; i++) {
      const double theta = quad_set.first_axis_nodes[i];
      values[j * n_theta + i] = f(s * std::cos(theta), s * std::sin(theta), mu);
    }
  }
  return values;
}

double IntegrateOnSphere(const std::function<double(double, double, double)>& f,
                         const QuadSet2D& quad_set) {
  return Integrate(EvaluateOnSphere(f, quad_set), quad_set.weights);
}

/// Exact value of \f$\int_{4\pi} x^a y^b z^c d\Omega\f$.
double AnalyticSphereMonomialIntegral(const int a, const int b, const int c) {
  if (a % 2 != 0 || b % 2 != 0 || c % 2 != 0) return 0.0;
  return 2.0 * std::tgamma((a + 1) / 2.0) * std::tgamma((b + 1) / 2.0) *
         std::tgamma((c + 1) / 2.0) / std::tgamma((a + b + c + 3) / 2.0);
}

}  // namespace

class SphericalFixture : public ::testing::TestWithParam<SphericalCase> {
 protected:
  void SetUp() override {
    quad_set_ = SphericalQuadSet(MakeAngularParams(GetParam()));
  }
  QuadSet2D quad_set_;
};

class SphericalQuadSetTest : public SphericalFixture {};
INSTANTIATE_TEST_SUITE_P(AllTypes, SphericalQuadSetTest,
                         ::testing::ValuesIn(SphericalCases()),
                         SphericalCaseName);

TEST_P(SphericalQuadSetTest, WeightsAreTheTensorProductOfTheAxisRules) {
  const SphericalCase& c = GetParam();
  const AngularParams params = MakeAngularParams(c);
  const QuadSet1D azimuthal = Create1DQuadSet(params.azimuthal_quad_set);
  const QuadSet1D polar = Create1DQuadSet(params.polar_quad_set);
  ASSERT_EQ(quad_set_.weights.Size(), c.n_azimuthal * c.n_polar);
  for (int j = 0; j < c.n_polar; j++) {
    for (int i = 0; i < c.n_azimuthal; i++) {
      EXPECT_DOUBLE_EQ(quad_set_.weights[j * c.n_azimuthal + i],
                       azimuthal.weights[i] * polar.weights[j])
          << "i=" << i << ", j=" << j;
      EXPECT_GT(quad_set_.weights[j * c.n_azimuthal + i], 0.0)
          << "i=" << i << ", j=" << j;
    }
  }
}

TEST_P(SphericalQuadSetTest, WeightsSumToTheAreaOfTheUnitSphere) {
  double sum = 0.0;
  for (int k = 0; k < quad_set_.weights.Size(); k++)
    sum += quad_set_.weights[k];
  EXPECT_NEAR(sum, 4.0 * std::numbers::pi, Tolerance(4.0 * std::numbers::pi));
}

// Every monomial Omega_x^a Omega_y^b Omega_z^c the rule is exact for:
// the closed form is nonzero only when a, b, and c are all even.
TEST_P(SphericalQuadSetTest, IntegratesMonomialsInTheDirectionCosinesExactly) {
  const SphericalCase& c = GetParam();
  int checked = 0;
  for (int a = 0; a <= 6; a++) {
    for (int b = 0; b <= 6; b++) {
      for (int k = 0; k <= 6; k++) {
        if (a + b > AzimuthalFrequency(c) || a + b + k > PolarDegree(c)) {
          continue;
        }
        const double exact = AnalyticSphereMonomialIntegral(a, b, k);
        const double result = IntegrateOnSphere(
            [a, b, k](double x, double y, double z) {
              return std::pow(x, a) * std::pow(y, b) * std::pow(z, k);
            },
            quad_set_);
        EXPECT_NEAR(result, exact, Tolerance(exact))
            << "a=" << a << ", b=" << b << ", c=" << k;
        checked++;
      }
    }
  }
  EXPECT_GT(checked, 0) << "no monomial is within this rule's exactness";
}

// int (Omega . n)^2 = 4 pi / 3 for every unit vector n. A tilted n mixes the
// azimuthal and polar rules, so an error in either shows up here. (The first
// and second moments are monomials and are covered by the exactness test; this
// is the rotation-invariance sanity check that transport codes rely on.)
TEST(SphericalQuadSet, SecondMomentAlongATiltedAxisIsRotationInvariant) {
  const QuadSet2D quad_set = SphericalQuadSet(MakeAngularParams(
      {QuadSetType::GaussLegendre, QuadSetType::Trapezoid, 8, 13}));
  const double nx = 1.0 / 3.0;
  const double ny = 2.0 / 3.0;
  const double nz = 2.0 / 3.0;
  const double expected = 4.0 * std::numbers::pi / 3.0;
  const double result = IntegrateOnSphere(
      [=](double x, double y, double z) {
        const double along = nx * x + ny * y + nz * z;
        return along * along;
      },
      quad_set);
  EXPECT_NEAR(result, expected, Tolerance(expected));
}

// -----------------------------------------------------------------------
// Convergence for a smooth, non-polynomial integrand:
//   int exp(k . Omega) dOmega = 4 pi sinh(|k|) / |k|
// -----------------------------------------------------------------------

namespace {

constexpr double kKx = 0.9;
constexpr double kKy = -1.2;
constexpr double kKz = 1.5;

double ExponentialIntegralOnSphere() {
  const double norm = std::sqrt(kKx * kKx + kKy * kKy + kKz * kKz);
  return 4.0 * std::numbers::pi * std::sinh(norm) / norm;
}

double ExponentialError(const SphericalCase& c) {
  const QuadSet2D quad_set = SphericalQuadSet(MakeAngularParams(c));
  const double result = IntegrateOnSphere(
      [](double x, double y, double z) {
        return std::exp(kKx * x + kKy * y + kKz * z);
      },
      quad_set);
  return std::abs(result - ExponentialIntegralOnSphere());
}

std::vector<std::pair<QuadSetType, QuadSetType>> TypePairs() {
  std::vector<std::pair<QuadSetType, QuadSetType>> pairs;
  const std::vector<QuadSetType> azimuthal_types = AzimuthalTypes();
  pairs.reserve(AllQuadSetTypes().size() * azimuthal_types.size());
  for (const QuadSetType polar : AllQuadSetTypes()) {
    for (const QuadSetType azimuthal : azimuthal_types) {
      pairs.emplace_back(polar, azimuthal);
    }
  }
  return pairs;
}

std::string TypePairName(
    const ::testing::TestParamInfo<std::pair<QuadSetType, QuadSetType>>& info) {
  return "Polar" + TraitsFor(info.param.first).name + "_Azimuthal" +
         TraitsFor(info.param.second).name;
}

}  // namespace

class SphericalQuadSetConvergenceTest
    : public ::testing::TestWithParam<std::pair<QuadSetType, QuadSetType>> {};

INSTANTIATE_TEST_SUITE_P(AllTypePairs, SphericalQuadSetConvergenceTest,
                         ::testing::ValuesIn(TypePairs()), TypePairName);

TEST_P(SphericalQuadSetConvergenceTest, ErrorDecreasesAsPointsAreAdded) {
  const auto [polar, azimuthal] = GetParam();
  const std::vector<std::pair<int, int>> counts = {
      {4, 5}, {8, 9}, {16, 17}, {32, 33}};
  std::vector<double> errors;
  errors.reserve(counts.size());
  for (const auto& [n_polar, n_azimuthal] : counts) {
    errors.push_back(
        ExponentialError({polar, azimuthal, n_polar, n_azimuthal}));
  }
  const double round_off = 1e-12 * ExponentialIntegralOnSphere();
  for (std::size_t k = 0; k + 1 < errors.size(); k++) {
    if (errors[k] < round_off) {
      // Already converged: the next refinement must stay converged.
      EXPECT_LT(errors[k + 1], 10.0 * round_off) << "refinement " << k;
      continue;
    }
    EXPECT_LT(errors[k + 1], errors[k]) << "refinement " << k;
  }
  EXPECT_LT(errors.back(), std::max(errors.front() / 10.0, round_off));
}

// The trapezoid rule is second order, so with the (spectrally accurate)
// azimuthal axis held fixed, halving the polar spacing cuts the error by 4.
TEST(SphericalQuadSet, TrapezoidPolarAxisConvergesAtSecondOrder) {
  const std::vector<int> polar_counts = {9, 17, 33, 65};
  std::vector<double> errors;
  errors.reserve(polar_counts.size());
  for (const int n_polar : polar_counts) {
    errors.push_back(ExponentialError(
        {QuadSetType::Trapezoid, QuadSetType::Trapezoid, n_polar, 41}));
  }
  for (std::size_t k = 0; k + 1 < errors.size(); k++) {
    const double order = std::log2(errors[k] / errors[k + 1]);
    EXPECT_NEAR(order, 2.0, 0.15) << "refinement " << k;
  }
}

// Gauss-type polar rules and a periodic trapezoid azimuthal rule converge
// geometrically for a smooth integrand, so a modest number of points reaches
// round-off.
TEST_P(SphericalQuadSetConvergenceTest, SpectralPairsReachRoundOff) {
  const auto [polar, azimuthal] = GetParam();
  const int n_azimuthal = 25;
  if (!TraitsFor(polar).spectral ||
      TraitsFor(azimuthal).periodic_frequency(n_azimuthal) < n_azimuthal - 2) {
    GTEST_SKIP() << "not a spectral pairing";
  }
  EXPECT_LT(ExponentialError({polar, azimuthal, 24, n_azimuthal}),
            1e-11 * ExponentialIntegralOnSphere());
}

// -----------------------------------------------------------------------
// Errors
// -----------------------------------------------------------------------

TEST(SphericalQuadSet, UnsupportedSphericalQuadTypeThrows) {
  AngularParams params = MakeAngularParams(
      {QuadSetType::GaussLegendre, QuadSetType::Trapezoid, 4, 5});
  params.quad_type = static_cast<SphericalQuadType>(99);
  EXPECT_THROW(SphericalQuadSet(params), Error);
}

// Gauss-Legendre only exists on [-1, 1], so it cannot integrate over theta in
// [0, 2 pi].
TEST(SphericalQuadSet, AzimuthalSetThatCannotCoverTwoPiThrows) {
  const AngularParams params = MakeAngularParams(
      {QuadSetType::GaussLegendre, QuadSetType::GaussLegendre, 4, 5});
  EXPECT_THROW(SphericalQuadSet(params), Error);
}

// The 2D set integrates over mu in [-1, 1] and theta in [0, 2 pi]. Rules that
// accept any interval (Trapezoid) must not silently be built on another one.
TEST(SphericalQuadSet, PolarSetNotOnMinusOneToOneThrows) {
  AngularParams params =
      MakeAngularParams({QuadSetType::Trapezoid, QuadSetType::Trapezoid, 5, 9});
  params.polar_quad_set.min = 0.0;
  params.polar_quad_set.max = 1.0;
  EXPECT_THROW(SphericalQuadSet(params), Error);
}

TEST(SphericalQuadSet, AzimuthalSetNotOnZeroToTwoPiThrows) {
  AngularParams params = MakeAngularParams(
      {QuadSetType::GaussLegendre, QuadSetType::Trapezoid, 4, 9});
  params.azimuthal_quad_set.min = 0.0;
  params.azimuthal_quad_set.max = std::numbers::pi;
  EXPECT_THROW(SphericalQuadSet(params), Error);
}

}  // namespace kingfisher
