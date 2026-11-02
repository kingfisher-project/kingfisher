// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, Liam Pohlmann
// Copyright (c) 2026, kingfisher-project

#include "utils/numerics.h"

#include <gtest/gtest.h>

namespace kingfisher {
TEST(UtilsNumerics, DoubleEqual) {
  EXPECT_TRUE(DoubleEqual(1.0, 1.0));
  EXPECT_TRUE(DoubleEqual(1.0, 1.0 + TOLERANCE / 2.0));
  EXPECT_FALSE(DoubleEqual(1.0, -1.0));
  EXPECT_FALSE(DoubleEqual(1.0, 2.5));
  EXPECT_FALSE(DoubleEqual(1.0, 1.0 + TOLERANCE));
}

TEST(UtilsNumerics, RelativeError) {
  EXPECT_DOUBLE_EQ(RelativeError(1.0, 1.0), 0.0);
  EXPECT_DOUBLE_EQ(RelativeError(0, 0), 0.0);
  EXPECT_EQ(RelativeError(0, 1.0), std::numeric_limits<double>::infinity());
}
}  // namespace kingfisher
