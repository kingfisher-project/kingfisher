// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, Liam Pohlmann
// Copyright (c) 2026, kingfisher-project

#include "utils/legendre_polynomials.h"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

#include "utils/numerics.h"

namespace kingfisher {

// Representative x values in the open interval, plus endpoints.
const mfem::Vector kInteriorX{
    -0.9, -0.5, -0.1, 0.0, 0.1, 0.5, 1.0 / std::sqrt(2.0), 0.9};
const mfem::Vector kAllX{
    -1.0, -0.9, -0.5, -0.1, 0.0, 0.1, 0.5, 1.0 / std::sqrt(2.0), 0.9, 1.0};

// =======================================================================
// LegendrePolynomial(n, x)
// =======================================================================

TEST(LegendrePolynomial, P0IsOne) {
  for (double x : kAllX) {
    EXPECT_DOUBLE_EQ(LegendrePolynomial(0, x), 1.0);
  }
}

TEST(LegendrePolynomial, P1IsX) {
  for (double x : kAllX) {
    EXPECT_DOUBLE_EQ(LegendrePolynomial(1, x), x);
  }
}

TEST(LegendrePolynomial, P2ClosedForm) {
  for (double x : kAllX) {
    double expected = 0.5 * (3.0 * x * x - 1.0);
    EXPECT_DOUBLE_EQ(LegendrePolynomial(2, x), expected);
  }
}

TEST(LegendrePolynomial, P3ClosedForm) {
  for (double x : kAllX) {
    double expected = 0.5 * (5.0 * x * x * x - 3.0 * x);
    EXPECT_NEAR(LegendrePolynomial(3, x), expected, TOLERANCE);
  }
}

TEST(LegendrePolynomial, P4ClosedForm) {
  for (double x : kAllX) {
    double x2 = x * x;
    double expected = (35.0 * x2 * x2 - 30.0 * x2 + 3.0) / 8.0;
    EXPECT_NEAR(LegendrePolynomial(4, x), expected, 1e-15);
  }
}

TEST(LegendrePolynomial, P5ClosedForm) {
  for (double x : kAllX) {
    double x2 = x * x;
    double x3 = x2 * x;
    double x5 = x3 * x2;
    double expected = (63.0 * x5 - 70.0 * x3 + 15.0 * x) / 8.0;
    EXPECT_NEAR(LegendrePolynomial(5, x), expected, 1e-15);
  }
}

TEST(LegendrePolynomial, EndpointValues) {
  for (int n = 0; n <= 10; ++n) {
    EXPECT_DOUBLE_EQ(LegendrePolynomial(n, 1.0), 1.0) << "n=" << n;
    double expected_minus_one = (n % 2 == 0) ? 1.0 : -1.0;
    EXPECT_DOUBLE_EQ(LegendrePolynomial(n, -1.0), expected_minus_one)
        << "n=" << n;
  }
}

// Bonnet's recursion, as an independent sanity check of the recursive
// implementation across orders: (n+1) P_{n+1} = (2n+1) x P_n - n P_{n-1}
TEST(LegendrePolynomial, BonnetRecurrence) {
  for (double x : kAllX) {
    for (int n = 1; n <= 9; ++n) {
      double lhs = (n + 1) * LegendrePolynomial(n + 1, x);
      double rhs = (2 * n + 1) * x * LegendrePolynomial(n, x) -
                   n * LegendrePolynomial(n - 1, x);
      EXPECT_DOUBLE_EQ(lhs, rhs);
    }
  }
}

TEST(LegendrePolynomial, ParitySymmetry) {
  for (double x : kAllX) {
    for (int n = 0; n <= 8; ++n) {
      double sign = (n % 2 == 0) ? 1.0 : -1.0;
      EXPECT_DOUBLE_EQ(LegendrePolynomial(n, -x),
                       sign * LegendrePolynomial(n, x));
    }
  }
}

// =======================================================================
// LegendrePolynomialPrime(n, x)
// =======================================================================

TEST(LegendrePolynomialPrime, D0IsZero) {
  for (double x : kAllX) {
    EXPECT_DOUBLE_EQ(LegendrePolynomialPrime(0, x), 0.0);
  }
}

// P_1(x) = x, so P_1'(x) should be identically 1 everywhere, including at
// x = 0.5 where 1/(1-x^2) = 4/3 != 1. This test is expected to fail
// against the current implementation, which hardcodes the n==1 case as
// 1.0 / (1.0 - x * x) instead of falling through to the general formula
// (which itself simplifies to 1 for n=1).
TEST(LegendrePolynomialPrime, D1IsOne) {
  for (double x : kInteriorX) {
    EXPECT_DOUBLE_EQ(LegendrePolynomialPrime(1, x), 1.0);
  }
}

TEST(LegendrePolynomialPrime, D2ClosedForm) {
  for (double x : kInteriorX) {
    EXPECT_DOUBLE_EQ(LegendrePolynomialPrime(2, x), 3.0 * x);
  }
}

TEST(LegendrePolynomialPrime, D3ClosedForm) {
  for (double x : kInteriorX) {
    double expected = 0.5 * (15.0 * x * x - 3.0);
    EXPECT_DOUBLE_EQ(LegendrePolynomialPrime(3, x), expected);
  }
}

TEST(LegendrePolynomialPrime, D4ClosedForm) {
  for (double x : kInteriorX) {
    double expected = 0.5 * (35.0 * x * x * x - 15.0 * x);
    EXPECT_DOUBLE_EQ(LegendrePolynomialPrime(4, x), expected);
  }
}

// Cross-check the derivative against central finite differences of
// LegendrePolynomial itself. Restricted to the interior since the
// closed-form recurrence divides by (1 - x^2), which is singular at the
// endpoints.
TEST(LegendrePolynomialPrime, MatchesFiniteDifference) {
  const double h = 1e-6;
  for (int n = 0; n <= 8; ++n) {
    for (double x : kInteriorX) {
      double numerical =
          (LegendrePolynomial(n, x + h) - LegendrePolynomial(n, x - h)) /
          (2.0 * h);
      double analytic = LegendrePolynomialPrime(n, x);
      EXPECT_NEAR(analytic, numerical, 1e-9);
    }
  }
}

// Legendre's differential equation:
//   (1 - x^2) P_n''(x) - 2x P_n'(x) + n(n+1) P_n(x) = 0
// P_n'' is approximated via a central finite difference of
// LegendrePolynomialPrime, so this also cross-validates the two
// functions against each other through an independent identity.
TEST(LegendrePolynomialPrime, SatisfiesLegendreODE) {
  const double h = 1e-4;
  for (int n = 0; n <= 6; ++n) {
    for (double x : kInteriorX) {
      double d1 = LegendrePolynomialPrime(n, x);
      double d2 =
          (LegendrePolynomial(n, x + h) - 2.0 * LegendrePolynomial(n, x) +
           LegendrePolynomial(n, x - h)) /
          (h * h);
      double lhs = (1.0 - x * x) * d2 - 2.0 * x * d1 +
                   n * (n + 1) * LegendrePolynomial(n, x);
      EXPECT_NEAR(lhs, 0.0, 1e-6);
    }
  }
}

// Endpoint derivative values: P_n'(1) = n(n+1)/2, P_n'(-1) = (-1)^(n+1) *
// n(n+1)/2.
TEST(LegendrePolynomialPrime, EndpointDerivativeAtPlusOne) {
  for (int n = 0; n <= 10; ++n) {
    double expected = n * (n + 1) / 2.0;
    EXPECT_DOUBLE_EQ(LegendrePolynomialPrime(n, 1.0), expected) << "n=" << n;
  }
}

TEST(LegendrePolynomialPrime, EndpointDerivativeAtMinusOne) {
  for (int n = 0; n <= 10; ++n) {
    double sign = (n % 2 == 0) ? -1.0 : 1.0;
    double expected = sign * n * (n + 1) / 2.0;
    EXPECT_DOUBLE_EQ(LegendrePolynomialPrime(n, -1.0), expected) << "n=" << n;
  }
}

void TestRoots(const mfem::Vector& expected_roots) {
  int order = expected_roots.Size();
  auto computed_roots = AllLegendreRoots(order);
  for (auto i = 0; i < order; i++)
    EXPECT_NEAR(computed_roots[i], expected_roots[i], TOLERANCE);
}

TEST(LegendrePolynomialRootFinding, P1) {
  mfem::Vector p1_roots{0.0};
  TestRoots(p1_roots);
}

TEST(LegendrePolynomialRootFinding, P2) {
  mfem::Vector p2_roots{-0.5773502691896257, 0.5773502691896257};
  TestRoots(p2_roots);
}
TEST(LegendrePolynomialRootFinding, P4) {
  mfem::Vector p4_roots{-0.8611363115940526, -0.3399810435848563,
                           0.3399810435848563, 0.8611363115940526};
  TestRoots(p4_roots);
}

TEST(LegendrePolynomialRootFinding, P5) {
  mfem::Vector p5_roots{-0.906179845938664, -0.5384693101056831, 0.0,
                           0.5384693101056831, 0.906179845938664};
  TestRoots(p5_roots);
}

TEST(LegendrePolynomialRootFinding, P8) {
  mfem::Vector p8_roots{-0.9602898564975363, -0.7966664774136267,
                           -0.525532409916329,  -0.18343464249564984,
                           0.18343464249564984, 0.525532409916329,
                           0.7966664774136267,  0.9602898564975363};
  TestRoots(p8_roots);
}

TEST(LegendrePolynomialRootFinding, P16) {
  mfem::Vector p16_roots{
      -0.9894009349916499, -0.9445750230732326,  -0.8656312023878318,
      -0.755404408355003,  -0.6178762444026438,  -0.4580167776572274,
      -0.2816035507792589, -0.09501250983763745, 0.09501250983763745,
      0.2816035507792589,  0.4580167776572274,   0.6178762444026438,
      0.755404408355003,   0.8656312023878318,   0.9445750230732326,
      0.9894009349916499};
  TestRoots(p16_roots);
}

TEST(LegendrePolynomialRootFinding, P32) {
  mfem::Vector p32_roots{
      -0.9972638618494816,  -0.9856115115452684, -0.9647622555875064,
      -0.9349060759377397,  -0.8963211557660521, -0.8493676137325699,
      -0.7944837959679423,  -0.7321821187402897, -0.6630442669302152,
      -0.5877157572407623,  -0.5068999089322295, -0.4213512761306354,
      -0.33186860228212767, -0.2392873622521371, -0.14447196158279643,
      -0.04830766568773834, 0.04830766568773834, 0.14447196158279643,
      0.2392873622521371,   0.33186860228212767, 0.4213512761306354,
      0.5068999089322295,   0.5877157572407623,  0.6630442669302152,
      0.7321821187402897,   0.7944837959679423,  0.8493676137325699,
      0.8963211557660521,   0.9349060759377397,  0.9647622555875064,
      0.9856115115452684,   0.9972638618494816};
  TestRoots(p32_roots);
}

TEST(LegendrePolynomialRootFinding, P51) {
  mfem::Vector p51_roots{-0.9989099908489034,  -0.9942612604367524,
                            -0.985915991735903,   -0.9739033680193239,
                            -0.9582678486139082,  -0.9390675440029624,
                            -0.9163738623097802,  -0.8902712180295274,
                            -0.8608567111822925,  -0.828239763823065,
                            -0.7925417120993812,  -0.7538953544853755,
                            -0.7124444575770366,  -0.66834322117537,
                            -0.6217557046007232,  -0.5728552163513039,
                            -0.5218236693661858,  -0.4688509042860411,
                            -0.4141339832263038,  -0.3578764566884095,
                            -0.300287606335332,   -0.24158166644779872,
                            -0.18197702695707751, -0.12169542101888876,
                            -0.06096110015057879, 0.0,
                            0.06096110015057879,  0.12169542101888876,
                            0.18197702695707751,  0.24158166644779872,
                            0.300287606335332,    0.3578764566884095,
                            0.4141339832263038,   0.4688509042860411,
                            0.5218236693661858,   0.5728552163513039,
                            0.6217557046007232,   0.66834322117537,
                            0.7124444575770366,   0.7538953544853755,
                            0.7925417120993812,   0.828239763823065,
                            0.8608567111822925,   0.8902712180295274,
                            0.9163738623097802,   0.9390675440029624,
                            0.9582678486139082,   0.9739033680193239,
                            0.985915991735903,    0.9942612604367524,
                            0.9989099908489034};
  TestRoots(p51_roots);
}

TEST(LegendrePolynomialRootFinding, P64) {
  mfem::Vector p64_roots{
      -0.9993050417357721,  -0.9963401167719552,   -0.9910133714767442,
      -0.983336253884626,   -0.973326827789911,    -0.9610087996520538,
      -0.9464113748584028,  -0.9295691721319396,   -0.9105221370785028,
      -0.889315445995114,   -0.8659993981540928,   -0.8406292962525803,
      -0.8132653151227975,  -0.7839723589433414,   -0.7528199072605319,
      -0.7198818501716109,  -0.6852363130542332,   -0.6489654712546573,
      -0.6111553551723933,  -0.5718956462026339,   -0.5312794640198946,
      -0.489403145707053,   -0.44636601725346414,  -0.4022701579639916,
      -0.3572201583376682,  -0.3113228719902109,   -0.2646871622087674,
      -0.21742364374000703, -0.16964442042399283,  -0.12146281929612057,
      -0.07299312178779904, -0.024350292663424374, 0.024350292663424374,
      0.07299312178779904,  0.12146281929612057,   0.16964442042399283,
      0.21742364374000703,  0.2646871622087674,    0.3113228719902109,
      0.3572201583376682,   0.4022701579639916,    0.44636601725346414,
      0.489403145707053,    0.5312794640198946,    0.5718956462026339,
      0.6111553551723933,   0.6489654712546573,    0.6852363130542332,
      0.7198818501716109,   0.7528199072605319,    0.7839723589433414,
      0.8132653151227975,   0.8406292962525803,    0.8659993981540928,
      0.889315445995114,    0.9105221370785028,    0.9295691721319396,
      0.9464113748584028,   0.9610087996520538,    0.973326827789911,
      0.983336253884626,    0.9910133714767442,    0.9963401167719552,
      0.9993050417357721};
  TestRoots(p64_roots);
}

TEST(ApproximateLegendrePrimeRoot, IncreasingInK) {
  for (int n = 3; n <= 10; ++n) {
    for (int k = 0; k <= n - 3; ++k) {
      EXPECT_LT(ApproximateLegendrePrimeRoot(n, k),
                ApproximateLegendrePrimeRoot(n, k + 1))
          << "n=" << n << " k=" << k;
    }
  }
}

// =======================================================================
// LegendrePrimeRoot(n, k) / AllLegendrePrimeRoots(n)
// =======================================================================
// Reference values below are analytic roots of P'_n(x), solved by hand
// from the closed-form derivative polynomials, ordered descending in k
// (k=0 is the root nearest +1), consistent with the ordering implied by
// ApproximateLegendrePrimeRoot's docstring example.

void TestPrimeRoots(const mfem::Vector& expected_roots) {
  int order = expected_roots.Size() + 1;  // n-1 roots for order n
  auto computed_roots = AllLegendrePrimeRoots(order);
  ASSERT_EQ(computed_roots.Size(), expected_roots.Size());
  for (int i = 0; i < expected_roots.Size(); ++i) {
    EXPECT_DOUBLE_EQ(computed_roots[i], expected_roots[i])
        << "order=" << order << " index=" << i;
  }
}

TEST(LegendrePrimeRootFinding, P2) {
  mfem::Vector p2_prime_roots{0.0};
  TestPrimeRoots(p2_prime_roots);
}

TEST(LegendrePrimeRootFinding, P3) {
  mfem::Vector p3_prime_roots{-0.4472135954999581, 0.4472135954999581};
  TestPrimeRoots(p3_prime_roots);
}

TEST(LegendrePrimeRootFinding, P4) {
  mfem::Vector p4_prime_roots{-0.6546536707079771, 0.0, 0.6546536707079771};
  TestPrimeRoots(p4_prime_roots);
}

TEST(LegendrePrimeRootFinding, P5) {
  mfem::Vector p5_prime_roots{-0.76505532392946474, -0.2852315164806451,
                                 0.2852315164806451, 0.76505532392946474};
  TestPrimeRoots(p5_prime_roots);
}

TEST(LegendrePrimeRootFinding, P6) {
  mfem::Vector p6_prime_roots{-0.83022389627856696, -0.46884879347071423,
                                 0.0, 0.46884879347071423, 0.83022389627856696};
  TestPrimeRoots(p6_prime_roots);
}

// LegendrePrimeRoot should agree with the corresponding entry of
// AllLegendrePrimeRoots for the same (n, k).
TEST(LegendrePrimeRoot, ConsistentWithAllLegendrePrimeRoots) {
  for (int n = 2; n <= 8; ++n) {
    auto all_roots = AllLegendrePrimeRoots(n);
    for (int k = 0; k <= n - 2; ++k) {
      EXPECT_DOUBLE_EQ(LegendrePrimeRoot(n, k), all_roots[k])
          << "n=" << n << " k=" << k;
    }
  }
}

// P_n' evaluated at each computed root should be (numerically) zero.
TEST(LegendrePrimeRoot, IsActuallyARootOfDerivative) {
  for (int n = 2; n <= 10; ++n) {
    for (int k = 0; k <= n - 2; ++k) {
      double root = LegendrePrimeRoot(n, k);
      EXPECT_NEAR(LegendrePolynomialPrime(n, root), 0.0, 1e-13)
          << "n=" << n << " k=" << k;
    }
  }
}

TEST(AllLegendrePrimeRoots, ReturnsNMinusOneRoots) {
  for (int n = 2; n <= 12; ++n) {
    EXPECT_EQ(AllLegendrePrimeRoots(n).Size(), n - 1) << "n=" << n;
  }
}

// Roots of P_n' are symmetric about 0 (same parity argument as P_n
// itself, since differentiation flips/preserves parity but roots of an
// even function's derivative are still symmetric about the origin).
TEST(AllLegendrePrimeRoots, SymmetricAboutZero) {
  for (int n = 2; n <= 12; ++n) {
    auto roots = AllLegendrePrimeRoots(n);
    int count = roots.Size();
    for (int i = 0; i < count; ++i) {
      EXPECT_DOUBLE_EQ(roots[i], -roots[count - 1 - i])
          << "n=" << n << " i=" << i;
    }
  }
}

// =======================================================================
// LegendrePolynomialPrimePrime(n, x)
// =======================================================================

TEST(LegendrePolynomialPrimePrime, ClosedFormsForLowOrders) {
  for (double x : kInteriorX) {
    EXPECT_NEAR(LegendrePolynomialPrimePrime(2, x), 3.0, 1e-13) << "x=" << x;
    EXPECT_NEAR(LegendrePolynomialPrimePrime(3, x), 15.0 * x, 1e-13)
        << "x=" << x;
    EXPECT_NEAR(LegendrePolynomialPrimePrime(4, x),
                (105.0 * x * x - 15.0) / 2.0, 1e-13)
        << "x=" << x;
    EXPECT_NEAR(LegendrePolynomialPrimePrime(5, x),
                (315.0 * x * x * x - 105.0 * x) / 2.0, 1e-12)
        << "x=" << x;
  }
}

TEST(LegendrePolynomialPrimePrime, MatchesFiniteDifferenceOfFirstDerivative) {
  const double h = 1e-5;
  for (int n = 2; n <= 10; ++n) {
    for (double x : kInteriorX) {
      const double fd = (LegendrePolynomialPrime(n, x + h) -
                         LegendrePolynomialPrime(n, x - h)) /
                        (2.0 * h);
      const double scale = std::max(1.0, std::abs(fd));
      EXPECT_NEAR(LegendrePolynomialPrimePrime(n, x), fd, 1e-6 * scale)
          << "n=" << n << " x=" << x;
    }
  }
}

// P_n''(1) = (n-1) n (n+1) (n+2) / 8 and P_n''(-1) = (-1)^n P_n''(1). The
// defining formula divides by 1 - x^2, so the endpoints need special handling.
TEST(LegendrePolynomialPrimePrime, EndpointValues) {
  for (int n = 2; n <= 8; ++n) {
    const double expected = (n - 1.0) * n * (n + 1.0) * (n + 2.0) / 8.0;
    EXPECT_NEAR(LegendrePolynomialPrimePrime(n, 1.0), expected, 1e-12)
        << "n=" << n;
    EXPECT_NEAR(LegendrePolynomialPrimePrime(n, -1.0),
                (n % 2 == 0 ? 1.0 : -1.0) * expected, 1e-12)
        << "n=" << n;
  }
}

// =======================================================================
// ApproximateLegendreRoot(n, k) / LegendreRoot(n, k) / AllLegendreRoots(n)
// =======================================================================

TEST(ApproximateLegendreRoot, MatchesDocumentedFormula) {
  for (int n : {1, 4, 7}) {
    for (int k = 0; k < n; ++k) {
      EXPECT_DOUBLE_EQ(ApproximateLegendreRoot(n, k),
                       std::cos((4.0 * k + 3.0) / (4.0 * n + 2.0) * M_PI))
          << "n=" << n << " k=" << k;
    }
  }
}

TEST(ApproximateLegendreRoot, DecreasesInK) {
  for (int n = 2; n <= 10; ++n) {
    for (int k = 1; k < n; ++k) {
      EXPECT_LT(ApproximateLegendreRoot(n, k),
                ApproximateLegendreRoot(n, k - 1))
          << "n=" << n << " k=" << k;
    }
  }
}

// The approximation only seeds Newton's method; for n >= 4 it is within 0.02
// of the converged root (the largest error, n = 4, is about 0.005).
TEST(ApproximateLegendreRoot, IsCloseToTheTrueRoot) {
  for (int n = 4; n <= 10; ++n) {
    for (int k = 0; k < n; ++k) {
      EXPECT_NEAR(ApproximateLegendreRoot(n, k), AllLegendreRoots(n)[n - 1 - k],
                  0.02)
          << "n=" << n << " k=" << k;
    }
  }
}

// Valid root numbers are k = 0, ..., n - 1.
TEST(ApproximateLegendreRoot, ThrowsForRootNumberAtOrAboveOrder) {
  EXPECT_THROW(ApproximateLegendreRoot(4, 4), std::invalid_argument);
  EXPECT_THROW(ApproximateLegendreRoot(4, 5), std::invalid_argument);
  EXPECT_NO_THROW(ApproximateLegendreRoot(4, 3));
}

TEST(LegendreRoot, MatchesKnownRoots) {
  EXPECT_NEAR(LegendreRoot(2, 0), 1.0 / std::sqrt(3.0), 1e-15);
  EXPECT_NEAR(LegendreRoot(3, 0), std::sqrt(0.6), 1e-15);
  EXPECT_NEAR(LegendreRoot(4, 0), 0.8611363115940526, 1e-15);
  EXPECT_NEAR(LegendreRoot(4, 1), 0.3399810435848563, 1e-15);
}

// k = 0 is the root nearest +1, so root k of order n is entry n - 1 - k of the
// ascending vector returned by AllLegendreRoots.
TEST(LegendreRoot, ConsistentWithAllLegendreRootsAndIsARoot) {
  for (int n = 2; n <= 12; ++n) {
    const mfem::Vector all_roots = AllLegendreRoots(n);
    for (int k = 0; k < n / 2; ++k) {
      const double root = LegendreRoot(n, k);
      EXPECT_EQ(root, all_roots[n - 1 - k]) << "n=" << n << " k=" << k;
      EXPECT_NEAR(LegendrePolynomial(n, root), 0.0, 1e-13)
          << "n=" << n << " k=" << k;
    }
  }
}

TEST(AllLegendreRoots, OnePointIsTheOrigin) {
  const mfem::Vector roots = AllLegendreRoots(1);
  ASSERT_EQ(roots.Size(), 1);
  EXPECT_EQ(roots[0], 0.0);
}

TEST(AllLegendreRoots, HasNAscendingAntisymmetricRoots) {
  for (int n = 2; n <= 20; ++n) {
    const mfem::Vector roots = AllLegendreRoots(n);
    ASSERT_EQ(roots.Size(), n) << "n=" << n;
    for (int i = 0; i < n; ++i) {
      EXPECT_GT(roots[i], -1.0);
      EXPECT_LT(roots[i], 1.0);
      if (i > 0) EXPECT_LT(roots[i - 1], roots[i]) << "n=" << n << " i=" << i;
      EXPECT_DOUBLE_EQ(roots[i], -roots[n - 1 - i]) << "n=" << n << " i=" << i;
    }
  }
}

// =======================================================================
// ApproximateLegendrePrimeRoot(n, k)
// =======================================================================

TEST(ApproximateLegendrePrimeRoot,
     IsTheAverageOfSurroundingRootApproximations) {
  for (int n : {3, 5, 8}) {
    for (int k = 0; k < n - 1; ++k) {
      const double expected = -0.5 * (ApproximateLegendreRoot(n, k) +
                                      ApproximateLegendreRoot(n, k + 1));
      EXPECT_DOUBLE_EQ(ApproximateLegendrePrimeRoot(n, k), expected)
          << "n=" << n << " k=" << k;
    }
  }
}

// Only a Newton seed: the worst case in this range is n = 4 (about 0.05 off),
// and it improves with n. Note the implementation returns the *negated*
// average of the cosines, so k = 0 is the most negative root, matching the
// ascending order of AllLegendrePrimeRoots.
TEST(ApproximateLegendrePrimeRoot, IsCloseToTheConvergedRoot) {
  for (int n = 4; n <= 10; ++n) {
    for (int k = 0; k < n - 1; ++k) {
      EXPECT_NEAR(ApproximateLegendrePrimeRoot(n, k), LegendrePrimeRoot(n, k),
                  0.1)
          << "n=" << n << " k=" << k;
    }
  }
}

// P'_n has n - 1 roots, k = 0, ..., n - 2.
TEST(ApproximateLegendrePrimeRoot, ThrowsForRootNumberAtOrAboveNMinusOne) {
  EXPECT_THROW(ApproximateLegendrePrimeRoot(4, 3), std::invalid_argument);
  EXPECT_THROW(ApproximateLegendrePrimeRoot(4, 10), std::invalid_argument);
  EXPECT_NO_THROW(ApproximateLegendrePrimeRoot(4, 2));
}

}  // namespace kingfisher