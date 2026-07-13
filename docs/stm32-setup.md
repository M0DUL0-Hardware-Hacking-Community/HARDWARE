# STM32 Development Environment

The Blink project builds its C11 application for STM32 Nucleo F401RE with
PlatformIO's STM32Cube framework. The application uses the STM32 HAL directly.

Official references:

- [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html)
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html)
- [PlatformIO setup](platformio-setup.md)

## PlatformIO workflow

From `Projects/Embedded/Blink`:

```sh
pio run -e nucleo_f401re
pio run -e nucleo_f401re --target upload
```

The Nucleo board normally uploads through its onboard ST-LINK debugger. Confirm the
board is `NUCLEO-F401RE`; a different Nucleo model can have another MCU, memory map,
clock configuration, or LED pin.

On Linux, install the PlatformIO USB rules and any ST-LINK rules supplied by the
selected tooling. Reconnect the board after updating udev rules.

## Native STM32Cube workflow

Choose one ST package:

- STM32CubeIDE: graphical configuration, build, flash, and debug environment.
- STM32CubeCLT: 64-bit Windows, Linux, and macOS command-line toolset containing GNU
  Arm C/C++, GDB, STM32CubeProgrammer, and device descriptions. “Arm C/C++” is ST's
  toolchain name; this project invokes its C compiler for application code.

Download the host-specific installer from ST, verify its checksum when provided, and
follow the current installation guide. Verify the command-line installation with the
compiler and programmer versions exposed by the installed package.

A typical generated native project uses STM32CubeMX/CubeIDE to configure clocks,
GPIO, startup code, a linker script, and HAL or LL libraries. Flashing can use the
IDE or STM32CubeProgrammer through ST-LINK.

The Blink `src/main.c` STM32 branch:

1. Initializes HAL on the reset/default clock, then enables and configures the LED GPIO.
2. Drives the LED on, waits for the configured half-period, drives it off, and waits again.
3. Repeats that sequence in an intentional nonterminating firmware loop.
4. Fail-stops if `HAL_Init()` fails before GPIO setup; that early failure cannot guarantee an
   LED indication.

## Debugging and release checks

- Confirm ST-LINK firmware and host tools are compatible.
- Verify the exact MCU part number and flash/RAM sizes.
- Keep generated clock, pin, startup, and linker inputs under version control.
- Use separate Debug and Release build directories.
- Inspect stack, heap, map file, interrupt vectors, and warning output.
- Test reset, watchdog, brownout, and power-loss behavior on the board.
- Pin STM32Cube firmware packages and compiler versions for released firmware.
