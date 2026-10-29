// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "utils/input_parameters.h"

#include <gtest/gtest.h>

#include <fstream>
#include <string>

#include "utils/errors.h"

namespace kingfisher {
namespace {

constexpr char kValidYaml[] =
    "problem:\n"
    "  name: lathrop\n"
    "  mode: fixed_source\n"
    "mesh:\n"
    "  filename: lathrop.msh\n";

// Returns the message of the Error thrown by parsing `yaml`, or fails.
std::string ParseErrorMessage(const std::string& yaml) {
  try {
    ParseInputString(yaml);
  } catch (const Error& e) {
    return e.what();
  }
  ADD_FAILURE() << "expected an Error";
  return "";
}

TEST(InputParameters, ReadsProblemAndMesh) {
  const InputParameters input = ParseInputString(kValidYaml);
  EXPECT_EQ(input.problem_params.name, "lathrop");
  EXPECT_EQ(input.problem_params.run_mode, RunMode::kFixedSource);
  EXPECT_EQ(input.mesh_params.filename, "lathrop.msh");
}

TEST(InputParameters, UnknownEnumValueListsAvailableValues) {
  const std::string what = ParseErrorMessage(
      "problem:\n  name: lathrop\n  mode: bogus\nmesh:\n  filename: m.msh\n");
  EXPECT_NE(what.find("bogus"), std::string::npos);
  EXPECT_NE(what.find("fixed_source"), std::string::npos);
}

TEST(InputParameters, NonScalarEnumValueThrows) {
  const std::string what = ParseErrorMessage(
      "problem:\n  name: lathrop\n  mode: [a, b]\nmesh:\n  filename: m.msh\n");
  EXPECT_NE(what.find("mode"), std::string::npos);
  EXPECT_NE(what.find("single value"), std::string::npos);
}

TEST(InputParameters, MissingKeyThrows) {
  EXPECT_THROW(ParseInputString("problem:\n  name: x\n"), Error);
}

TEST(InputParameters, MalformedYamlThrowsInsteadOfAborting) {
  EXPECT_THROW(ParseInputString("problem: [unclosed\n"), Error);
}

TEST(InputParameters, ParseErrorLineNumberIsOneBased) {
  // ryml numbers lines from 0 and reports where the parser stopped, which is
  // the end of the buffer (line 5 counting from 1) for an unclosed bracket.
  const std::string what = ParseErrorMessage("a: 1\nb: 2\nc: [x\nd: 4\n");
  EXPECT_NE(what.find("at line 5:"), std::string::npos) << what;
}

TEST(InputParameters, ReadsFileAndPrefixesErrorsWithFilename) {
  const std::string good = testing::TempDir() + "good.yaml";
  std::ofstream(good) << kValidYaml;
  EXPECT_EQ(ReadInputFile(good).problem_params.name, "lathrop");

  const std::string bad = testing::TempDir() + "bad.yaml";
  std::ofstream(bad) << "problem:\n  name: x\n";
  try {
    ReadInputFile(bad);
    FAIL() << "expected an Error";
  } catch (const Error& e) {
    EXPECT_NE(std::string(e.what()).find("bad.yaml"), std::string::npos);
  }
}

TEST(InputParameters, MissingFileThrows) {
  EXPECT_THROW(ReadInputFile(testing::TempDir() + "does_not_exist.yaml"),
               Error);
}

}  // namespace
}  // namespace kingfisher
