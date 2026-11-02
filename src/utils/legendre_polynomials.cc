// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "utils/legendre_polynomials.h"

#include <cmath>

#include "utils/errors.h"
#include "utils/numerics.h"

namespace kingfisher {

double LegendrePolynomial(const int n, const double x) {
  switch (n) {
    case 0:
      return 1.0;

    case 1:
      return x;

    default:
      double previous = 1.0;
      double current = x;
      for (double j = 1; j < n; j++) {
        double next =
            ((2.0 * j + 1.0) * x * current - j * previous) / (j + 1.0);
        previous = current;
        current = next;
      }
      return current;
  }
}

double LegendrePolynomialPrime(const int n, const double x) {
  if (DoubleEqual(x, -1.0))
    return std::pow(-1, n - 1) * n * (n + 1) / 2.0;
  else if (DoubleEqual(x, 1.0))
    return n * (n + 1) / 2.0;

  switch (n) {
    case 0:
      return 0;

    case 1:
      return 1.0;

    default:
      return (static_cast<double>(n) * LegendrePolynomial(n - 1, x) -
              static_cast<double>(n) * x * LegendrePolynomial(n, x)) /
             (1.0 - x * x);
  }
}

double LegendrePolynomialPrimePrime(const int n, const double x) {
  if (DoubleEqual(x, 1.0))
    return n * (n - 1) * (n + 1) * (n + 2) / 8.0;
  else if (DoubleEqual(x, -1.0))
    return std::pow(-1.0, n) * n * (n - 1) * (n + 1) * (n + 2) / 8.0;

  return (2 * x * LegendrePolynomialPrime(n, x) -
          n * (n + 1) * LegendrePolynomial(n, x)) /
         (1.0 - x * x);
}

mfem::Vector AllLegendreRoots(const int n) {
  const int n_even = n / 2;
  mfem::Vector computed_roots(n);
  computed_roots = 0.0;  // mfem::Vector is uninitialized; odd n has a 0 root

  for (auto i = 0; i < n_even; i++) {
    computed_roots[n - 1 - i] = LegendreRoot(n, i);
    computed_roots[i] = -computed_roots[n - 1 - i];
  }
  return computed_roots;
}

double LegendreRoot(const int n, const int k) {
  double x_old = ApproximateLegendreRoot(n, k);
  unsigned int safety = 10000;
  for (auto i = 0; i < safety; i++) {
    double x_new = x_old - LegendrePolynomial(n, x_old) /
                               LegendrePolynomialPrime(n, x_old);
    double relative_error = std::abs((x_new - x_old) / std::max(1.0, x_new));
    double backward_error = std::abs(LegendrePolynomial(n, x_new));

    if (backward_error < 1e-8 && relative_error < TOLERANCE) return x_new;
    x_old = x_new;
  }
  throw std::runtime_error("LegendreRoot did not converge.");
}

double ApproximateLegendreRoot(const int n, const int k) {
  KINGFISHER_ASSERT(k < n, "k must be less than n.");

  return std::cos((4.0 * static_cast<double>(k) + 3.0) /
                  (4.0 * static_cast<double>(n) + 2.0) * M_PI);
}

mfem::Vector AllLegendrePrimeRoots(const int n) {
  const int n_even = (n - 1) / 2;
  mfem::Vector computed_roots(n - 1);
  computed_roots = 0.0;  // mfem::Vector is uninitialized; even n has a 0 root

  for (auto i = 0; i < n_even; i++) {
    computed_roots[i] = LegendrePrimeRoot(n, i);
    computed_roots[n - 2 - i] = -computed_roots[i];
  }
  return computed_roots;
}

double LegendrePrimeRoot(const int n, const int k) {
  double x_old = ApproximateLegendrePrimeRoot(n, k);
  unsigned int safety = 100;

  double relative_error = std::numeric_limits<double>::infinity();
  double backward_error = std::numeric_limits<double>::infinity();
  for (auto i = 0; i < safety; i++) {
    double x_new = x_old - LegendrePolynomialPrime(n, x_old) /
                               LegendrePolynomialPrimePrime(n, x_old);
    relative_error = std::abs((x_new - x_old) / std::max(1.0, x_new));
    backward_error = std::abs(LegendrePolynomialPrime(n, x_new));

    if (backward_error < 1e-8 && relative_error < TOLERANCE) return x_new;
    x_old = x_new;
  }
  std::ostringstream error_message;
  error_message << std::scientific << std::setprecision(6)
                << "LegendrePrimeRoot did not converge. "
                << "Backward Error: " << backward_error
                << ", Relative Error: " << relative_error;

  throw std::runtime_error(error_message.str());
}

double ApproximateLegendrePrimeRoot(const int n, const int k) {
  KINGFISHER_ASSERT(k < n - 1, "k must be less than n-1.");
  return -(ApproximateLegendreRoot(n, k) + ApproximateLegendreRoot(n, k + 1)) /
         2.0;
}
}  // namespace kingfisher
