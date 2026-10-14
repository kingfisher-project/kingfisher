<!--
Please read the contributing guide first:
https://kingfisher-project.github.io/kingfisher/developer-guide/contributing.html
-->

# Description

Summarize the change and the motivation for it.

Fixes # (issue)

# AI Assistance

<!--
If an AI tool helped produce this pull request, disclose each model used:
the harness (e.g., Claude Code), the model, and the reasoning effort ("not
applicable" if the tool doesn't expose one). List each harness/model
combination if you used more than one. If no AI tools were used, write "None".
-->

- Harness:
- Model:
- Reasoning effort:

# Checklist

- [ ] I have performed a self-review of my own code
- [ ] I have run clang-format (version 18) on any C++ source files
- [ ] New C++ names follow the [Google C++ style](https://kingfisher-project.github.io/kingfisher/developer-guide/style.html#naming)
- [ ] New Python code follows PEP 8 (if applicable)
- [ ] I have added tests for every new function, class, and method, and a regression test for any bug fix
- [ ] New numerical methods have verification tests (manufactured or analytic solutions, convergence rates)
- [ ] Hot loops don't allocate memory or make virtual calls, and hot data is stored as structure of arrays
- [ ] I have updated the documentation (Doxygen comments, theory pages, input reference) (if applicable)
