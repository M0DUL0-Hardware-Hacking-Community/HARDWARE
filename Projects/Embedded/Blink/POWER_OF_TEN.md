# Power of Ten application record

This record applies to `src/main.c` and `targets/pico-sdk/main.c`. ESP-IDF,
STM32Cube, avr-libc, the Pico SDK, board packages, compiler runtimes, startup code,
and bootloaders are third-party platform boundaries.

1. **Simple control flow:** Application code has no `goto`, recursion, function
   pointers, `setjmp`, or `longjmp`.
2. **Bounded work:** Each target has one intentionally nonterminating firmware loop,
   the scheduler exception allowed by Rule 2. STM32 also fail-stops if `HAL_Init()`
   fails; that loop has no operational work or exit path.
3. **No dynamic allocation:** Application code performs no dynamic allocation.
4. **Short functions:** Every application function is fewer than sixty physical
   lines.
5. **Assertions:** Both source files exhaustively check the active-high/active-low
   truth table at compile time. They also restrict polarity to `0` or `1` and reject
   zero or oversized delay values. Target-specific checks validate the configured
   GPIO shape or range. Compile-time checks do not meet the original rule's average
   runtime assertion density; this is the explicit application deviation.
6. **Smallest scope:** Pin, polarity, and timing values are build-time calibration
   constants. No mutable global state or shared state-machine abstraction exists.
7. **Checked interfaces:** ESP-IDF status returns and STM32 `HAL_Init()` are checked.
   The AVR register operations, STM32 GPIO/delay calls, and Pico GPIO/delay calls
   used here provide no status return to check.
8. **Restricted preprocessing:** Preprocessing is limited to includes, build-time
   constants, compile-time validation, and the three native PlatformIO target
   branches. Conditional compilation is the explicit hardware boundary.
9. **Restricted pointers:** Application code declares and dereferences no pointers.
   Native SDK declarations may accept pointers inside their platform boundary.
10. **Zero warnings:** PlatformIO compiles project source with `-Wall`, `-Wextra`,
    and `-Werror`; the Pico SDK target applies the same flags. Source-level static
    analysis covers both C files. Third-party framework code may use separate warning
    policies.

This is evidence for the application source, not a claim of strict whole-firmware
Power of Ten compliance.
