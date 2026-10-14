# tests/CMakeLists.txt
#
# GoogleTest is added in cmake/Dependencies.cmake.

include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.cc")
add_executable(run_kingfisher_tests ${TEST_SOURCES})
target_compile_definitions(run_kingfisher_tests PRIVATE TEST_DIR="${TEST_DIR}")
target_link_libraries(run_kingfisher_tests PRIVATE GTest::gtest_main kingfisher_lib)

# NOTE(parallel): gtest_discover_tests runs each test as a single process.
# Tests that need several MPI ranks have to be registered by hand instead:
#   add_test(NAME <name>
#            COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 4
#                    $<TARGET_FILE:run_kingfisher_tests> --gtest_filter=<Suite>.*)
gtest_discover_tests(run_kingfisher_tests)
