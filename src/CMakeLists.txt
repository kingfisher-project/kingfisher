# src/CMakeLists.txt
#
# Third-party packages are added in cmake/Dependencies.cmake.

add_library(
	kingfisher_lib
	utils/errors.cc
	utils/input_parameters.cc
)

target_compile_features(kingfisher_lib PUBLIC cxx_std_20)
target_include_directories(kingfisher_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# The dependencies are PUBLIC so that the executable and the tests get them
# through kingfisher_lib.
# NOTE(parallel): MPI::MPI_CXX goes here once MFEM is built with MPI.
target_link_libraries(kingfisher_lib PUBLIC fmt::fmt ryml::ryml mfem)

add_executable(kingfisher main.cc)
target_link_libraries(kingfisher PRIVATE kingfisher_lib)
