// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#include "utils/input_parameters.h"

#include <gtest/gtest.h>

#include <fstream>
#include <string>
#include <vector>

#include "utils/errors.h"

namespace kingfisher {

// A complete two-group input.
constexpr char kValidYaml[] = R"(version: 0
problem:
  name: lathrop
  mode: fixed_source
mesh:
  filename: lathrop.msh
boundary_conditions:
  - name: outer_wall
    type: vacuum
  - name: reflective_wall
    type: reflective
materials:
  - name: fuel
    total_xs: [0.75, 1.5]
    scatter_xs: [[[0.1, 0.2], [0.3, 0.4]]]
  - name: moderator
    total_xs: [2.0, 3.0]
    scatter_xs: [[[0.5, 0.6], [0.7, 0.8]]]
sources:
  - name: square_source
    type: volumetric
    expression: ["1.0/4.0/pi", "0"]
spatial:
  node_placement: gauss_lobatto_legendre
  nodes_per_axis: 3
solver:
  source_iterations:
    tolerance: 1.0e-6
    max_iterations: 50
output:
  format: vtkhdf
  export_values: [scalar_flux, angular_flux]
)";

// Returns kValidYaml with the first occurrence of `from` replaced by `to`.
std::string Modified(const std::string& from, const std::string& to) {
  std::string yaml = kValidYaml;
  const std::size_t pos = yaml.find(from);
  EXPECT_NE(pos, std::string::npos) << "test bug: '" << from << "' not found";
  if (pos != std::string::npos) yaml.replace(pos, from.size(), to);
  return yaml;
}

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

TEST(InputParameters, ReadsNumbers) {
  const InputParameters input = ParseInputString(kValidYaml);
  EXPECT_EQ(input.spatial_params.nodes_per_axis, 3u);
  EXPECT_EQ(input.spatial_params.se_node_placement,
            SENodePlacement::kGaussLobattoLegendre);
  EXPECT_DOUBLE_EQ(input.solver_params.source_iter_params.tolerance, 1.0e-6);
  EXPECT_EQ(input.solver_params.source_iter_params.max_iterations, 50u);
}

TEST(InputParameters, ReadsExportValuesInOrder) {
  const InputParameters input = ParseInputString(kValidYaml);
  EXPECT_EQ(input.output_params.output_format, OutputFormat::kVTKHDF);
  ASSERT_EQ(input.output_params.export_values.size(), 2u);
  EXPECT_EQ(input.output_params.export_values[0], ExportValues::kScalarFlux);
  EXPECT_EQ(input.output_params.export_values[1], ExportValues::kAngularFlux);
}

TEST(InputParameters, BadExportValueNamesTheList) {
  const std::string what = ParseErrorMessage(
      Modified("[scalar_flux, angular_flux]", "[scalar_flux, bogus]"));
  EXPECT_NE(what.find("export_values"), std::string::npos) << what;
  EXPECT_NE(what.find("bogus"), std::string::npos) << what;
  EXPECT_NE(what.find("angular_flux"), std::string::npos) << what;
}

TEST(InputParameters, ExportValuesMustBeAList) {
  const std::string what =
      ParseErrorMessage(Modified("[scalar_flux, angular_flux]", "scalar_flux"));
  EXPECT_NE(what.find("export_values"), std::string::npos) << what;
  EXPECT_NE(what.find("list"), std::string::npos) << what;
}

TEST(InputParameters, ReadsBoundariesInOrder) {
  const InputParameters input = ParseInputString(kValidYaml);
  ASSERT_EQ(input.boundaries.size(), 2u);
  EXPECT_EQ(input.boundaries[0].name, "outer_wall");
  EXPECT_EQ(input.boundaries[0].type, BoundaryType::kVacuum);
  EXPECT_EQ(input.boundaries[1].name, "reflective_wall");
  EXPECT_EQ(input.boundaries[1].type, BoundaryType::kReflective);
}

TEST(InputParameters, ReadsMaterialsAndFlattensScatter) {
  const InputParameters input = ParseInputString(kValidYaml);
  ASSERT_EQ(input.materials.size(), 2u);
  EXPECT_EQ(input.materials[0].name, "fuel");
  EXPECT_EQ(input.materials[0].total_xs, (std::vector<double>{0.75, 1.5}));
  // [moment][from][to] flattened row-major.
  EXPECT_EQ(input.materials[0].scatter_matrix,
            (std::vector<double>{0.1, 0.2, 0.3, 0.4}));
  EXPECT_EQ(input.materials[1].name, "moderator");
  EXPECT_EQ(input.materials[1].scatter_matrix,
            (std::vector<double>{0.5, 0.6, 0.7, 0.8}));
}

TEST(InputParameters, RaggedScatterThrows) {
  const std::string what = ParseErrorMessage(
      Modified("[[[0.1, 0.2], [0.3, 0.4]]]", "[[[0.1, 0.2], [0.3]]]"));
  EXPECT_NE(what.find("fuel"), std::string::npos) << what;
  EXPECT_NE(what.find("scatter_xs"), std::string::npos) << what;
}

