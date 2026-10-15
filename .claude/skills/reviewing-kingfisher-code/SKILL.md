---
name: reviewing-kingfisher-code
description: Reviews code changes in the Kingfisher codebase against its contribution criteria (correctness and numerics, data-oriented design, testing, Google C++ style, PEP 8, design, performance, docs, dependencies). Use when asked to review a PR, branch, patch, or set of code changes in Kingfisher.
---

Apply the repository-wide guidance in `AGENTS.md` (project overview, build and
test workflow, and conventions). The full style guide is
`docs/source/developer-guide/style.md` and the testing requirements are in
`docs/source/developer-guide/testing.md`.

## Determine Review Context

1. **Fetch PR metadata (if reviewing a PR).** If the user references a PR
   number, a branch associated with a PR, or a GitHub PR URL, get the PR
   details to find the base ref:
   - **Preferred:** `gh pr view <number> --json baseRefName,headRefName,title,body`
   - **Fallback:** the GitHub MCP server if available.
   - **Last resort:** WebFetch on the PR URL.
   - Use `baseRefName` as the diff base in the next step.

2. **Identify what to review.**
   - **PR review:** `gh pr diff <number>`, or `git diff <baseRefName>...HEAD`.
   - **No PR context:** `git diff main...HEAD`. Kingfisher's integration
     branch is `main`.
   - **User gives an explicit base or commit range:** use that.

3. **Read the changed files in context.** Look at the surrounding code, related
   headers, and the existing style to judge consistency.

4. **Look beyond the diff.** Ask "what else depends on the behavior being
   changed?" Grep for callers of changed functions, users of changed data
   structures, and tests that exercise them. Diffs that change a data layout or
   an invariant often break code that isn't in the diff.

## Review Criteria

Assess each area below. If an area looks good, briefly confirm it passes. If it
doesn't apply (for example Python in a C++-only PR), say "N/A".

### Purpose and Scope
- Do the changes have a clear, well-defined purpose?
- Is the PR focused, or does it mix unrelated changes that should be split?

### Correctness and Numerics
- Is all of the logic correct? Trace through it; don't assume.
- When the changes implement a numerical method (discretization, angular
  quadrature, sweep, iterative solver, preconditioner, acceleration):
  - Is the method sound, and consistent with established references? Are the
    references cited in comments or in `docs/source/theory/`?
  - Is the quadrature/integration order adequate for the polynomial order
    used?
  - Are conservation, positivity, and boundary conditions handled correctly?
  - Are there numerical stability, conditioning, or convergence concerns?
- Are edge cases and error conditions handled (zero cross sections, void
  regions, vacuum/reflective boundaries, degenerate elements, empty inputs)?

### Data-Oriented Design
This is a priority for Kingfisher. Flag designs that will be slow to iterate
over, even if they are clean from an object-oriented point of view.

- **Layout:** Is hot data stored as structure of arrays in contiguous buffers
  (`mfem::Vector`, `mfem::Array`, flat buffers indexed by group, angle, and
  element)? Flag arrays of per-element or per-angle objects,
  `std::vector<std::unique_ptr<T>>` for data that is iterated in bulk, and
  pointer-chasing in inner loops.
- **Hot loops** (angular sweeps, element assembly, source and group iterations):
  - No heap allocation (`new`, `std::make_unique`, growing containers,
    temporary `mfem::Vector`/`DenseMatrix` constructed per iteration).
    Workspaces should be allocated once and reused.
  - No virtual dispatch, `std::function`, or other indirect calls per element
    or per angle. Dispatch once, outside the loop.
  - No hidden copies (pass by value of large objects, implicit conversions).
- **Access patterns:** Does the loop order follow the memory order? Is the
  work batched (all elements for one angle, all groups for one element) rather
  than one item at a time through an object interface?
- **Separation of data and behavior:** Plain structs and free functions are
  preferred over class hierarchies when the data is processed in bulk.
- **Device readiness:** Does the code keep MFEM's memory model usable
  (`Read()`, `Write()`, `ReadWrite()`, `mfem::forall`), so that GPU execution
  remains possible?

### Testing
Every change in behavior must be tested. Missing tests for new behavior is a
`[Major]` finding.

