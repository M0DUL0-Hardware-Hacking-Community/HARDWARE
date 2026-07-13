# Power of Ten C11 Coding Standard

This project applies the source paper in `reference/10rules.pdf` to a deliberately
small subset of C11. Deviations require a written safety case in the code review.

Unit tests verify selected behavior and recovery paths; they do not prove compliance
with structural rules. Control flow, loop bounds, allocation, function size,
preprocessing, pointers, and warning cleanliness require source review, compiler
checks, and static analysis. Hardware timing and GPIO behavior require board tests.

1. **Simple control flow.** Do not use `goto`, `setjmp`/`longjmp`, direct recursion,
   indirect recursion, or function pointers. Keep the call graph acyclic. Early
   error returns are allowed when they simplify control flow.
2. **Bound every loop.** Every loop has a statically visible maximum iteration
   count. Use fixed-capacity buffers and fail explicitly if the bound is reached.
   A deliberately nonterminating scheduler loop must be isolated and documented.
3. **No allocation after initialization.** Production operations use fixed arrays,
   fixed buffers, or statically sized pools. Do not use `malloc`, `calloc`, `realloc`,
   or `free` after initialization. Allocation in host-only test infrastructure is
   outside this rule.
4. **Short functions.** A function is at most 60 physical lines, including its
   declaration and local declarations. Prefer one independently testable purpose.
5. **Defensive checks.** Aim for at least two meaningful checks per function,
   covering preconditions, postconditions, invariants, and external results. Checks
   have no side effects and lead to an explicit recovery or error return.
6. **Minimal scope.** Declare data at the narrowest useful scope. Avoid mutable
   globals. Use `const` for values that do not change and keep variables
   single-purpose.
7. **Validate interfaces.** Validate every parameter. Check every non-`void` return
   value. An intentionally ignored result must be explicitly cast to `void` and
   justified where the reason is not obvious.
8. **Restrict preprocessing.** Use the preprocessor only for includes, include
   guards, and simple complete-unit macros. Do not use token pasting, variadic or
   recursive macros. Avoid conditional compilation outside platform boundaries.
9. **Restrict pointers.** Prefer values and fixed arrays with explicit capacities.
   Permit at most one dereference level. Pointer parameters do not own storage. Do
   not hide pointers in aliases or macros, and do not use function pointers.
10. **Zero warnings.** Build with maximum practical warnings and `-Werror` or `/WX`.
    Run at least one strong static analyzer for every change and resolve all findings.

## C11 restrictions

Write application code to C11 without depending on compiler extensions. Vendor build
systems may select a GNU C dialect for SDK headers, but project code remains in the
C11 subset. Variable-length arrays, implicit function declarations,
implementation-defined integer sizes in stored or wire formats, and hidden allocation
are prohibited. Use fixed-width integer types, fixed-capacity arrays, prefixed `enum`
status values, and explicit result checks. Public functions and types use the
`power10_` prefix.
