# cmake/Dependencies.cmake
#
# Every third-party library Kingfisher builds against comes from CPM here. Set
# CPM_SOURCE_CACHE (for example to ~/.cache/CPM) so that all build directories
# share one copy of each download instead of fetching MFEM again for each preset.

CPMAddPackage("gh:fmtlib/fmt#12.2.0")
CPMAddPackage("gh:biojppm/rapidyaml@0.16.0")

if(KINGFISHER_BUILD_TESTS)
    CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.17.0
        OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt ON"
    )
endif()

# MFEM's library target is plain `mfem`. It has no `mfem::` namespace, either
# here or from an installed MFEMConfig.cmake.
#
# NOTE(parallel): To build MFEM (and so Kingfisher) with MPI:
#   1. Change "MFEM_USE_MPI OFF" to ON. MFEM_USE_METIS follows it by default,
#      so remove "MFEM_USE_METIS OFF" as well.
#   2. Provide hypre and METIS. The simplest way is to let MFEM fetch them
#      itself by adding "MFEM_FETCH_HYPRE ON" and "MFEM_FETCH_METIS ON" below
#      (both options exist as of v4.10, see config/defaults.cmake). Or add them
#      here with CPM, before MFEM, and pass their locations to MFEM:
#        CPMAddPackage(NAME hypre GITHUB_REPOSITORY hypre-space/hypre
#                      GIT_TAG v2.33.0 SOURCE_SUBDIR src
#                      OPTIONS "HYPRE_ENABLE_MPI ON")
#        ... then "HYPRE_DIR ${hypre_BINARY_DIR}" (likewise METIS_DIR) in OPTIONS.
#   3. Find MPI before MFEM: find_package(MPI REQUIRED COMPONENTS CXX)
#   4. Link MPI::MPI_CXX (src/CMakeLists.txt) and run tests through mpiexec
#      (tests/CMakeLists.txt).
CPMAddPackage(
    NAME mfem
    GITHUB_REPOSITORY mfem/mfem
    GIT_TAG v4.10
    OPTIONS
        "MFEM_USE_MPI OFF"
        "MFEM_USE_METIS OFF"
        "MFEM_ENABLE_TESTING OFF"
        "MFEM_ENABLE_EXAMPLES OFF"
        "MFEM_ENABLE_MINIAPPS OFF"
)
