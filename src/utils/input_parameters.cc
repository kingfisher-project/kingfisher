#include "utils/input_parameters.h"

#include <fstream>
#include <ryml.hpp>
#include <sstream>

#include "utils/errors.h"

namespace kingfisher {
void ReadInputFile(const std::string filename) {
  std::ifstream input_file(filename, std::ios::binary);
  KingfisherError(!input_file, "Failed to open yaml input file.");

  std::stringstream buffer;
  buffer << input_file.rdbuf();
  std::string contents = buffer.str();

  ryml::Tree tree = ryml::parse_in_place(contents);
}
void CheckInputFileVersion() {}
}  // namespace kingfisher
