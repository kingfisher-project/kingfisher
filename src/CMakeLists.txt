# src/CMakeLists.txt

CPMAddPackage("gh:fmtlib/fmt#12.2.0")
CPMAddPackage("gh:biojppm/rapidyaml@0.16.0")

add_library(
	kingfisher_lib
	main.cc
)

target_compile_features(kingfisher_lib PRIVATE cxx_std_20)

target_link_libraries(kingfisher_lib PRIVATE fmt::fmt)

target_include_directories(kingfisher_lib PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_sources(kingfisher_lib PRIVATE ${SOURCES})

add_executable(kingfisher main.cc)
target_compile_features(kingfisher PRIVATE cxx_std_20)
target_include_directories(kingfisher PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(kingfisher PRIVATE kingfisher_lib fmt::fmt ryml::ryml)

