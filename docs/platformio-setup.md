# PlatformIO Development Environment

PlatformIO provides one project format and command-line interface across many
microcontroller families. The Blink project currently defines environments for
ESP32 DOIT DevKit V1, Arduino Uno, STM32 Nucleo F401RE, and Raspberry Pi Pico RP2040.

Official references:

- [PlatformIO installation](https://docs.platformio.org/en/latest/core/installation/)
- [PlatformIO IDE for VS Code](https://docs.platformio.org/en/stable/integration/ide/vscode.html)
- [PlatformIO Core installer](https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html)
- [PlatformIO project structure](https://docs.platformio.org/en/latest/core/quickstart.html)

## Choose one installation

Use either PlatformIO IDE for VS Code or standalone PlatformIO Core. The VS Code
extension already contains PlatformIO Core; installing another Core creates duplicate
package stores and makes it unclear which `pio` executable is running.

PlatformIO should run as the normal user. Do not install platforms or upload firmware
with `sudo` or an administrator shell.

## Option A: PlatformIO IDE for VS Code

1. Install [Visual Studio Code](https://code.visualstudio.com/).
2. Open Extensions and install the official `PlatformIO IDE` extension.
3. Restart VS Code when requested.
4. Open `Projects/Embedded/Blink` as the project folder.
5. Use the PlatformIO toolbar or its integrated terminal.

On Linux, install Python virtual-environment support before installing the extension:

```sh
sudo apt update
sudo apt install python3-venv
```

You do not need a separate `pio` installation when commands run from PlatformIO's
integrated terminal.

## Option B: Standalone PlatformIO Core

PlatformIO's installer creates an isolated Python environment. On macOS or Linux:

```sh
curl -fsSL -o get-platformio.py \
  https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 get-platformio.py
```

Review downloaded scripts before executing them. Follow PlatformIO's
[shell-command guide](https://docs.platformio.org/en/stable/core/installation/shell-commands.html)
to expose `pio` without installing a second copy.

On Windows, download `get-platformio.py` from the same official repository and run
it from PowerShell:

```powershell
python.exe .\get-platformio.py
```

For CI, PlatformIO recommends installing Core through Python package management in
an isolated virtual environment. Never install it into an unrelated security,
scientific, or system Python environment.

## Verify PlatformIO

```sh
pio --version
pio system info
pio boards esp32doit-devkit-v1
pio boards uno
pio boards nucleo_f401re
```

If `pio` resolves to an unexpected installation, inspect it with `which pio` on Unix
or `Get-Command pio` in PowerShell. Remove duplicate PATH entries before continuing.

## Build the Blink targets

From `Projects/Embedded/Blink`:

```sh
pio run -e esp32devkit_v1
pio run -e uno
pio run -e nucleo_f401re
pio run -e pico_rp2040
```

PlatformIO downloads the selected compiler, framework, uploader, and board metadata
on first use. This can take several minutes and requires network access.

Build and upload one target:

```sh
pio run -e esp32devkit_v1 --target upload
```

Specify a port only when automatic detection is ambiguous:

```sh
pio run -e esp32devkit_v1 --target upload --upload-port /dev/ttyUSB0
pio device list
pio device monitor --baud 115200
```

On Windows use a port such as `COM3`. macOS commonly uses `/dev/cu.usbserial-*` or
`/dev/cu.usbmodem*`. Linux commonly uses `/dev/ttyUSB0` or `/dev/ttyACM0`.

## Linux USB permissions

Install PlatformIO's official
[99-platformio-udev.rules](https://docs.platformio.org/en/latest/core/installation/udev-rules.html),
reload udev, and reconnect the board. Some distributions also require membership in
`dialout` or an equivalent serial group. Log out and back in after group changes.

Do not work around permissions with `sudo pio`, `chmod 777`, or globally writable USB
rules. Those approaches create root-owned PlatformIO caches and weaken the machine.

## Reproducible projects

An unversioned entry such as `platform = espressif32` tracks PlatformIO's selected
release. This repository pins the platform releases used by its build checks. When
upgrading one, validate the build and the exact hardware before committing the new
version:

```ini
[env:esp32devkit_v1]
platform = espressif32@7.0.1
board = esp32doit-devkit-v1
framework = arduino
```

Do not blindly copy version numbers from another project. Use `pio pkg list` to
record the actual framework, compiler, and uploader versions too.

## Troubleshooting

- **Source is not compiled:** PlatformIO expects application sources under `src/`.
- **Library is not found:** private reusable code belongs under `lib/<name>/src/`.
- **Wrong environment builds:** pass `-e <environment>` or check `default_envs`.
- **Upload cannot open port:** close serial monitors and install the correct USB rules.
- **Board connects and disappears:** use a known USB data cable and verify boot mode.
- **Python dependency conflicts:** use PlatformIO's isolated installer or VS Code Core.
- **Stale toolchain behavior:** run `pio run --target clean`; do not delete source files.
- **Different result in CI:** pin platforms and compare `pio pkg list` output.
