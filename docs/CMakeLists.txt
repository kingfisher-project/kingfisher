# docs/CMakeLists.txt
#
# Builds the Doxygen reference and the Sphinx site. The tools are not fetched;
# install them first:
#   pip install -r docs/requirements.txt     (Sphinx and its extensions)
#   doxygen and graphviz from the system package manager

find_package(Doxygen COMPONENTS dot)
if(NOT Doxygen_FOUND)
    message(FATAL_ERROR
        "Building the docs needs Doxygen and Graphviz (dot). Install them with "
        "your package manager, e.g. `apt install doxygen graphviz` or "
        "`brew install doxygen graphviz`.")
endif()

find_program(SPHINX_BUILD sphinx-build)
if(NOT SPHINX_BUILD)
    message(FATAL_ERROR
        "Building the docs needs Sphinx. Install it into your Python "
        "environment with `pip install -r docs/requirements.txt`.")
endif()
find_program(SPHINX_AUTOBUILD sphinx-autobuild)

# Doxygen theme. Only the CSS is needed, so nothing is built.
CPMAddPackage(
    NAME doxygen-awesome-css
    GITHUB_REPOSITORY jothepro/doxygen-awesome-css
    VERSION 2.4.2
    DOWNLOAD_ONLY YES
)

set(DOCS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(DOCS_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(DOCS_HTML_DIR "${DOCS_BUILD_DIR}/html")
set(DOXYGEN_AWESOME_DIR "${doxygen-awesome-css_SOURCE_DIR}")
configure_file(Doxyfile.in "${DOCS_BUILD_DIR}/Doxyfile" @ONLY)

# The Doxygen HTML goes inside the Sphinx output (html/doxygen), so that it is
# published at <site>/doxygen/.
add_custom_target(
    docs-doxygen
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOCS_HTML_DIR}"
    COMMAND Doxygen::doxygen "${DOCS_BUILD_DIR}/Doxyfile"
    WORKING_DIRECTORY "${DOCS_SOURCE_DIR}"
    COMMENT "Building the Doxygen reference"
    VERBATIM
)

set(SPHINX_ARGS
    -D "breathe_projects.kingfisher=${DOCS_BUILD_DIR}/doxygen-xml"
    "${DOCS_SOURCE_DIR}/source"
    "${DOCS_HTML_DIR}"
)

# Warnings are errors, as they are in continuous integration.
add_custom_target(
    docs
    COMMAND "${SPHINX_BUILD}" -b html -W --keep-going ${SPHINX_ARGS}
    COMMENT "Building the Sphinx site in ${DOCS_HTML_DIR}"
    VERBATIM
)
add_dependencies(docs docs-doxygen)

if(SPHINX_AUTOBUILD)
    # Live preview. Rebuild docs-doxygen to refresh the API reference.
    add_custom_target(
        docs-serve
        COMMAND "${SPHINX_AUTOBUILD}" ${SPHINX_ARGS}
        USES_TERMINAL
        VERBATIM
    )
    add_dependencies(docs-serve docs-doxygen)
endif()
