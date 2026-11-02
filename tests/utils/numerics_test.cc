// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, Liam Pohlmann
// Copyright (c) 2026, kingfisher-project

#include "utils/numerics.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>

#include "../tests/constants.h"

namespace kingfisher {
TEST(UtilsNumerics, DoubleEqual) {
  EXPECT_TRUE(DoubleEqual(1.0, 1.0));
  EXPECT_TRUE(DoubleEqual(1.0, 1.0 + kTolerance / 2.0));
  EXPECT_FALSE(DoubleEqual(1.0, -1.0));
  EXPECT_FALSE(DoubleEqual(1.0, 2.5));
  EXPECT_FALSE(DoubleEqual(1.0, 1.0 + kTolerance));
}

TEST(UtilsNumerics, DoubleEqualHonorsCustomTolerance) {
  EXPECT_TRUE(DoubleEqual(1.0, 1.05, 0.1));
  EXPECT_FALSE(DoubleEqual(1.0, 1.2, 0.1));
  EXPECT_FALSE(DoubleEqual(1.0, 1.05, 0.01));
}

TEST(UtilsNumerics, DoubleEqualIsSymmetricAndRejectsNaN) {
  EXPECT_EQ(DoubleEqual(1.0, 1.05, 0.1), DoubleEqual(1.05, 1.0, 0.1));
  EXPECT_EQ(DoubleEqual(1.0, 1.2, 0.1), DoubleEqual(1.2, 1.0, 0.1));
  const double nan = std::numeric_limits<double>::quiet_NaN();
  EXPECT_FALSE(DoubleEqual(nan, nan));
  EXPECT_FALSE(DoubleEqual(1.0, nan));
}

TEST(UtilsNumerics, RelativeError) {
  EXPECT_DOUBLE_EQ(RelativeError(1.0, 1.0), 0.0);
  EXPECT_DOUBLE_EQ(RelativeError(0, 0), 0.0);
  EXPECT_EQ(RelativeError(0, 1.0), std::numeric_limits<double>::infinity());
}

// The error is relative to the *first* argument, so it is not symmetric.
TEST(UtilsNumerics, RelativeErrorDividesByNewValue) {
  EXPECT_DOUBLE_EQ(RelativeError(2.0, 1.0), 0.5);
  EXPECT_DOUBLE_EQ(RelativeError(1.0, 2.0), 1.0);
}

TEST(UtilsNumerics, RelativeErrorIsNonNegativeForNegativeValues) {
  EXPECT_DOUBLE_EQ(RelativeError(-2.0, -1.0), 0.5);
  EXPECT_DOUBLE_EQ(RelativeError(-2.0, 1.0), 1.5);
}

TEST(UtilsNumerics, ToleranceConstants) {
  EXPECT_EQ(kTolerance, 1e-15);
  EXPECT_EQ(kExpNearTolerance, 1e-14);
  EXPECT_LT(kTolerance, kExpNearTolerance);
}
}  // namespace kingfisher
