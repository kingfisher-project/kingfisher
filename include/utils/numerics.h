// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, Liam Pohlmann
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_UTILS_NUMERICS_H_
#define KINGFISHER_UTILS_NUMERICS_H_

#include <cmath>
#include <limits>

namespace kingfisher {
/// @brief General tolerance value for double comparisons
inline constexpr double TOLERANCE = 1e-15;

/// @brief Tolerance value for EXPECT_NEAR in tests
inline constexpr double EXP_NEAR_TOLERANCE = 1e-14;

/**
 * @brief Tests if two double-type numbers are equal using the TOLERANCE
 * value. Taken from https://github.com/lipohlmann/hummingbird.
 *
 * @param first First number to compare
 * @param second Second number to compare
 * @param tolerance Tolerance for the absolute difference between the first and
 * second values
 * @return true
 * @return false
 */
inline bool DoubleEqual(const double first, const double second,
                        const double tolerance = TOLERANCE) {
  double difference = second - first;
  return (std::abs(difference) < tolerance);
}

/**
 * @brief Compute the relative error between two values.
 *
 * @param new_val New value
 * @param old_val Old value
 * @return double
 */
inline double RelativeError(const double new_val, const double old_val) {
  if (new_val == 0.0)
    return old_val == 0.0 ? 0.0 : std::numeric_limits<double>::infinity();
  return std::abs((new_val - old_val) / new_val);
}
}  // namespace kingfisher

#endif  // KINGFISHER_UTILS_NUMERICS_H_
