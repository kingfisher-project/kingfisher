# src/CMakeLists.txt
#
# Third-party packages are added in cmake/Dependencies.cmake.

# TODO: kingfisher_lib compiles main.cc only so that it has a source file. That
# puts a second main() into the test binary next to gtest_main's. Replace it
# with the library sources once there are some.
add_library(
	kingfisher_lib
	utils/errors.cc
)

target_compile_features(kingfisher_lib PUBLIC cxx_std_20)
target_include_directories(kingfisher_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_sources(kingfisher_lib PRIVATE ${SOURCES})

# The dependencies are PUBLIC so that the executable and the tests get them
# through kingfisher_lib.
# NOTE(parallel): MPI::MPI_CXX goes here once MFEM is built with MPI.
target_link_libraries(kingfisher_lib PUBLIC fmt::fmt ryml::ryml mfem)

add_executable(kingfisher main.cc)
target_link_libraries(kingfisher PRIVATE kingfisher_lib)
