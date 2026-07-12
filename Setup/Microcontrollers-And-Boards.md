# Microcontrollers and Boards

Board-selection and sourcing notes for Raspberry Pi computers and microcontrollers,
with emphasis on availability in Malaysia and Southeast Asia.

Catalog reviewed: **12 July 2026**. Prices and stock are deliberately not copied into
recommendations because storefront data can change without notice.

## Budget sourcing context

Shopee is widely used in Southeast Asia for budget development boards, compatible
breakouts, headers, breadboards, jumper wires, USB cables, and kits. It can be the
best-value source when official provenance is not essential and the buyer can verify
the listing.

Use Argon40 or Cytron for official or less-common Raspberry Pi items that are not
reliably available through budget marketplace listings. For Raspberry Pi boards,
power supplies, and safety- or compatibility-critical accessories, compare the final
Shopee price with a specialist seller after shipping and warranty. Very small savings
may not justify uncertain RAM capacity, board revision, power rating, or authenticity.

## Raspberry Pi product types

Raspberry Pi products in these catalogs fall into two different development models:

| Product family | Examples | Development model |
| --- | --- | --- |
| Linux single-board computers | Raspberry Pi 5, Pi 4, Zero 2 W | Install an OS, then deploy Linux applications |
| Microcontroller boards | Pico RP2040, Pico 2 RP2350, wireless variants | Cross-compile firmware and flash UF2 images |
| Bare microcontrollers | RP2040, RP2350A, RP2350B | Design a PCB using the datasheet and hardware design guide |
| Compute modules and industrial boards | CM-series and carrier solutions | Product integration with carrier and thermal design |

Do not buy a Raspberry Pi computer when a deterministic microcontroller is required,
or a Pico when the project needs a full Linux userspace. See the repository's
[development environment index](../docs/development-environments.md) before choosing.

## Suppliers

