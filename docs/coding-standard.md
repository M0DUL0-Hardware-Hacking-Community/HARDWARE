# Power of Ten C++ Coding Standard

The source paper in `reference/10rules.pdf` was written for C. This project applies
its intent to a deliberately small subset of C++20. Exceptions require a written
safety case in the code review.

1. **Simple control flow.** Do not use `goto`, exceptions, `setjmp`/`longjmp`,
   direct recursion, indirect recursion, or function pointers. Keep the call graph
   acyclic. Early error returns are allowed when they simplify control flow.
2. **Bound every loop.** Every loop has a statically visible maximum iteration
   count. Use fixed-capacity containers and fail explicitly if the bound is reached.
   A deliberately nonterminating scheduler loop must be isolated and documented.
3. **No allocation after initialization.** Production operations use fixed-capacity
   storage (`std::array`, fixed buffers, or statically sized pools). Do not use
   `new`, `delete`, allocating standard containers, or hidden heap allocation after
   initialization. Allocation in host-only test infrastructure is outside this rule.
4. **Short functions.** A function is at most 60 physical lines, including its
   declaration and local declarations. Prefer one independently testable purpose.
5. **Defensive checks.** Aim for at least two meaningful checks per function,
   covering preconditions, postconditions, invariants, and external results. Checks
   have no side effects and lead to an explicit recovery or error return.
6. **Minimal scope.** Declare data at the narrowest useful scope. Avoid mutable
   globals. Prefer immutable values and single-purpose variables.
7. **Validate interfaces.** Validate every parameter. Check every non-`void` return
   value. An intentionally ignored result must be explicitly cast to `void` and
   justified where the reason is not obvious.
8. **Restrict preprocessing.** Use the preprocessor only for includes, include
   guards, and simple complete-unit macros. Do not use token pasting, variadic or
   recursive macros. Avoid conditional compilation outside platform boundaries.
9. **Restrict pointers.** Prefer references, values, `std::span`, and fixed arrays.
   Permit at most one dereference level. Do not hide pointers in aliases or macros,
   and do not use function pointers.
10. **Zero warnings.** Build with maximum practical warnings and `-Werror` or `/WX`.
    Run at least one strong static analyzer for every change and resolve all findings.

## C++ restrictions

Exceptions, RTTI, coroutines, unbounded templates, and implicit allocation make
resource or control-flow analysis harder. They are prohibited in safety-critical
production paths unless the project documents and mechanically verifies their
bounds. Prefer `enum class`, `std::array`, `std::span`, `std::optional`, and explicit
status values. This template uses exceptions only if a host toolchain does so before
production initialization; application code does not throw or catch.
