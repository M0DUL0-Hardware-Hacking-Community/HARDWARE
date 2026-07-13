# Arduino AVR and Arduino Uno Environment

The Blink project supports Arduino Uno through PlatformIO's Arduino framework. The
official Arduino CLI is also useful for native Arduino sketches and CI.

Official references:

- [Arduino CLI installation](https://docs.arduino.cc/arduino-cli/installation)
- [Arduino CLI getting started](https://arduino.github.io/arduino-cli/latest/getting-started/)
- [PlatformIO setup](platformio-setup.md)

## PlatformIO workflow

From `Projects/Embedded/Blink`:

```sh
pio run -e uno
pio run -e uno --target upload
```

The `uno` board definition supplies `LED_BUILTIN`, normally digital pin 13. Confirm
the board selection before upload; clones can use a different USB-serial interface.

## Install Arduino CLI

On macOS or Linux with Homebrew:

```sh
brew update
brew install arduino-cli
```

Arduino also provides signed/prebuilt downloads for Windows, macOS, Linux, and Linux
ARM from its official installation page. Put the executable on PATH and verify:

```sh
arduino-cli version
arduino-cli config init
arduino-cli core update-index
arduino-cli core install arduino:avr
arduino-cli board list
```

For a conventional Arduino sketch directory:

```sh
arduino-cli compile --fqbn arduino:avr:uno /path/to/sketch
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno /path/to/sketch
```

Use the port reported by `arduino-cli board list`; Windows uses `COM` ports and
macOS commonly uses `/dev/cu.usbmodem*` or `/dev/cu.usbserial*`.

The Blink project uses PlatformIO's `src/` layout and is not an Arduino CLI sketch.
Supporting Arduino CLI directly requires a conventional sketch containing the same
small setup/loop implementation.

## Hardware and upload checks

- Select `arduino:avr:uno`, not a similarly named board with another bootloader.
- Use a known USB data cable.
- Close serial monitors before uploading.
- Install the USB-serial driver required by a clone's interface chip.
- Never connect an LED without a current-limiting resistor.
- Respect AVR GPIO voltage and current limits from the board and MCU datasheets.

## Reproducibility

Record `arduino-cli version`, the `arduino:avr` core version, and the fully qualified
board name. In CI, install a specific reviewed core version rather than automatically
updating immediately before release builds.
