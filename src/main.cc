// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include <fmt/core.h>
#include <fmt/ostream.h>

#include "utils/input_parameters.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s <input_file_name>\n", argv[0]);
    return 1;
  }

  std::string input_filename = argv[1];
  fmt::print("Loading {} input file... ", input_filename);
  kingfisher::InputParameters input_params =
      kingfisher::ReadInputFile(input_filename);
  fmt::print("Done\n");

  return 0;
}
