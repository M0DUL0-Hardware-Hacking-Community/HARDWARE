# All About UART (Adapted for SEA)

A hands-on guide to understanding and working with UART communication. UART (Universal Asynchronous Receiver-Transmitter) is found on a wide range of devices — routers, baby monitors, embedded systems, and more. We use the Raspberry Pi Pico as our practice target because it's cheap, accessible, and exposes GND, TX, and RX pins, but the skills here apply to any device with a UART header.

> **Why "Adapted"?** This course is adapted from the original by Matt Brown / Brown Fine Security. Equipment links and recommendations have been adjusted for Southeast Asian audiences, where Shopee, Lazada, and AliExpress are the preferred platforms over Amazon or other Western retailers.

---

## Required Equipment

| Item       | Description                                               | Link     |
| ---------- | --------------------------------------------------------- | -------- |
| **Target** | Raspberry Pi Pico / Pico 2 (or any device with UART pins) | [Buy](#) |
| **Cable**  | Micro-USB **data** cable (not charge-only)                | [Buy](#) |
| **Tool 1** | Multimeter                                                | [Buy](#) |
| **Tool 2** | UART to USB Adapter                                       | [Buy](#) |
| **Tool 3** | Logic Analyzer                                            | [Buy](#) |

## Optional Equipment

| Item             | Description                   | Link     |
| ---------------- | ----------------------------- | -------- |
| Soldering Iron   | For attaching header pins     | [Buy](#) |
| Lead-Free Solder | Recommended for health/safety | [Buy](#) |
| Header Pins      | Standard 2.54mm pitch         | [Buy](#) |
| Fume Extractor   | Recommended when soldering    | [Buy](#) |

---

## UART Theory

UART (Universal Asynchronous Receiver-Transmitter) is one of the oldest and most widely used serial communication protocols. It's found on routers, baby monitors, IoT devices, embedded systems — basically anything with a serial debug port. We use the Pi Pico as a practice target because it's cheap and easy to work with, but the skills here apply to any device with UART pins.

### What Makes UART Different

Unlike SPI and I2C, UART is **asynchronous** — there is no shared clock signal between devices. Both the transmitter and receiver must independently agree on the communication speed (baud rate) before data exchange begins. This makes UART very simple in terms of wiring but requires both sides to be configured identically.

**Key characteristics:**

- **Asynchronous** — no clock line needed
- **Full duplex** — TX and RX are separate lines, so both devices can send and receive at the same time
- **Point-to-point** — connects exactly two devices (no multi-slave bus like I2C)
- **Simple wiring** — only TX, RX, and GND required
- **Universal** — works across virtually all microcontrollers, computers, and communication modules

---

### The 3-Wire Interface: TX, RX, GND

A minimal UART connection requires only three wires:

![UART Wiring Diagram](images/uart-theory-diagram.png)

```
Device A              Device B
  TX  ────────────►  RX
  RX  ◄────────────  TX
 GND ──────────────  GND
```

#### TX (Transmit)

The output pin. The device drives this pin to send serial data. Bits are sent one at a time, with the **least significant bit (LSB) first** in standard UART. The idle state (no data being sent) is **HIGH**.

#### RX (Receive)

The input pin. The device monitors this pin to receive incoming serial data. It's a high-impedance input — never leave a floating RX pin if the UART peripheral is enabled, as noise can trigger false start bits.

#### GND (Ground)

Provides the common voltage reference. **Without a common ground, the receiver has no reference for what constitutes a HIGH or LOW voltage on the TX line.** This is one of the most common UART wiring mistakes — forgetting to connect GND between devices.

#### The Cross-Connection Rule

**TX of device A connects to RX of device B, and RX of device A connects to TX of device B.** Think of it as two people speaking — person A's mouth (TX) connects to person B's ear (RX), and vice versa.

> **Common mistake:** Connecting TX to TX and RX to RX. This does not work — both devices would be transmitting on the same wire and listening on the other, resulting in no communication or potential hardware damage.

---

### The UART Data Frame

UART sends data in structured packets called **frames**. Each frame contains the following components:

![UART Frame Format](images/uart-frame-format.png)

```
IDLE ─────┐ START ┌──┐  ┌──┐     ┌──┐  ┌──┐  ┌──┐  ┌──┐  ┌──┐  ┌──┐  ┌──┐  ┌─── STOP ─── IDLE
          │  (0)  │D0│  │D1│     │D2│  │D3│  │D4│  │D5│  │D6│  │D7│  │PAR│  │ (1)  │
          └───────┘  └──┘  └─────┘  └──┘  └──┘  └──┘  └──┘  └──┘  └──┘  └──┘  └──┘
```

#### Start Bit (1 bit)

The line is normally held **HIGH** when idle. To begin transmission, the transmitter pulls the line **LOW** for one bit period. This HIGH-to-LOW transition is how the receiver detects "data is coming." The start bit is always **0**.

#### Data Bits (5 to 9 bits)

The actual payload. **8 bits is the most common configuration** (one full byte). Data bits are transmitted **LSB first**. For example, if sending the letter 'S' (0x53 = 01010011 in binary), it's sent as: 1, 1, 0, 0, 1, 0, 1, 0.

#### Parity Bit (optional, 1 bit)

An error-detection bit inserted between the data bits and the stop bit. Two types:

- **Even parity** — the parity bit is set so the total number of 1s in the frame is even
- **Odd parity** — the parity bit is set so the total number of 1s in the frame is odd

Parity can only detect single-bit errors. If two or more bits flip, parity won't catch it. Many modern systems skip parity entirely and rely on higher-level checksums instead.

#### Stop Bit(s) (1, 1.5, or 2 bits)

Always **1** (HIGH). Signals the end of the frame and guarantees the line returns to idle before the next frame. One stop bit is most common; two stop bits give the receiver extra time to process but reduce throughput.

---

### 8N1: The Most Common Configuration

The shorthand **8N1** means:

| Parameter     | Value |
| ------------- | ----- |
| **Data bits** | 8     |
| **Parity**    | None  |
| **Stop bits** | 1     |

This gives **10 bits per byte** (1 start + 8 data + 1 stop). At 115200 baud, each byte takes ~86.8 microseconds, giving an effective throughput of **11,520 bytes per second**.

Other common configurations: **7E1**, **8E1**, **8O1**.

---

### Baud Rate: The Speed of Communication

Baud rate is the number of signal changes (symbols) per second. For UART, where each symbol is one bit, baud rate equals **bits per second (bps)**. Both devices **must** be configured to the same baud rate — if they differ, the receiver will misinterpret bits and you'll see **garbage data**.

#### Standard UART Baud Rates

| Baud Rate  | Common Use                                                       |
| ---------- | ---------------------------------------------------------------- |
| **9600**   | Default for Arduino Serial, GPS modules, most AT-command modules |
| **19200**  | Some industrial equipment                                        |
| **38400**  | HC-05 Bluetooth AT command mode                                  |
| **57600**  | Debug consoles                                                   |
| **115200** | Most modern embedded devices, ESP32, Raspberry Pi Pico           |
| **230400** | High-speed sensor data                                           |
| **460800** | Firmware flashing                                                |
| **921600** | Fast data transfer                                               |

> **Why 9600?** Historically, it was the fastest reliable speed for early serial hardware. It's still the universal default for GPS modules and AT-command devices.

> **Why 115200?** It's the fastest standard baud rate that works reliably on most modern microcontrollers without special configuration.

#### The 2% Rule

Both sides must be within **2%** of each other's baud rate for reliable communication. At 9600 baud, that's a tolerance of ±192 bps. Mismatches beyond this cause framing errors and garbled output.

---

### Voltage Levels and Logic Families

Different systems operate at different voltages. **Connecting them without level shifting can damage hardware.**

| Standard       | Voltage Levels | Logic                       | Common Devices                     |
| -------------- | -------------- | --------------------------- | ---------------------------------- |
| **TTL (3.3V)** | 0V to 3.3V     | Active HIGH                 | Raspberry Pi, ESP32, STM32, Pico   |
| **TTL (5V)**   | 0V to 5V       | Active HIGH                 | Arduino Uno, ATmega                |
| **RS-232**     | ±3V to ±25V    | **Inverted** (1 = negative) | PC COM ports, industrial equipment |
| **RS-485**     | Differential   | Differential                | Industrial networks, Modbus        |

> **Important:** RS-232 is **inverted** — a logic 1 is a negative voltage, and a logic 0 is a positive voltage. This is the opposite of TTL. You need a transceiver chip (like MAX3232) to convert between TTL and RS-232.

> **Level shifting:** A 5V TX signal going into a 3.3V RX pin can damage the input. Use a voltage divider or logic level converter when connecting devices with different voltage levels.

---

### Flow Control

When the receiver can't process data fast enough, data can be lost. Flow control prevents this.

#### Hardware Flow Control (RTS/CTS)

Two additional pins:

- **RTS (Request to Send)** — the receiver asserts this when it's ready to accept data
- **CTS (Clear to Send)** — the transmitter checks this before sending

The transmitter waits for CTS to be asserted before sending data. This is reliable but requires two extra wires.

#### Software Flow Control (XON/XOFF)

Uses special characters in the data stream:

- **XON (0x11)** — "resume sending"
- **XOFF (0x13)** — "stop sending"

No extra wires needed, but these characters can't appear in the actual data, making it unsuitable for binary data.

---

### UART vs SPI vs I2C

| Feature          | UART                                                    | SPI                                               | I2C                                     |
| ---------------- | ------------------------------------------------------- | ------------------------------------------------- | --------------------------------------- |
| **Wires**        | 2-3 (TX, RX, GND)                                       | 4+ (SCLK, MOSI, MISO, CS)                         | 2 (SDA, SCL) + GND                      |
| **Clock**        | None (asynchronous)                                     | Shared clock (SCLK)                               | Shared clock (SCL)                      |
| **Duplex**       | Full duplex                                             | Full duplex                                       | Half duplex                             |
| **Speed**        | Up to ~1 Mbps typical                                   | Up to 100+ MHz                                    | 100 kHz to 5 MHz                        |
| **Multi-device** | Point-to-point only                                     | One master, multiple slaves (CS per slave)        | Multi-master, multi-slave (addressed)   |
| **Complexity**   | Simple                                                  | Moderate                                          | Moderate                                |
| **Best for**     | Debug consoles, GPS, Bluetooth, simple device-to-device | High-speed data: SD cards, displays, flash memory | Multiple sensors, low-pin-count systems |

**Use UART when:** You need simple, reliable communication between two devices, or you're interfacing with a debug/serial console.

---

### Common Devices That Use UART

- **GPS modules** (e.g., NEO-6M) — output NMEA sentences at 9600 baud
- **Bluetooth modules** (HC-05, HC-06) — AT command configuration and data
- **ESP8266/ESP32** — programmable via UART at 115200 baud
- **Raspberry Pi Pico** — debug serial at 115200 baud
- **Routers** — serial console for firmware recovery and debug
- **GSM/GPRS modules** (SIM800L) — AT commands at 9600/19200 baud
- **RFID readers** — send card UIDs over UART

---

### Common Pitfalls and Debugging

| Problem                                | Likely Cause                             | Fix                                                                               |
| -------------------------------------- | ---------------------------------------- | --------------------------------------------------------------------------------- |
| **Garbage characters**                 | Baud rate mismatch                       | Verify both sides use the same baud rate                                          |
| **No data at all**                     | TX/RX not crossed, or wrong pins         | Swap TX and RX wires                                                              |
| **Random/system resets**               | Missing common ground                    | Connect GND between devices                                                       |
| **Intermittent errors**                | Voltage level mismatch                   | Check if devices operate at the same logic voltage; use a level shifter if needed |
| **"Almost readable but wrong"**        | Frame format mismatch (e.g., 8N1 vs 7E1) | Match data bits, parity, and stop bits on both sides                              |
| **Random data when nothing connected** | Floating RX pin                          | Enable internal pull-up resistor on RX                                            |

<!-- Add a diagram showing TX/RX wiring between Pico and adapter -->

---

## Lab Setup

Before we can use any of the tools, we need to prepare our target device and workspace. This section covers flashing the firmware onto the Pi Pico, setting up your soldering station, and soldering header pins.

---

### Flashing the Firmware

The firmware files for this course are included in this repository under the `firmware/` directory. There are two versions — one for the Pi Pico (RP2040) and one for the Pi Pico 2 (RP2350). **Make sure you flash the correct one for your board.**

#### Which Firmware Do I Have?

| Board | Chip | Firmware File |
|-------|------|---------------|
| Raspberry Pi Pico / Pico W | RP2040 | `firmware/all_about_uart_pico.uf2` |
| Raspberry Pi Pico 2 / Pico 2 W | RP2350 | `firmware/all_about_uart.uf2` |

> **Important:** The RP2040 UF2 will **not** work on the Pico 2, and vice versa. The Pico 2's bootloader only recognizes RP2350 binaries. If you flash the wrong one, the board will appear as a USB drive again instead of rebooting.

#### Flashing Steps (Same for Pico and Pico 2)

1. **Download the correct UF2 file** from the `firmware/` folder in this repository.

2. **Enter BOOTSEL mode:**
   - Hold down the **BOOTSEL button** on the Pico (the small white button on the board).
   - While holding it, **plug the USB cable** into the Pico and your computer.
   - Release the BOOTSEL button once the Pico appears as a USB mass storage device.
     - Pi Pico shows up as **RPI-RP2**
     - Pi Pico 2 shows up as **RP2350**

3. **Drag and drop** the `.uf2` file onto the USB drive that appeared.

4. The Pico will **automatically reboot** and the USB drive will disappear. That's it — firmware is flashed.

<!-- Screenshot: USB drive showing RPI-RP2 or RP2350, with the UF2 file being dragged onto it -->

![Flashing UF2](images/flash-uf2-drag-drop.png)

> **Tip:** You only need to hold BOOTSEL when flashing new firmware. After that, just plug in the USB cable normally and the Pico will boot into the firmware you just flashed.

> **Re-flashing:** To flash a different firmware later, just hold BOOTSEL again while plugging in — it always works, even if there's already firmware on the board.

---

### Setting Up the Soldering Station

The Raspberry Pi Pico comes without header pins attached. You'll need to solder them on before you can connect jumper wires to the UART pins.

#### Lead-Free Solder Temperature

Lead-free solder (typically SAC305 — Sn96.5/Ag3.0/Cu0.5) has a higher melting point than traditional leaded solder. Set your soldering station to the correct range:

| Solder Type | Melting Point | Iron Temperature Setting |
|-------------|---------------|--------------------------|
| **Lead-free (SAC305)** | 217–220°C | **350°C – 400°C** |
| Leaded (63/37 Sn/Pb) | 183°C | 315°C – 370°C |

**For this course, use lead-free solder at 350°C–380°C.** This gives enough heat to melt the solder quickly without overheating the components.

> **Why higher temperature?** Lead-free solder needs more heat to flow properly. If you set it too low (below 350°C), the solder won't wet the pad and pin correctly, resulting in **cold joints** — dull, grainy, weak connections.

> **Don't go too high either.** Above 400°C risks damaging the PCB pads, burning the flux core, and shortening your iron tip's lifespan.

---

#### Station Setup Checklist

1. **Turn on the iron** and let it heat up to your set temperature (usually 350–380°C for lead-free). This takes 1–3 minutes.

2. **Tin the tip** — melt a small amount of fresh solder onto the tip before your first joint. The tip should look shiny and silver, not dull or black.

3. **Wet the sponge** — squeeze the included sponge so it's damp (not dripping). Use this to wipe the tip between joints.

4. **Turn on the fume extractor** if you have one. Lead-free solder flux fumes are irritating to breathe — always work in a ventilated area or use a fume extractor positioned near the work.

<!-- Screenshot: Soldering station with temperature display showing 360°C, iron in stand, sponge wet -->

![Soldering Station Setup](images/soldering-station-setup.png)

---

### Soldering Header Pins onto the Pico

You need two 1x20 pin headers (2.54mm pitch / 0.1" standard) — one for each side of the Pico.

#### What You'll Need

- Raspberry Pi Pico (or Pico 2)
- 2x 20-pin male header strips (break a longer strip down to 20 pins if needed)
- Lead-free solder wire
- Soldering iron set to ~360°C
- Breadboard or spare PCB (to hold the Pico steady while soldering)
- Fume extractor (recommended)

#### Step-by-Step

1. **Insert the headers into a breadboard** first — long pins down into the breadboard, short pins pointing up. This holds them straight and steady. Space the two rows to match the Pico's pin spacing.

<!-- Screenshot: Headers inserted into a breadboard, long pins down, short pins up -->

![Headers in Breadboard](images/headers-in-breadboard.png)

2. **Place the Pico on top** of the short pins, making sure the pin holes line up. The Pico should sit flat against the breadboard with the pins poking through.

<!-- Screenshot: Pico placed on top of headers in the breadboard -->

![Pico on Headers](images/pico-on-headers.png)

3. **Solder one pin first** (any pin) — touch the iron tip to the pad and pin simultaneously for 1–2 seconds, then feed solder from the opposite side. The solder should flow and form a small cone shape.

<!-- Screenshot: Close-up of one pin being soldered -->

![First Pin Soldered](images/first-pin-soldered.png)

4. **Check alignment** — make sure the Pico is sitting flat and the headers are straight. If not, reheat the one pin and adjust.

5. **Solder the remaining 39 pins** — work your way around. For each pin:
   - Touch the iron to the pad + pin (1–2 seconds)
   - Feed solder from the other side
   - Remove solder wire
   - Remove iron
   - Let it cool (1–2 seconds)

6. **Inspect your joints** — each one should look like a small, shiny cone (volcano shape). Dull, blobby, or cracked joints are cold joints and need to be redone.

<!-- Screenshot: Completed soldering on one side, showing clean joints -->

![Completed Solder Joints](images/completed-solder-joints.png)

> **Common mistakes:**
> - **Too much solder** — blobby joint, might bridge two pins together
> - **Too little solder** — pin not fully connected
> - **Iron too cold** — solder won't flow, joint looks grainy
> - **Iron too hot / held too long** — pad lifts off the board, burnt flux

> **Fixing mistakes:** Use a **solder sucker** (desoldering pump) to remove excess solder. Heat the joint with the iron, then press the sucker's release button to suck up the molten solder.

---

### Verifying the Setup

After flashing and soldering, verify everything works.

#### Setting Up PlatformIO (Recommended)

We use **C** for all code in this course. [VSCode](https://code.visualstudio.com/) with the [PlatformIO extension](https://platformio.org/install/ide?install=vscode) is the recommended setup — it handles the toolchain, board definitions, and flashing all in one.

1. Install VSCode.
2. Open VSCode, go to **Extensions** (left sidebar), search for **PlatformIO IDE** and install it.
3. Restart VSCode when prompted.
4. Click the PlatformIO icon (alien head) in the left sidebar and select **New Project**.
5. Name your project, select **Raspberry Pi Pico** (or **Pico 2**) as the board, and **Arduino** as the framework.
6. PlatformIO will set up the project structure and download the necessary toolchain.

> **Other IDEs work too** — CLion, STM32CubeIDE, or even bare `arm-none-eabi-gcc` with Make/CMake. Use whatever you're comfortable with. PlatformIO just makes it easier to get started.

---

#### Test 1: Blink the Onboard LED

Open `src/main.c` (PlatformIO creates this for you) and replace its contents with:

```c
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }
}
```

Build and flash — the onboard LED should blink once per second.

---

#### Test 2: UART Output

Replace `src/main.c` with:

```c
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

int main() {
    stdio_init_all();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    while (true) {
        uart_puts(UART_ID, "Hello from Pico!\r\n");
        sleep_ms(1000);
    }
}
```

Build, flash, and connect a USB-to-UART adapter to GP0 (TX) and GP1 (RX). You should see "Hello from Pico!" at 115200 baud in your serial terminal (screen, picocom, minicom, etc.).

> **Note for Pico 2 users:** The code is identical — the Pico SDK handles the RP2040 vs RP2350 differences automatically.

---

## UART and Logic Analyzers

A Logic Analyzer captures digital signals on wires and lets you decode what's being communicated. We use the **HiLetgo 8-channel 24MHz USB Logic Analyzer** (a cheap FX2-based clone) with **PulseView** — the open-source GUI from the sigrok project. This section covers both setting up the software and connecting the hardware.

---

### Part 1: Software — Installing PulseView / sigrok

PulseView is the GUI; sigrok is the backend that talks to the hardware. The HiLetgo logic analyzer uses the **fx2lafw** driver (generic driver for FX2-based logic analyzers).

#### Ubuntu / Debian

```bash
sudo apt install pulseview sigrok-firmware-fx2lafw
```

#### Fedora

```bash
sudo dnf install pulseview sigrok-firmware-fx2lafw
```

#### Arch Linux

```bash
sudo pacman -S pulseview sigrok-firmware-fx2lafw
```

#### openSUSE

```bash
sudo zypper install pulseview sigrok-firmware-fx2lafw
```

#### macOS

Download the nightly DMG from [sigrok.org/wiki/Downloads](https://sigrok.org/wiki/Downloads) and drag PulseView.app into your Applications folder.

> **Apple Silicon (M1/M2/M3/M4) note:** PulseView's DMG is built for x86_64 and runs under Rosetta 2. If it crashes on launch with a missing `libintl.8.dylib` error, you need to install the x86_64 version of `gettext` via a Rosetta Homebrew prefix. See the [full workaround here](https://gist.github.com/mandrean/fbe00ac1fc11b1b2625e8e9e53c1f7dc).

#### Windows

Download the installer from [sigrok.org/wiki/Downloads](https://sigrok.org/wiki/Downloads). You may need to install the **WinUSB** driver using [Zadig](https://zadig.akeo.ie/) before PulseView can see the device. See [sigrok.org/wiki/Windows](https://sigrok.org/wiki/Windows) for driver instructions.

---

#### USB Permissions (Linux)

On Linux, non-root users can't access USB devices by default. Add your user to the `plugdev` group:

```bash
sudo groupadd -f plugdev
sudo usermod -aG plugdev $USER
```

**Log out and back in** for the group change to take effect.

> **If PulseView still can't see the device:** Make sure the udev rules are installed. The `sigrok-firmware-fx2lafw` package should handle this, but if not, you can manually copy the rules from the sigrok source.

---

#### Connecting to the Device in PulseView

1. Plug the HiLetgo logic analyzer into a USB port. You should see faint LEDs illuminate on the board.

2. Open PulseView.

3. Click the device dropdown (top toolbar, says `<No Device>` by default) and select **Connect to Device**.

4. In the dialog, select **fx2lafw** as the driver and **USB** as the interface.

5. Click **Scan for devices using driver above**.

6. Your device should appear as **"Logic with 8 channels"**. Select it and click **OK**.

<!-- Screenshot: PulseView device selection dialog -->

![PulseView Connect to Device](images/pulseview-connect-device.png)

You should now see eight colored channels (D0 through D7) in the main window, matching the CH0–CH7 labels on the physical analyzer.

---

#### Adding the UART Protocol Decoder

Once you've captured a signal, you can decode it:

1. Click the **"Add protocol decoder"** button in the toolbar (yellow and green icon).

2. Search for and double-click **UART**.

3. A new decode track appears at the bottom. Click the settings icon to configure:
   - **RX (UART receive line):** Set to the channel your target's TX pin is connected to (e.g., D0)
   - **Baud rate:** Set to match your target (e.g., 115200, 9600)
   - **Data format:** Change to **ascii** for readable output

4. Zoom in to see the decoded bytes and annotations.

<!-- Screenshot: PulseView with UART decoder showing decoded output -->

![PulseView UART Decode](images/pulseview-uart-decode.png)

> **Tip:** If you don't know the baud rate, use the **cursors** tool to measure the width of one bit, then calculate `1 / bit_width` to get the baud rate. Or try the common rates (9600, 115200) until the decode looks correct.

---

### Part 2: Hardware — Connecting the Logic Analyzer

Now that the software is ready, here's how to physically connect the logic analyzer to your target device to capture UART traffic.

#### What You'll Need

- HiLetgo USB Logic Analyzer (or similar FX2-based 8-channel LA)
- Female-to-female jumper wires (for header pin targets) or male-to-female (for test points)
- Your target device with exposed UART pins (Pico, router, baby monitor, etc.)
- Multimeter (to help identify GND if needed)

---

#### Step 1: Identify UART Pins on Your Target

Before connecting anything, you need to find **TX**, **RX**, and **GND** on your target device.

<!-- Screenshot: Target device with UART pins labeled (Pico example) -->

![Target UART Pins](images/target-uart-pins.png)

**How to find them:**

- **Check the datasheet/schematic** — look for pins labeled UART, SERIAL, or DEBUG
- **Look for a 3 or 4-pin header** — many devices have a dedicated debug header
- **Use a multimeter** — GND will show continuity with any ground point on the board (USB shield, mounting holes, ground plane)

> **On the Raspberry Pi Pico:** UART pins are GP0 (TX) and GP1 (RX), next to GND on the board header.

> **On routers:** Look for a 4-pin header near the board edge — often labeled J1 or JP1. Pin 1 is usually GND.

---

#### Step 2: Connect GND First

Always connect ground before anything else.

<!-- Screenshot: GND wire connected between logic analyzer and target -->

![GND Connection](images/la-gnd-connection.png)

- Connect a **GND pin** from the logic analyzer (one of the GND pins on the header) to the **GND** on your target device.

> **Why GND first?** Without a common ground reference, the logic analyzer can't correctly interpret voltage levels. You'll get random noise or no signal at all.

---

#### Step 3: Connect the Target's TX to a Logic Analyzer Channel

Connect the target device's **TX pin** to one of the logic analyzer's input channels (e.g., **D0/CH0**).

<!-- Screenshot: TX wire connected from target to LA channel D0 -->

![TX to D0 Connection](images/la-tx-d0-connection.png)

> **Important:** You're connecting the target's **TX** (transmit) to the logic analyzer's **input** channel. The logic analyzer is a passive listener — it only reads signals, it doesn't transmit. So TX goes to an input, and you **don't** need to connect RX unless you want to capture bidirectional traffic.

---

#### Step 4: (Optional) Connect RX for Bidirectional Capture

If you also want to capture data going **to** the target (e.g., commands you send), connect the target's **RX pin** to another channel (e.g., **D1/CH1**).

<!-- Screenshot: Both TX and RX connected to LA channels -->

![TX and RX Connection](images/la-tx-rx-connection.png)

---

#### Step 5: Plug the Logic Analyzer into Your Computer

Insert the USB end of the logic analyzer into a USB port on your computer. The LEDs on the board should light up.

<!-- Screenshot: Logic analyzer plugged into USB with LEDs on -->

![LA USB Connected](images/la-usb-connected.png)

---

#### Step 6: Power On the Target Device and Capture

1. In PulseView, set your **sample rate** (1 MHz is sufficient for UART) and **sample count** (e.g., 50M samples for ~50 seconds of capture at 1 MHz).

2. Click **Run** in PulseView.

3. **Power on your target device** (or reset it) so it sends UART data during boot.

4. You should see square wave activity appear on the channel(s) you connected.

5. Click **Stop** when you're done.

<!-- Screenshot: PulseView showing captured UART waveform on D0 -->

![Captured UART Waveform](images/pulseview-captured-waveform.png)

---

#### Step 7: Decode the Capture

1. Add the **UART decoder** (as described in Part 1).
2. Set the RX channel to match your wiring (e.g., D0).
3. Set the baud rate (try 115200 or 9600).
4. Zoom in to see the decoded text.

<!-- Screenshot: Final decoded UART output in PulseView -->

![Final UART Decode](images/pulseview-final-decode.png)

---

#### Wiring Summary

```
Target Device                    Logic Analyzer
┌──────────┐                    ┌──────────────┐
│      TX  ├────────────────────┤ D0 (CH0)     │
│      RX  ├──── (optional) ───┤ D1 (CH1)     │
│     GND  ├────────────────────┤ GND          │
└──────────┘                    └──────────────┘
                                     │
                                     │ USB
                                     ▼
                                ┌──────────┐
                                │ Computer │
                                └──────────┘
```

> **Remember:** TX on the target goes to an input channel on the logic analyzer. The logic analyzer is read-only — it captures but doesn't transmit.

---

## Interacting With UART

Once your hardware is wired up and the logic analyzer is capturing, you'll want to actually talk to the UART port from your computer. This section covers the tools and commands you'll use most often.

### Finding Your UART Device

When you plug in a USB-to-UART adapter (or the Pico's built-in UART), Linux assigns it a device file — usually `/dev/ttyUSB0` or `/dev/ttyACM0`. Here's how to find the right one.

#### Plug In and Check `dmesg`

```bash
dmesg | tail
```

<!-- Screenshot: dmesg output showing ttyUSB0 or ttyACM0 detection -->

![dmesg output](images/dmesg-uart.png)

You should see something like:

```
[12345.678] usb 1-1: new full-speed USB device number 7 using xhci_hcd
[12345.789] usb 1-1: New USB device found, idVendor=1a86, idProduct=7523
[12345.790] usb 1-1: Product: USB Serial
[12345.791] ch341-uart converter now attached to ttyUSB0
```

The key line is `attached to ttyUSB0` — that's your device.

#### List Serial Devices

```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

If nothing shows up, the driver might not be loaded or the cable might be charge-only.

#### Check with `lsusb`

```bash
lsusb
```

<!-- Screenshot: lsusb output showing UART adapter -->

![lsusb output](images/lsusb-uart.png)

Common UART adapter chips you'll see:

| Chip | ID Vendor:Product | Typical Device |
|------|-------------------|----------------|
| CH340/CH341 | `1a86:7523` | `/dev/ttyUSB0` |
| CP2102 | `10c4:ea60` | `/dev/ttyUSB0` |
| FT232R | `0403:6001` | `/dev/ttyUSB0` |
| Pico (CDC) | `2e8a:000a` | `/dev/ttyACM0` |

> **Pico note:** When the Pico is running firmware with `stdio_init_all()`, it appears as a CDC ACM device (`/dev/ttyACM0`), not `/dev/ttyUSB0`. Use `dmesg` to confirm.

#### Permissions (Linux)

If you get `Permission denied` when trying to open the device, add yourself to the `dialout` group:

```bash
sudo usermod -aG dialout $USER
```

Log out and back in for the change to take effect. This only needs to be done once.

---

### Terminal Tools

There are many ways to do this.

#### Screen

<!-- Screenshot of screen session connected to UART -->

![Screen Command](images/screen-example.png)

```bash
screen /dev/ttyUSB0 115200
```

To exit screen: `Ctrl+A`, then `K`, then `Y`.

#### Picocom

<!-- Screenshot of picocom session -->

![Picocom Example](images/picocom-example.png)

```bash
picocom -b 115200 /dev/ttyUSB0
```

To exit picocom: `Ctrl+A`, then `Ctrl+X`.

#### Minicom

<!-- Screenshot of minicom session -->

![Minicom Example](images/minicom-example.png)

```bash
minicom -b 115200 -D /dev/ttyUSB0
```

To exit minicom: `Ctrl+A`, then `X`.

---

### Custom Python Scripts

<!-- Screenshot of Python script output or code -->

![Python Script](images/python-script-example.png)

For scripted interactions, `pyserial` is the standard:

```bash
pip install pyserial
```

```python
import serial

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
ser.write(b'Hello UART!\n')
response = ser.readline()
print(response)
ser.close()
```

---

### Quick Reference

| Task | Command |
|------|---------|
| Find device | `dmesg \| tail` |
| List serial ports | `ls /dev/ttyUSB* /dev/ttyACM*` |
| Check USB devices | `lsusb` |
| Connect (screen) | `screen /dev/ttyUSB0 115200` |
| Connect (picocom) | `picocom -b 115200 /dev/ttyUSB0` |
| Connect (minicom) | `minicom -b 115200 -D /dev/ttyUSB0` |
| Fix permissions | `sudo usermod -aG dialout $USER` |

---

## Credits

- Matt Brown ([@nmatt0](https://twitter.com/nmatt0)) — Brown Fine Security
- Andrew Bellini ([@D1gitalAndrew](https://twitter.com/D1gitalAndrew))