- Does every new public function, class, and method have GoogleTest coverage in
  `tests/` (`*_test.cc`)?
- Does every bug fix include a regression test that fails without the fix?
- Do new numerical methods have verification tests: an analytic or
  manufactured solution, with the observed convergence rate checked against
  the expected rate where that applies?
- Are edge cases and error paths tested, not only the happy path?
- Are the tests meaningful? Flag tests that can't fail, assert only that code
  runs, or use tolerances so loose they wouldn't catch a real error.
- Will the tests still work when run in parallel later (no reliance on global
  state or fixed file paths outside `TEST_DIR`)?

### C++ Style (Google C++ Style Guide)
- Naming. CI runs clang-tidy (`.clang-tidy`), which checks the case of most
  names, so focus on what it can't check: whether `snake_case` methods are
  really simple accessors or mutators, file names, header guards, and whether
  names are clear and consistent with the rest of the codebase.
  - `PascalCase` for types and for functions and methods.
  - `snake_case` for variables and parameters.
  - `snake_case_` (trailing underscore) for class data members; plain
    `snake_case` for struct data members.
  - `kPascalCase` for constants (every `constexpr`, local ones included); only
    local non-`constexpr` `const` variables are `snake_case`.
  - `enum class` only; flag any plain `enum` declared in Kingfisher, since
    clang-tidy 18 can't check this. Enumerators are `PascalCase` with no `k`
    (`BoundaryCondition::Vacuum`).
  - `snake_case` for namespaces; `KINGFISHER_UPPER_CASE` for macros.
  - `snake_case.cc` / `snake_case.h` file names; tests are `*_test.cc`.
- Header guards are `KINGFISHER_<PATH>_<FILE>_H_`.
- Is the code clang-format clean (`.clang-format`, Google base style)? Are
  `NOLINT` comments limited to one check and justified?
- C++20 idioms where they help (`std::span`, concepts, ranges, designated
  initializers), without novelty for its own sake.
- Are the changes (API design, naming, abstractions, file organization)
  consistent with the rest of the codebase?

### Python Style
Kingfisher will add a Python API for generating YAML input files. When a PR
contains Python:
- Does it conform to PEP 8?
- Are there numpydoc docstrings and type hints on public functions and classes?
- Are filesystem paths handled with `pathlib.Path`?
- Are inputs validated in setters, with clear error messages?
- Is YAML written through a YAML library, never by string formatting?
- Are there pytest tests for new functions and classes?

### Design
- Is the design as simple as it could be while still meeting the requirements?
- Is there an existing facility in MFEM or in Kingfisher that does the same
  thing and should be used instead?
- Are there alternative designs that would be simpler or fit the data-oriented
  approach better?

### Memory and Performance
- Are there memory leaks, dangling references, or unclear ownership?
- Do the changes introduce performance regressions or much higher memory use?
- MPI readiness: do the changes add global or static state, or assumptions of a
  single process, that will break under domain decomposition?

### Documentation
- Do new public APIs have Doxygen comments?
- Are new methods described in `docs/source/theory/`, and new input options in
  `docs/source/user-guide/input.md`?
- Are breaking changes to the input format or the API clearly noted?

### Dependencies
- Do the changes add an external dependency? If so, is it justified, added
  through CPM in `cmake/Dependencies.cmake`, and pinned to a tag or version?

## Output Format

Produce your review as a structured report with these sections:

**Context**: What is being compared (for example "PR #12 against `main`", or
"current branch vs. `main`").

**Summary**: A short paragraph on what the changes do and your overall
assessment.

**Detailed Findings**: For each criterion above, a brief assessment. Use `✓`
for items that pass, and flag issues with a severity:
- `[Minor]`: style nits, small improvements, non-blocking suggestions
- `[Moderate]`: worth addressing, but not strictly blocking
- `[Major]`: should be resolved before merging

Group findings into:
1. **Blocking issues**: would justify requesting changes before merge.
2. **Non-blocking suggestions**: improvements that could be made now or later.
3. **Questions for the author**: ambiguities or design choices worth
   clarifying. Don't ask questions you can answer yourself by reading the code.

Each finding names the file and line (`path/to/file.cc:123`) and says what to
change, not only what is wrong.
