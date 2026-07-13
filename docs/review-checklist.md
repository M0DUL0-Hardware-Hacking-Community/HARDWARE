# Review Checklist

- [ ] All application sources compile as C11; public functions and types use `power10_`.
- [ ] The call graph remains acyclic; no recursion, `goto`, or callbacks.
- [ ] Every loop has an obvious compile-time maximum.
- [ ] Runtime paths perform no heap allocation or deallocation.
- [ ] Every function is at most 60 physical lines.
- [ ] Preconditions, postconditions, and invariants have side-effect-free checks.
- [ ] Variables have minimal scope and one purpose.
- [ ] Parameters and non-`void` results are checked.
- [ ] Preprocessor usage is limited and each conditional is justified.
- [ ] Pointer use has no more than one dereference level.
- [ ] Strict C compilation, tests, clang-tidy, and cppcheck report zero warnings.
