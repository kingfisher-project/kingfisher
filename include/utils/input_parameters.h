// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef KINGFISHER_UTILS_INPUT_PARAMETERS_H_
#define KINGFISHER_UTILS_INPUT_PARAMETERS_H_

#include <string>

namespace kingfisher {
struct InputParameters {};

void ReadInput(const std::string filename);
void CheckInputFileVersion();

}  // namespace kingfisher

#endif  // KINGFISHER_UTILS_INPUT_PARAMETERS_H_
