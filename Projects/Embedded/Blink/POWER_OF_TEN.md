# Power of Ten application record

This record applies to `src/main.cpp` and `targets/pico-sdk/main.cpp`. Arduino, the
Pico SDK, board packages, compiler runtimes, and bootloaders are third-party platform
boundaries.

1. **Simple control flow:** Application code has no `goto`, recursion, exception
   handling, or indirect function calls.
2. **Bounded work:** `setup()` contains no loop. Each Arduino `loop()` call performs
   two fixed GPIO writes and two fixed delays before returning; the Arduino runtime
   owns perpetual callback scheduling. The native Pico example uses an explicitly
   nonterminating loop with no exit path, the scheduler exception allowed by Rule 2.
3. **No dynamic allocation:** Application code performs no dynamic allocation.
4. **Short functions:** Every application function is fewer than twenty physical
   lines.
5. **Assertions:** Four compile-time checks cover the active-high/active-low truth
   table, and the configured polarity is restricted to `0` or `1`. Compile-time
   configuration checks do not satisfy the original rule's average runtime assertion
   density. This is the application code's explicit deviation: its only parameter is
   an exhaustive two-value boolean covered by that truth table.
6. **Smallest scope:** Board pin and polarity values remain file-local. Timing
   constants live only in the functions that use them; no shared state-machine
   abstraction remains for two fixed GPIO levels.
7. **Checked interfaces:** No returned status is discarded. The Arduino and Pico
   GPIO/delay APIs used here return `void` and provide no failure status to check.
8. **Restricted preprocessing:** Preprocessing is limited to includes, missing-board
   configuration guards, and simple object-like board/SDK constants.
9. **Restricted pointers:** Application code declares and dereferences no pointers.
10. **Zero warnings:** Every PlatformIO environment compiles project source with
    `-Wall`, `-Wextra`, and `-Werror`, and CI runs source-level cppcheck on both
    adapters. Board packages may apply separate policies or emit warnings while
    compiling third-party framework files.

Rule 5's assertion density is an explicit application deviation. Platform internals
remain outside this evidence, so this is not a claim of strict application or
whole-firmware compliance.