| Supplier | Region and currency | Useful for | Catalog |
| --- | --- | --- | --- |
| Argon40 | International; Malaysia selectable with MYR | Official Raspberry Pi products and Argon40 cases/accessories | [Raspberry Pi Official Products](https://argon40.com/collections/raspberry-pi-official-products) |
| Cytron Technologies | Malaysia; MYR | Local boards, kits, headers, power, robotics, HATs, cameras, displays, and support | [Raspberry Pi category](https://my.cytron.io/c-raspberry-pi?r=1) |
| Cytron Technologies | Malaysia; MYR | Pico, Pico 2, wireless variants, and bare RP-series MCUs | [Raspberry Pi MCU category](https://my.cytron.io/c-raspberry-pi-mcu) |
| Shopee | Southeast Asian marketplace; local currency | Budget-compatible boards, kits, headers, cables, and generic accessories | Search by exact part and revision; verify each seller independently |

Check taxes, shipping, warranty handling, plug type, and header options before comparing
the final cost. A low board-only price may exclude the power supply, storage, cable,
cooling, headers, or case needed for a usable system.

No single Shopee seller is listed here because sellers, URLs, stock, and product
quality change frequently. Prefer recent local reviews with photos of the exact PCB,
not reviews aggregated across several variants in one listing.

## Argon40 catalog snapshot

The supplied Argon40 collection identifies itself as official Raspberry Pi products
and showed approximately 50 product listings at review time. Malaysia was available
as a storefront region using MYR.

Observed product groups included:

- Raspberry Pi 5 and Raspberry Pi 4 boards
- Raspberry Pi Zero and Zero 2 W boards
- Raspberry Pi Pico, Pico W, and Pico 2 microcontroller boards
- Bare RP2040 microcontrollers
- Official USB-C power supplies, cases, and the Raspberry Pi 5 Active Cooler
- Raspberry Pi 5 M.2 HAT+, camera modules, and touch displays
- Raspberry Pi AI Camera, AI Kit, and AI HAT+ variants

Argon40 also manufactures its own cases, storage, cooling, and power accessories.
Confirm whether a listing belongs to the official Raspberry Pi collection or is an
Argon40 accessory, and verify compatibility with the exact Pi model.

Direct catalog:
[argon40.com/collections/raspberry-pi-official-products](https://argon40.com/collections/raspberry-pi-official-products)

## Cytron Malaysia catalog snapshot

Cytron's Raspberry Pi catalog is organized into main boards, Pico expansion, kits,
power supplies, HAT/carrier boards, enclosures, displays, cameras, accessories,
industrial products, and books. This makes it useful when a project needs a complete
local bundle rather than a board alone.

Observed product groups included:

- Raspberry Pi 5, Raspberry Pi 4, bundles, and related accessories
- Raspberry Pi Pico RP2040 and Raspberry Pi Pico 2 RP2350
- Pico/Pico 2 basic kits and options with pre-soldered headers
- Pico W and Pico 2 W wireless variants
- Bare RP2040 and RP2350A/RP2350B microcontrollers
- Official and third-party power supplies, cases, cooling, and cables
- Camera Module 3, HQ Camera, AI Camera, and other camera modules
- Robotics kits, motor-control platforms, HATs, relay boards, and cellular HATs
- Raspberry Pi Debug Probe and educational material

The dedicated Pico 2 listing offered SMD-friendly, pre-soldered-header, and bundled
options at review time. It describes RP2350A, 520 KB SRAM, 4 MB QSPI flash, 3.3 V
GPIO, USB drag-and-drop programming, SWD, and GP25 as the onboard LED. Always verify
these specifications against Raspberry Pi's current official documentation.

Direct catalogs:

- [All Raspberry Pi products](https://my.cytron.io/c-raspberry-pi?r=1)
- [Raspberry Pi microcontrollers](https://my.cytron.io/c-raspberry-pi-mcu)
- [Raspberry Pi Pico 2](https://my.cytron.io/p-raspberry-pi-pico2-board)

## Board selection

| Goal | Starting point | Important additions |
| --- | --- | --- |
| Learn bare-metal C/C++ | Pico RP2040 | Micro-USB data cable, headers, breadboard |
| Learn newer secure MCU features | Pico 2 RP2350 | Micro-USB data cable, Pico SDK 2.0+, optional debug probe |
| Add Wi-Fi/Bluetooth to MCU work | Pico W or Pico 2 W | Wireless-capable SDK adapter and antenna clearance |
| Run Linux IoT services | Raspberry Pi 5 or Pi 4 | Correct PSU, storage, cooling, case, micro-HDMI cable |
| Small Linux endpoint | Zero 2 W | Suitable PSU, microSD, adapters, header if required |
| Design a custom PCB | RP2040 or RP2350 | Datasheet, design guide, reference design, assembly capability |

Pico W-family onboard LEDs are controlled through the wireless device rather than the
same direct GPIO arrangement as non-wireless Pico boards. Firmware and tutorials must
use the correct board adapter.

## Before ordering

1. Record the exact product and board revision.
2. Confirm RP2040 versus RP2350 and wireless versus non-wireless.
3. Decide between bare, SMD-friendly, and pre-soldered-header versions.
4. Check logic voltage; Pico-family GPIO is 3.3 V and is not 5 V tolerant.
5. Confirm USB connector type and that the cable carries data.
6. For Pi 5, budget for a compliant power supply and appropriate cooling.
7. For Linux boards, select reliable boot storage and plan backups.
8. For bare ICs, use the official design guide, errata, and reference design.
9. Recheck local stock, lead time, warranty, and total delivered cost.
10. For marketplace listings, save screenshots of the selected variant and stated
    specifications before ordering.

## Development documentation

- [Raspberry Pi Pico environment setup](../docs/raspberry-pi-pico-setup.md)
- [PlatformIO environment setup](../docs/platformio-setup.md)
- [Firmware flashing and deployment](../docs/flashing-and-deployment.md)
- [Portable Blink project](../Projects/Embedded/Blink/)
