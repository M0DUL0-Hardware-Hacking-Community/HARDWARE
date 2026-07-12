# STM32 Development Environment

The Blink project includes a PlatformIO Arduino environment for STM32 Nucleo F401RE.
ST's native environment uses STM32Cube tools and requires a separate HAL/LL adapter.

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
  Arm C/C++, GDB, STM32CubeProgrammer, and device descriptions.

Download the host-specific installer from ST, verify its checksum when provided, and
follow the current installation guide. Verify the command-line installation with the
compiler and programmer versions exposed by the installed package.

A typical generated native project uses STM32CubeMX/CubeIDE to configure clocks,
GPIO, startup code, a linker script, and HAL or LL libraries. Flashing can use the
IDE or STM32CubeProgrammer through ST-LINK.

The current Blink `src/main.cpp` is an Arduino adapter. Native STM32 support requires
a `targets/stm32cube/` adapter that:

1. Initializes the generated clock and GPIO configuration.
2. Supplies a wrapping millisecond counter, commonly derived from the HAL tick.
3. Applies `blink::Output` through HAL, LL, or verified register access.
4. Links the unchanged `blink_core` sources.
5. Drives the LED to a known safe level on every error path.

## Debugging and release checks

- Confirm ST-LINK firmware and host tools are compatible.
- Verify the exact MCU part number and flash/RAM sizes.
- Keep generated clock, pin, startup, and linker inputs under version control.
- Use separate Debug and Release build directories.
- Inspect stack, heap, map file, interrupt vectors, and warning output.
- Test reset, watchdog, brownout, and power-loss behavior on the board.
- Pin STM32Cube firmware packages and compiler versions for released firmware.
