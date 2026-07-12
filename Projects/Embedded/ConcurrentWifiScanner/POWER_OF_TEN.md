# Power of Ten compliance record

This record applies to the application code in `src/`. ESP-IDF, FreeRTOS, the
Wi-Fi binary, C library, bootloader, and toolchain runtime are third-party
boundary code and are not claimed to comply with the repository's C++ standard.

## Rule-by-rule evidence

1. **Simple control flow:** Application code has no `goto`, recursion,
   exceptions, `setjmp`, or `longjmp`. `-fno-exceptions` is enabled. FreeRTOS
   requires task-entry function pointers; the three entries are isolated,
   validate their context, terminate explicitly, and are the documented boundary
   exception.
2. **Bound every loop:** SSID copying is bounded by 32 bytes; scanning and report
   receipt by three cycles; report logging by 20 records; LED flashing by two
   pulses; and the LED state machine by 400 ticks. Runtime report counts are
   validated before use.
3. **No allocation after initialization:** All application task stacks, task
   controls, queue storage, event storage, reports, and buffers have fixed sizes.
   The application does not use `new`, `delete`, or allocating standard
   containers. ESP-IDF allocates its internal Wi-Fi scan list and driver objects
   during boundary operations and frees scan records when retrieved.
4. **Short functions:** The automated audit confirms every application function
   is below 60 physical lines.
5. **Defensive checks:** Task contexts, queue/event handles, report counts, loop
   inputs, GPIO results, scan results, initialization results, cleanup results,
   and timeouts are checked. Compile-time assertions protect fixed capacities.
6. **Minimal scope:** Mutable runtime state is held in one static storage object
   because FreeRTOS tasks outlive `app_main` stack frames. Other values use the
   narrowest practical scope.
7. **Validate interfaces:** Application parameters are references, values, or
   required SDK boundary pointers. Every fallible ESP-IDF/FreeRTOS result is
   checked or passed to explicit cleanup/error handling.
8. **Restrict preprocessing:** Application preprocessing is limited to includes
   and the header include guard. SDK macros are used only at the platform
   boundary.
9. **Restrict pointers:** Application domain logic uses values and fixed arrays.
   Opaque SDK handles and the single-level task-context cast are required boundary
   exceptions; there is no application pointer arithmetic or multi-level
   dereference.
10. **Zero warnings:** Host tests and firmware compile with strict warnings as
    errors. Firmware application sources also run GCC `-fanalyzer`; exceptions
    and RTTI are disabled. `clang-format --dry-run --Werror` and
    `git diff --check` are part of the verification commands.

## Security-assessment scope

Risk labels are deterministic heuristics based only on advertised authentication,
cipher, and WPS metadata. A low label means no obvious issue was visible in those
fields; it is not proof of security. The application performs no authentication,
credential testing, injection, deauthentication, or exploitation.
