# CMakeLists.txt

# 3.25 is the first release that supports workflow presets (CMakePresets.json v6).
cmake_minimum_required(VERSION 3.25)

project(
	kingfisher
	VERSION 0.1.0
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(
      FATAL_ERROR
        "In-source builds are not allowed. Please create a separate build directory."
    )
endif()

option(KINGFISHER_BUILD_TESTS "Build the Kingfisher unit tests" ${PROJECT_IS_TOP_LEVEL})
option(KINGFISHER_BUILD_DOCS "Build the documentation (needs Doxygen and Sphinx; see docs/requirements.txt)" OFF)

# NOTE(parallel): Placeholder for the MPI build. Turning this on does nothing
# yet. To wire it up, see the NOTE(parallel) comments in
# cmake/Dependencies.cmake, src/CMakeLists.txt, and tests/CMakeLists.txt, then
# add an "mpi" configure preset in CMakePresets.json that inherits from "base"
# and sets KINGFISHER_USE_MPI to ON.
option(KINGFISHER_USE_MPI "Build Kingfisher and MFEM with MPI (not implemented yet)" OFF)
if(KINGFISHER_USE_MPI)
    message(FATAL_ERROR "KINGFISHER_USE_MPI is not implemented yet. See NOTE(parallel) in cmake/Dependencies.cmake.")
endif()

include(cmake/CPM.cmake)

set(TEST_DIR "${PROJECT_SOURCE_DIR}/tests")

if(KINGFISHER_BUILD_DOCS)
    # The docs only read the sources, so the docs preset skips compiling the
    # libraries (and downloading MFEM) entirely.
    add_subdirectory(docs)
else()
    include(cmake/Dependencies.cmake)
    add_subdirectory(src)

    if(KINGFISHER_BUILD_TESTS)
        include(CTest)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
