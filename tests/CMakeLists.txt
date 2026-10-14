# tests/CMakeLists.txt

include(FetchContent)

find_package(GTest QUIET)
if(NOT GTest_FOUND)
        FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/5a9c3f9e8d9b90bbbe8feb32902146cb8f7c1757.zip
        )
        FetchContent_MakeAvailable(googletest)
endif()
include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.cc")
add_executable(run_kingfisher_tests ${TEST_SOURCES})
target_include_directories(run_kingfisher_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(run_kingfisher_tests PRIVATE TEST_DIR="${TEST_DIR}")
target_link_libraries(run_kingfisher_tests PRIVATE GTest::gtest_main kingfisher_lib)

gtest_discover_tests(run_kingfisher_tests)

