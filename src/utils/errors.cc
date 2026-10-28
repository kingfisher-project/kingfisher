// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "utils/errors.h"

#include <string>
#include <string_view>

namespace kingfisher {

void KingfisherError(bool condition, std::string_view message,
                     std::source_location location) {
  if (condition) {
    throw Error(std::string(location.file_name()) + ":" +
                std::to_string(location.line()) + ": " + std::string(message));
  }
}

namespace detail {

void AssertFailed(const char* condition, std::string_view message,
                  const char* file, int line) {
  throw Error(std::string(file) + ":" + std::to_string(line) +
              ": Assertion failed: " + condition + ": " + std::string(message));
}

}  // namespace detail
}  // namespace kingfisher
