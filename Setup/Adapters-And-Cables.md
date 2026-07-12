# Adapters and Cables

Interface adapters and cabling connect a development host to UART, SPI, I2C, JTAG,
SWD, flash memory, and other target interfaces. A correct protocol is not sufficient:
the connector pinout, reference voltage, ground, direction, and target power state
must also be correct.

Catalog reviewed: **12 July 2026**. Check current price, stock, lead time, and shipping
on the linked supplier page before ordering.

## Budget sourcing context

Shopee is normally the budget source for USB-UART adapters, jumper leads, test clips,
USB cables, generic SWD/JTAG probes, logic analysers, and pitch adapters in Southeast
Asia. Check the exact chipset, logic voltage, connector pitch, included harness, and
customer photos. Many similarly named adapters use different chips or omit proper
level shifting.

For Dupont wire, breadboards, pin headers, common sensors, breakout modules, and
soldering-related prototyping supplies, start with an established local maker seller:

- [MakerHub MY on Shopee](https://shopee.com.my/makerhub)
- [MakerHub Malaysia storefront](https://makerhub.my/)
- [MakerHub 40-piece Dupont wire](https://shopee.com.my/40pcs-Dupont-Wire-10cm-20cm-30cm-for-Breadboard-DIY-Experiment-Jumper-Wire-Breadboard-wire-i.1165814930.24676987244)

Maker-focused sellers are usually easier to compare by pin pitch, connector gender,
wire length, module voltage, chipset, and included documentation. Factory or warehouse
sellers may be cheaper for bulk generic wiring and housings; order a small sample and
continuity-test it before buying quantity.

TIGARD V1 is treated as a specialist exception. The supplied Mouser listing provides
an identifiable manufacturer part, distributor part number, interface specification,
and traceable product source that generic marketplace listings may not provide.

## TIGARD V1

| Attribute | Details |
| --- | --- |
| Manufacturer | Securing Hardware |
| Manufacturer part | `TIGARD-V1` |
| Mouser part | `392-TIGARD-V1` |
| Category | Interface development board for hardware hacking |
| Controller | FTDI FT2232H dual-channel USB interface |
| Interfaces | UART, SPI, JTAG, I2C, SWD, and USB |
| Reference/supply range | 1.8 V through 5 V selections, or target-supplied VTGT |
| Malaysian distributor page | [Mouser Malaysia TIGARD-V1](https://my.mouser.com/ProductDetail/Securing-Hardware/TIGARD-V1?qs=aP1CjGhiNiFnjSEE%2FnXyEw%3D%3D) |
| Design and usage documentation | [tigard-tools/tigard](https://github.com/tigard-tools/tigard) |

Mouser classifies TIGARD V1 as an interface development tool for the FT2232H and
lists UART, SPI/JTAG, I2C, SWD, and USB. It is not tied to one MCU vendor. It is most
useful for hardware-security labs, flash analysis, serial-console access, boundary
scan/debug work, and protocol experimentation.

### Architecture

TIGARD exposes two FT2232H channels. One is normally used as a USB serial UART. The
other uses the FTDI MPSSE engine for JTAG, SPI, I2C, or SWD. The mode switch changes
the shared signal routing:

- `JTAG/SPI`: signals remain separate for normal JTAG and SPI operation.
- `SWD/I2C`: data input/output are joined to form bidirectional SWDIO or SDA.

Dedicated headers cover UART, Cortex debug, JTAG, SPI flash, I2C/Qwiic-compatible
connections, and an external logic-analyser breakout. The LA header is intended to
let an external analyser observe communication; TIGARD is not primarily a logic
analyser.

### Voltage modes

The voltage switch controls the level-shifter reference and optionally VTGT:

- `1V8`, `3V3`, or `5V` powers the level-shifter side and can supply VTGT.
- `VTGT` disconnects onboard target power and takes the reference from the target.

This supports three distinct arrangements:

1. **Target-powered:** select `VTGT`, connect target VTGT, and let the powered target
   establish the logic reference.
2. **TIGARD-powered:** select a verified voltage and connect VTGT to an otherwise
   unpowered, low-current target.
3. **Self-powered interface:** select a voltage for TIGARD's level shifters but leave
   VTGT disconnected while the target uses its own supply.

Never move the voltage or mode switches by guesswork. Measure the target first. Do
not connect two active power supplies together, and do not assume a nominal 3.3 V
device lacks lower-voltage I/O domains.

### Compatible software

The official project documentation identifies standard FT2232H-compatible tools:

- USB serial drivers and terminal programs for UART
- OpenOCD and UrJTAG for JTAG/SWD workflows
- flashrom, libmpsse, and pyftdi for SPI flash and buses
- libmpsse and pyftdi for I2C
- iceprog for iCE40 FPGA programming
- avrdude for supported AVR programming workflows

Tool support depends on its FTDI backend, operating-system driver, target definition,
and configuration. Confirm the selected FTDI interface/channel before issuing erase,
write, reset, or debug commands.

### First-use checklist

1. Read the [official TIGARD documentation](https://github.com/tigard-tools/tigard).
2. Inspect the target schematic or identify ground and voltage with a multimeter.
3. Disconnect target power before attaching clips or loose jumper wires.
4. Set the voltage and protocol switches deliberately.
5. Connect ground before signal wires; connect VTGT only when the power plan requires it.
6. Verify the FT2232H enumerates and identify both host interfaces.
7. Start with a known board and a non-destructive operation such as UART receive or ID read.
8. Use a logic analyser or oscilloscope when signal direction or integrity is uncertain.
9. Do not write flash until a verified backup has been read more than once and hashed.

## Cable inventory

A useful adapter kit should include:

- Short female-female and female-male 2.54 mm jumpers
- A keyed 10-pin Cortex SWD cable and pinout adapter
- Suitable JTAG ribbon cables and pitch adapters
- SOIC-8 test clip or socket adapter for supported SPI flash packages
- Qwiic/STEMMA QT cable for the TIGARD I2C header
- USB data cable matching the adapter connector
- Grabber hooks for temporary test points
- Clearly labelled ground and VTGT leads

For ordinary prototyping, also keep separate 2.54 mm Dupont sets in male-male,
male-female, and female-female forms; solid breadboard jumpers; breakaway pin headers;
heat-shrink; and a proper Dupont-style crimp tool if making custom leads. Do not use
friction-fit Dupont jumpers where vibration, safety, or high current requires a keyed,
locking connector.

Keep cables short for faster buses, label both ends, and store target-specific pinout
notes with the cable or adapter. Never rely on wire colour alone.

Generic versions of these cables and clips are reasonable Shopee purchases when the
pitch, pin count, gender, keying, conductor count, and length are verified. Buy a few
spares, then continuity-test inexpensive harnesses before connecting valuable targets.

## Related setup

- [Logic analysers](Logic-Analysers.md)
- [Microcontrollers and boards](Microcontrollers-And-Boards.md)
- [ESD mats](ESD-Mats.md)
- [Storage and maintenance](Storage-And-Maintenance.md)
