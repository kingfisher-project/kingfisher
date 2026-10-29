// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "utils/input_parameters.h"

#include <array>
#include <cstddef>
#include <fstream>
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "utils/errors.h"

namespace kingfisher {
namespace {

// ryml aborts by default. These handlers turn its errors into
// kingfisher::Error. They are installed on the Tree rather than globally, so
// nothing outside this file is affected.
std::string ToString(ryml::csubstr s) { return std::string(s.str, s.len); }

[[noreturn]] void ThrowBasic(ryml::csubstr msg, const ryml::ErrorDataBasic&,
                             void*) {
  throw Error("YAML error: " + ToString(msg));
}

[[noreturn]] void ThrowParse(ryml::csubstr msg,
                             const ryml::ErrorDataParse& data, void*) {
  // ryml counts lines from 0 and reports npos when the line is unknown.
  const std::string line =
      data.ymlloc.line == ryml::npos
          ? ""
          : " at line " + std::to_string(data.ymlloc.line + 1);
  throw Error("YAML parse error" + line + ": " + ToString(msg));
}

[[noreturn]] void ThrowVisit(ryml::csubstr msg, const ryml::ErrorDataVisit&,
                             void*) {
  throw Error("YAML input error (missing or malformed key): " + ToString(msg));
}

ryml::Callbacks ThrowingCallbacks() {
  ryml::Callbacks callbacks = ryml::get_callbacks();
  callbacks.set_error_basic(ThrowBasic);
  callbacks.set_error_parse(ThrowParse);
  callbacks.set_error_visit(ThrowVisit);
  return callbacks;
}

// YAML spellings of each enum. A table is added together with the block that
// reads it.
template <typename E>
using Entry = std::pair<std::string_view, E>;

constexpr std::array kRunModeNames = {
    Entry<RunMode>{"fixed_source", RunMode::kFixedSource}};

// Returns the enumerator whose YAML spelling is the scalar in `node`. Throws an
// Error that lists the available spellings if there is no match.
template <typename E, std::size_t N>
E ParseEnum(const ryml::ConstNodeRef& node,
            const std::array<Entry<E>, N>& table) {
  const std::string key = node.has_key() ? ToString(node.key()) : "value";
  if (!node.has_val()) throw Error("'" + key + "' must be a single value.");

  const std::string_view text(node.val().str, node.val().len);
  for (const auto& [name, enumerator] : table) {
    if (name == text) return enumerator;
  }

  std::string available;
  for (const auto& entry : table) {
    available += (available.empty() ? "" : ", ") + std::string(entry.first);
  }
  throw Error("Unknown " + key + " '" + std::string(text) +
              "'. Available: " + available + ".");
}

void ReadProblem(const ryml::ConstNodeRef& root, ProblemParams* problem) {
  root["problem"]["name"].load(&problem->name);
  problem->run_mode = ParseEnum(root["problem"]["mode"], kRunModeNames);
}

void ReadMesh(const ryml::ConstNodeRef& root, MeshParams* mesh) {
  root["mesh"]["filename"].load(&mesh->filename);
}

}  // namespace

InputParameters ParseInputString(std::string yaml) {
  // The tree holds views into `yaml`, so `yaml` must outlive it.
  ryml::Tree tree(ThrowingCallbacks());
  ryml::parse_in_place(ryml::to_substr(yaml), &tree);
  const ryml::ConstNodeRef root = tree.crootref();

  InputParameters input_params;
  ReadProblem(root, &input_params.problem_params);
  ReadMesh(root, &input_params.mesh_params);
  return input_params;
}

InputParameters ReadInputFile(const std::string& filename) {
  std::ifstream input_file(filename, std::ios::binary);
  KingfisherError(!input_file,
                  "Failed to open yaml input file '" + filename + "'.");

  std::stringstream buffer;
  buffer << input_file.rdbuf();

  try {
    return ParseInputString(buffer.str());
  } catch (const Error& e) {
    throw Error(filename + ": " + e.what());
  }
}

void CheckInputFileVersion() {}
}  // namespace kingfisher
