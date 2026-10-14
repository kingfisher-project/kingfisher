# CMakeLists.txt

cmake_minimum_required(VERSION 3.24)

project(
	kingfisher
	VERSION 0.1.0
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$(CMAKE_BINARY_DIR}")
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(
      FATAL_ERROR
        "In-source builds are not allowed. Please create a separate build directory."
    )
endif()

include(CTest)
enable_testing()

get_filename_component(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
cmake_path(APPEND ROOT_DIR "source" OUTPUT_VARIABLE SOURCE_DIR)
cmake_path(APPEND ROOT_DIR "test" OUTPUT_VARIABLE TEST_DIR)

add_subdirectory(src)
add_subdirectory(tests)

