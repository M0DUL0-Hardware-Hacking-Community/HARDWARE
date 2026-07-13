# Adding Other Microcontroller Platforms

No build system or binary is universal across all microcontrollers. Portability means
sharing the application source where a common framework exists and keeping native
SDK adapters small.

## Common vendor environments

| Family | Primary environment |
| --- | --- |
| Nordic nRF | [nRF Connect SDK](https://nrfconnectdocs.nordicsemi.com/ncs/latest/nrf/installation/install_ncs.html) |
| Zephyr-supported boards | [Zephyr getting started](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) |
| Microchip PIC/dsPIC/SAM | [MPLAB X](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide) |
| Texas Instruments MCU | [Code Composer Studio](https://www.ti.com/tool/CCSTUDIO) |
| Renesas MCU | [e2 studio](https://www.renesas.com/en/software-tool/e-studio) |
| NXP MCU | [MCUXpresso IDE](https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE) |
| Silicon Labs MCU | [Simplicity Studio](https://www.silabs.com/developers/simplicity-studio) |

Use the vendor's current compatibility matrix. IDE and SDK support can differ by host
architecture even when all three major desktop operating systems are listed.

## Repository structure

The Blink example uses one Arduino source for several board families and keeps native
SDK adapters separate:

```text
Projects/Embedded/Blink/
├── src/main.cpp
├── platformio.ini
└── targets/
    ├── pico-sdk/       # Unverified native example
    └── <other-sdk>/   # Add only when needed
```

Each target directory owns SDK initialization, startup/linker inputs, board selection,
GPIO/time access, build configuration, and flash/debug commands.

## Porting checklist

1. Identify the exact MCU, board revision, debugger, and connection.
2. Install the vendor-supported SDK and compiler in an isolated environment.
3. Build and flash the vendor's unmodified blink example first.
4. Reuse `src/main.cpp` through an Arduino-compatible core, or create a native target
   with the same on/delay/off/delay behavior.
5. Map the LED GPIO and active polarity from the schematic.
6. Use a documented delay or scheduler primitive with known units.
7. Check every SDK result and define a safe error state.
8. Add the target build and flash commands to its README.
9. Pin and record SDK, compiler, and programmer versions.
10. Add CI cross-builds and real-board hardware tests.

## Adapter contract

An adapter initializes one safe GPIO, drives it on for 500 ms, drives it off for
500 ms, and repeats through a bounded application loop or the platform scheduler. It
must not add heap allocation, unbounded retries, or silent error handling.

## Definition of supported

A platform should be listed as supported only after all of these pass:

- Cross-compilation with warnings treated as errors for project code
- Link and memory-map validation
- Flash through the documented programmer
- Cold boot and reset
- LED timing and polarity on the exact board
- Error recovery and watchdog behavior

A configuration example that has not been compiled or tested on hardware must be
labelled as an example, not as verified support.