TEST(InputParameters, ScatterGroupCountMustMatchTotalXs) {
  EXPECT_THROW(
      ParseInputString(Modified("[[[0.1, 0.2], [0.3, 0.4]]]", "[[[0.1]]]")),
      Error);
}

TEST(InputParameters, ReadsSources) {
  const InputParameters input = ParseInputString(kValidYaml);
  ASSERT_EQ(input.sources.size(), 1u);
  EXPECT_EQ(input.sources[0].name, "square_source");
  EXPECT_EQ(input.sources[0].type, SourceType::kVolumetric);
  EXPECT_EQ(input.sources[0].expression,
            (std::vector<std::string>{"1.0/4.0/pi", "0"}));
}

TEST(InputParameters, SourcesBlockIsOptional) {
  const std::string yaml = Modified(
      "sources:\n  - name: square_source\n    type: volumetric\n"
      "    expression: [\"1.0/4.0/pi\", \"0\"]\n",
      "");
  EXPECT_TRUE(ParseInputString(yaml).sources.empty());
}

TEST(InputParameters, UnknownEnumValueListsAvailableValues) {
  const std::string what =
      ParseErrorMessage(Modified("mode: fixed_source", "mode: bogus"));
  EXPECT_NE(what.find("bogus"), std::string::npos);
  EXPECT_NE(what.find("fixed_source"), std::string::npos);
}

TEST(InputParameters, NonScalarEnumValueThrows) {
  const std::string what =
      ParseErrorMessage(Modified("mode: fixed_source", "mode: [a, b]"));
  EXPECT_NE(what.find("mode"), std::string::npos);
  EXPECT_NE(what.find("single value"), std::string::npos);
}

TEST(InputParameters, NonNumericValueThrows) {
  EXPECT_THROW(
      ParseInputString(Modified("tolerance: 1.0e-6", "tolerance: abc")), Error);
}

TEST(InputParameters, NegativeUnsignedValueThrows) {
  EXPECT_THROW(
      ParseInputString(Modified("max_iterations: 50", "max_iterations: -1")),
      Error);
}

TEST(InputParameters, WrongVersionThrows) {
  const std::string what =
      ParseErrorMessage(Modified("version: 0", "version: 1"));
  EXPECT_NE(what.find("version 0"), std::string::npos) << what;
}

TEST(InputParameters, MissingKeyThrows) {
  EXPECT_THROW(ParseInputString("version: 0\nproblem:\n  name: x\n"), Error);
}

TEST(InputParameters, MissingKeyNamesTheFullPath) {
  const std::string what =
      ParseErrorMessage(Modified("  mode: fixed_source\n", ""));
  EXPECT_NE(what.find("'problem.mode'"), std::string::npos) << what;
}

TEST(InputParameters, MissingKeyInListEntryNamesTheList) {
  const std::string what =
      ParseErrorMessage(Modified("    total_xs: [0.75, 1.5]\n", ""));
  EXPECT_NE(what.find("'total_xs'"), std::string::npos) << what;
  EXPECT_NE(what.find("'materials'"), std::string::npos) << what;
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
  std::ofstream(bad) << "version: 0\nproblem:\n  name: x\n";
  try {
    ReadInputFile(bad);
    FAIL() << "expected an Error";
  } catch (const Error& e) {
    EXPECT_NE(std::string(e.what()).find("bad.yaml"), std::string::npos);
  }
}

// Keeps the shipped example in sync with the parser.
TEST(InputParameters, ReadsLathropExample) {
  const InputParameters input = ReadInputFile(
      std::string(TEST_DIR) + "/../examples/lathrop/lathrop.yaml");
  EXPECT_EQ(input.problem_params.name, "lathrop");
  EXPECT_EQ(input.boundaries.size(), 2u);
  ASSERT_EQ(input.materials.size(), 1u);
  EXPECT_EQ(input.materials[0].total_xs, (std::vector<double>{0.75}));
  EXPECT_EQ(input.materials[0].scatter_matrix, (std::vector<double>{0.5}));
  ASSERT_EQ(input.sources.size(), 1u);
  EXPECT_EQ(input.sources[0].expression,
            (std::vector<std::string>{"1.0/4.0/pi"}));
  EXPECT_EQ(input.spatial_params.nodes_per_axis, 3u);
  EXPECT_EQ(input.output_params.export_values.size(), 2u);
}

TEST(InputParameters, AcceptsDocumentMarkers) {
  EXPECT_EQ(ParseInputString(std::string("---\n") + kValidYaml + "...\n")
                .problem_params.name,
            "lathrop");
}

TEST(InputParameters, RejectsMultipleDocuments) {
  EXPECT_THROW(ParseInputString(std::string("---\n") + kValidYaml + "---\n" +
                                kValidYaml),
               Error);
}

TEST(InputParameters, MissingFileThrows) {
  EXPECT_THROW(ReadInputFile(testing::TempDir() + "does_not_exist.yaml"),
               Error);
}

}  // namespace kingfisher
