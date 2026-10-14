# Contributing

How to propose a change to Kingfisher.

## Pull requests

1. Create a branch off `main`.
2. Make your changes, following the [code style](style.md), and add [tests](testing.md) and [documentation](documentation.md) for them.
3. Open a pull request against `main` and fill in the template, including the AI Assistance section.
4. CI then runs automatically:
   - **CI** builds Kingfisher and runs the tests (`cmake --workflow --preset debug`).
   - **C++ Format and Lint Check** checks the changed files with clang-format 18 and clang-tidy 18. Add the `cpp-format-suggest` label to get the fixes as inline suggestions.
   - **Docs** builds the documentation.
   - **Claude Code Review** posts a review once the pull request is opened or marked ready for review. The review criteria are in `.claude/skills/reviewing-kingfisher-code/SKILL.md`.
5. Address the review comments. To get a new Claude review after pushing changes, comment `@claude review` on the pull request.
6. A maintainer merges the pull request once CI passes and the review is resolved.

Open the pull request as a draft while it is still in progress; the Claude review waits until it is marked ready.

## Reporting a bug

Open an issue on [GitHub](https://github.com/kingfisher-project/kingfisher/issues) with the input file, the command you ran, the output or error, and the Kingfisher version or commit.
