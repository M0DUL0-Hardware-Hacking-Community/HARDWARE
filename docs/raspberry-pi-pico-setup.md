# Raspberry Pi Pico Development Environment

This guide configures a machine to build and flash this repository's portable Blink
project for:

- Raspberry Pi Pico with RP2040
- Raspberry Pi Pico 2 with RP2350, using its Arm Cortex-M33 cores by default

The official Raspberry Pi Pico C/C++ SDK supports RP2350 starting with SDK 2.0.0.
Use a current stable SDK for both boards. The official
[Pico SDK repository](https://github.com/raspberrypi/pico-sdk) and
[Getting Started guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
are the authoritative references.

## Choose a setup path

The official Raspberry Pi Pico extension for Visual Studio Code is the recommended
setup for most developers. It manages the SDK, CMake, Ninja, Arm toolchain,
`picotool`, OpenOCD, and debugger configuration together. Use the manual setup for
headless machines, CI, custom IDEs, or tightly controlled tool versions.

Do not mix pieces from several installations unless their paths are explicit. A
common source of build failures is CMake finding an SDK from one installation and a
compiler or `picotool` from another.

## Option A: Official VS Code extension

The official extension supports Windows 10/11, macOS Sonoma 14 or newer, Linux x64
and arm64, and 64-bit Raspberry Pi OS. Install:

1. [Visual Studio Code](https://code.visualstudio.com/)
2. The official
   [Raspberry Pi Pico VS Code extension](https://github.com/raspberrypi/pico-vscode)
3. On macOS, the Command Line Tools with `xcode-select --install`

Windows and Raspberry Pi OS require no additional extension prerequisites. On other
Linux distributions, ensure Python 3.10+, Git 2.28+, and `tar` are installed. USB
debugging may additionally require distribution USB rules and libraries. Consult the
extension's current requirements rather than relying on old third-party installers.

Open `Projects/Embedded/Blink` in VS Code and use the extension to import it as an
existing Pico project. Select `pico` for RP2040 or `pico2` for RP2350. If using the
repository's command-line CMake files with an SDK installed by the extension, locate
that SDK and pass it explicitly:

```sh
cmake -S targets/pico-sdk -B build/pico-rp2040 \
  -DPICO_SDK_PATH=/absolute/path/to/pico-sdk \
  -DPICO_BOARD=pico
```

The path is intentionally explicit because the extension's managed installation
location can change between extension versions and operating systems.

## Option B: Manual command-line setup

Every manual installation needs:

- Git
- Python 3
- CMake
- A native C/C++ compiler for SDK host tools
- GNU Arm Embedded Toolchain with `arm-none-eabi-gcc` and `arm-none-eabi-g++`
- Raspberry Pi Pico SDK 2.0.0 or newer
- Ninja or Make
- Optional `picotool` for command-line flashing and inspection

### Debian, Ubuntu, and Raspberry Pi OS

```sh
sudo apt update
sudo apt install \
  build-essential cmake git ninja-build python3 \
  gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

The `libstdc++-arm-none-eabi-newlib` package name may differ or be bundled with the
Arm compiler on some distributions. If it is unavailable, install the distribution's
complete GNU Arm Embedded package or use the official VS Code extension.

Raspberry Pi OS also provides an official setup script for command-line systems:

```sh
wget https://raw.githubusercontent.com/raspberrypi/pico-setup/master/pico_setup.sh
chmod +x pico_setup.sh
./pico_setup.sh
```

Review downloaded scripts before executing them. Raspberry Pi documents this method
on its [C/C++ SDK setup page](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html).

### macOS

Install Apple's development tools first:

```sh
xcode-select --install
```

With [Homebrew](https://brew.sh/) installed:

```sh
brew install cmake git ninja python arm-none-eabi-gcc
```

The official VS Code extension is generally simpler because it keeps compatible
versions of these tools together. On Apple silicon, ensure every manually installed
tool is an arm64 build or runs consistently under the same architecture.

### Windows

The official VS Code extension is the preferred Windows setup because it installs
compatible tools without requiring global PATH changes. For a manual installation,
install current releases of:

- [Git for Windows](https://git-scm.com/download/win)
- [Python](https://www.python.org/downloads/windows/)
- [CMake](https://cmake.org/download/)
- [Ninja](https://github.com/ninja-build/ninja/releases)
- [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

Select installer options that add Git, Python, CMake, Ninja, and the Arm toolchain's
`bin` directory to PATH. Start a new PowerShell session after changing PATH.

## Install the Pico SDK

Install one SDK checkout that both RP2040 and RP2350 builds share:

```sh
mkdir -p "$HOME/pico"
git clone --branch master https://github.com/raspberrypi/pico-sdk.git \
  "$HOME/pico/pico-sdk"
git -C "$HOME/pico/pico-sdk" submodule update --init
```

Raspberry Pi defines the SDK's `master` branch as the latest stable release. A team
that needs reproducible builds should select and record a reviewed release tag rather
than following `master` automatically.

Set the SDK path for the current Unix shell:

```sh
export PICO_SDK_PATH="$HOME/pico/pico-sdk"
```

Add the export to `~/.zshrc`, `~/.bashrc`, or the relevant shell profile to make it
persistent. On Windows PowerShell:

```powershell
$env:PICO_SDK_PATH = "$HOME\pico\pico-sdk"
```

To persist it for future Windows sessions:

```powershell
[Environment]::SetEnvironmentVariable(
  "PICO_SDK_PATH",
  "$HOME\pico\pico-sdk",
  "User"
)
```

The Blink build also accepts `-DPICO_SDK_PATH=/absolute/path` when modifying the
environment is undesirable, such as in CI.

## Verify the installation

On Linux and macOS:

```sh
git --version
python3 --version
cmake --version
arm-none-eabi-gcc --version
arm-none-eabi-g++ --version
test -f "$PICO_SDK_PATH/external/pico_sdk_import.cmake"
```

On Windows PowerShell:

```powershell
git --version
python --version
cmake --version
arm-none-eabi-gcc --version
arm-none-eabi-g++ --version
Test-Path "$env:PICO_SDK_PATH\external\pico_sdk_import.cmake"
```

Every version command must succeed, and the final path check must report success.

## Build the Blink firmware

Run commands from `Projects/Embedded/Blink`.

### Raspberry Pi Pico RP2040

```sh
cmake -S targets/pico-sdk -B build/pico-rp2040 \
  -DPICO_BOARD=pico \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/pico-rp2040
```

Expected firmware:

```text
build/pico-rp2040/blink_pico.uf2
```

### Raspberry Pi Pico 2 RP2350

The normal Pico 2 build uses the Arm Cortex-M33 cores:

```sh
cmake -S targets/pico-sdk -B build/pico2-rp2350 \
  -DPICO_BOARD=pico2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/pico2-rp2350
```

Expected firmware:

```text
build/pico2-rp2350/blink_pico.uf2
```

`PICO_BOARD=pico2` selects RP2350 automatically. Never reuse an RP2040 build
directory for RP2350 or the reverse. Delete and recreate a build directory after
changing SDK, compiler, board, or processor platform.

## Flash over BOOTSEL USB storage

1. Disconnect the board from USB.
2. Hold `BOOTSEL`.
3. Connect USB while continuing to hold `BOOTSEL`.
4. Release the button when the storage volume appears.
5. Copy the matching UF2 file onto the volume.

The original Pico RP2040 volume is named `RPI-RP2`. The Pico 2 RP2350 volume is
named `RP2350`. The board disconnects the volume, reboots, and runs the firmware
after the copy completes.

Do not copy an RP2040 UF2 onto Pico 2 or an RP2350 UF2 onto the original Pico. Keep
the board name in the build-directory name to reduce this risk.

## Install and use picotool

`picotool` can inspect UF2/ELF files and communicate with RP2040 or RP2350 in
BOOTSEL mode. The official
[picotool repository](https://github.com/raspberrypi/picotool) links prebuilt Windows,
macOS, and Linux packages from
[pico-sdk-tools](https://github.com/raspberrypi/pico-sdk-tools).

After installation:

```sh
picotool version
picotool info -a build/pico-rp2040/blink_pico.uf2
picotool load -f build/pico-rp2040/blink_pico.uf2
picotool reboot
```

Use the RP2350 build path when flashing Pico 2. If CMake cannot locate a separately
downloaded `picotool`, pass its package directory during configuration:

```sh
cmake -S targets/pico-sdk -B build/pico-rp2040 \
  -DPICO_BOARD=pico \
  -Dpicotool_DIR=/absolute/path/to/picotool
```

On Linux, install the official `99-picotool.rules` from the picotool repository to
use USB operations without root. Reload udev rules and reconnect the board. Avoid
running the complete compiler or IDE as root.

## RP2350 RISC-V option

Raspberry Pi Pico 2 can alternatively run its Hazard3 RISC-V cores. This is optional;
the normal `PICO_BOARD=pico2` commands above use Arm and the GNU Arm toolchain.

RISC-V builds require the separate toolchain supplied through Raspberry Pi's
[pico-sdk-tools](https://github.com/raspberrypi/pico-sdk-tools). Start with an empty
build directory and configure both the platform and toolchain path:

```sh
export PICO_PLATFORM=rp2350-riscv
export PICO_TOOLCHAIN_PATH=/absolute/path/to/riscv-toolchain
cmake -S targets/pico-sdk -B build/pico2-riscv -DPICO_BOARD=pico2
cmake --build build/pico2-riscv
```

Do not set `PICO_PLATFORM=rp2350-riscv` globally unless every Pico 2 project should
use RISC-V. It can make expected Arm builds configure with the wrong compiler.

## CI and shared machines

- Pin an SDK release tag and compiler version.
- Pass `PICO_SDK_PATH` and `PICO_TOOLCHAIN_PATH` explicitly.
- Use separate build directories for `pico`, `pico2`, and `pico2` RISC-V.
- Cache downloaded SDKs and compilers, not generated CMake caches across platforms.
- Build both RP2040 and RP2350 on every change to portable firmware.
- Run native `blink_tests` before cross-compiling firmware.
- Store generated UF2 files as versioned CI artifacts.
- Flash and test real boards before release; compilation is not a hardware test.

## Troubleshooting

### Pico SDK path is missing

Confirm that `PICO_SDK_PATH` names the SDK root, not its `src` directory:

```sh
ls "$PICO_SDK_PATH/external/pico_sdk_import.cmake"
```

### CMake reports the wrong processor

Use a fresh build directory. RP2040 and RP2350 values are cached by CMake:

```sh
cmake -E remove_directory build/pico2-rp2350
cmake -S targets/pico-sdk -B build/pico2-rp2350 -DPICO_BOARD=pico2
```

Only remove generated build directories. Never remove source or SDK directories as
a troubleshooting step.

### Arm compiler is not found

Confirm `arm-none-eabi-gcc` and `arm-none-eabi-g++` are both on PATH. If installed in
a custom location, pass its parent directory with `-DPICO_TOOLCHAIN_PATH=...`.

### Board does not appear in BOOTSEL mode

Use a USB data cable, not a charge-only cable. Disconnect the board, hold `BOOTSEL`,
connect it directly to the computer, and then release the button. Try another cable
or USB port before changing drivers.

### picotool cannot access the board on Linux

Install the official udev rules, reload them, and reconnect the board. Group and rule
changes may require a logout and login. Avoid making USB devices globally writable.

### Firmware builds but the LED does not blink

Verify that the UF2 matches the board. The non-wireless Pico and Pico 2 use the SDK's
`PICO_DEFAULT_LED_PIN`. Pico W-family onboard LEDs are connected through the wireless
chip and require a different adapter; changing only `PICO_BOARD` is insufficient.
