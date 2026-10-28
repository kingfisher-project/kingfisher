// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_UTILS_ERRORS_H_
#define KINGFISHER_UTILS_ERRORS_H_

#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>

// NOTE(parallel): an Error thrown on one MPI rank can leave the other ranks
// waiting in a collective call. Revisit this when MPI is added.

namespace kingfisher {

/**
 * @brief The exceptions thrown in Kingfisher. Includes commonly-used
 * KingfisherError() and KINGFISHER_ASSERT()
 *
 */
class Error : public std::runtime_error {
 public:
  /** @param message The full message, including the source location. */
  explicit Error(const std::string& message) : std::runtime_error(message) {}
};

/**
 * Throws an Error if `condition` is true.
 *
 * Use this to check user input and other conditions that can fail in a
 * correct program. It is always active. Format the message with
 * `fmt::format` if it needs values. Do not call it in hot loops.
 *
 * @param condition The error condition: the Error is thrown when it is true.
 * @param message Describes what went wrong and, if possible, how to fix it.
 * @param location The call site, filled in by the compiler.
 * @throws Error If `condition` is true. `what()` is
 *   `<file>:<line>: <message>`.
 */
void KingfisherError(
    bool condition, std::string_view message,
    std::source_location location = std::source_location::current());

namespace detail {

/** Throws the Error for a failed KINGFISHER_ASSERT(). */
[[noreturn]] void AssertFailed(const char* condition, std::string_view message,
                               const char* file, int line);

}  // namespace detail
}  // namespace kingfisher

/**
 * @def KINGFISHER_ASSERT(condition, message)
 * Throws an Error if `condition` is false.
 *
 * Use this for internal invariants (programmer mistakes), not for user input.
 * It is active only when NDEBUG is not defined. When NDEBUG is defined,
 * `condition` and `message` are not evaluated.
 */
#ifndef NDEBUG
#define KINGFISHER_ASSERT(condition, message)                             \
  do {                                                                    \
    if (!(condition)) {                                                   \
      ::kingfisher::detail::AssertFailed(#condition, (message), __FILE__, \
                                         __LINE__);                       \
    }                                                                     \
  } while (false)
#else
#define KINGFISHER_ASSERT(condition, message) \
  do {                                        \
    (void)sizeof(condition);                  \
    (void)sizeof(message);                    \
  } while (false)
#endif

#endif  // KINGFISHER_UTILS_ERRORS_H_
