# Power of Ten application record

This record applies to application code in `src/`. ESP-IDF, FreeRTOS, the Wi-Fi
binary, C library, bootloader, and toolchain runtime are third-party boundaries.

## Rule-by-rule evidence

1. **Simple control flow:** Application code has no `goto`, recursion, exceptions,
   `setjmp`, or `longjmp`. `app_main` owns scanning and reporting. FreeRTOS requires
   one LED task-entry function pointer; that checked boundary is the sole exception.
2. **Bound every loop:** SSID escaping is bounded by 32 input bytes; BSSID work by
   6 bytes; scanning by 3 sweeps; driver-record retrieval by `UINT16_MAX`; catalog
   lookup and reporting by 128 entries; and the LED task by a fixed 60-second
   deadline.
3. **No allocation after initialization:** The LED task, event group, catalog, AP
   records, and buffers use static or fixed-capacity storage. Application code uses
   no `new`, `delete`, or allocating containers. ESP-IDF owns and releases its
   internal scan list at the platform boundary.
4. **Short functions:** Every application function remains at most 60 physical
   lines. Mapping and reporting helpers each have one purpose.
5. **Defensive checks:** Country setup, Wi-Fi lifecycle calls, scan completion,
   record retrieval, catalog state, BSSIDs, sweep order, advertised country bytes,
   GPIO results, task creation, event waits, capacity limits, and cleanup results are
   checked explicitly. The original rule's average runtime assertion density is not
   met; fake assertions in small mapping and logging helpers would have no recovery
   value, so this is an explicit application-level deviation.
6. **Minimal scope:** `app_main` exclusively owns scan results. The LED task receives
   only an event-group handle. No queue, mutex, logger task, or shared result writer
   exists.
7. **Validate interfaces:** Application parameters are values or references. The
   sole SDK `void*` task context is checked before its single-level cast. Every
   fallible SDK result is handled or explicitly logged during best-effort cleanup.
8. **Restrict preprocessing:** Application preprocessing is limited to includes and
   header guards. ESP-IDF formatting and assertion macros remain at the SDK boundary.
9. **Restrict pointers:** Domain logic uses values and fixed arrays. Opaque SDK
   handles and the task-context cast are required single-level boundary pointers;
   application code performs no pointer arithmetic.
10. **Zero warnings:** Project code requests `-Wall`, `-Wextra`, `-Werror`, GCC
    `-fanalyzer`, disabled exceptions, and disabled RTTI. ESP-IDF may append targeted
    warning exemptions for its component and framework build. Host tests use strict
    warnings as errors.

The FreeRTOS callback, runtime assertion density, SDK preprocessing/pointers, and
vendor warning exemptions are explicit deviations; this is not a claim of strict
application or whole-firmware compliance.

## Bounded simplifications

- The catalog uses a linear BSSID lookup. Its hard ceiling is 128 entries; replace it
  only if measured scan time shows the bounded lookup matters.
- The deployment country is a compile-time hardware setting. This build uses `MY`;
  change it only when the physical deployment country changes.
- Detailed output uses normal ESP-IDF AP records. Raw beacon information elements
  require a separate promiscuous-mode parser and are intentionally excluded.

## Security-assessment scope

Risk labels are deterministic heuristics based only on advertised authentication,
pairwise/group cipher, and WPS metadata. A low label means no obvious issue appeared
in those fields; it is not proof of security. The application performs no
association, credential testing, injection, deauthentication, or exploitation.
