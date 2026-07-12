# Logic Analysers

A logic analyser samples digital signals and helps decode UART, SPI, I2C, JTAG, and
other buses. It does not replace a multimeter or oscilloscope: it normally cannot show
analogue voltage shape, ringing, rise time, overshoot, or unsafe voltage.

Catalog guidance reviewed: **12 July 2026**.

## Budget sourcing

- [Search Shopee Malaysia for 8-channel USB logic analyser](https://shopee.com.my/search?keyword=8%20channel%20usb%20logic%20analyzer)
- [Search Shopee Malaysia for FX2 logic analyser](https://shopee.com.my/search?keyword=fx2%20logic%20analyzer)
- [Search Shopee Malaysia for logic analyser test hooks](https://shopee.com.my/search?keyword=logic%20analyzer%20test%20hooks)

The common low-cost 8-channel USB units often use a Cypress FX2-compatible design.
Compatibility should be checked against [sigrok fx2lafw](https://sigrok.org/wiki/Fx2lafw)
and [PulseView](https://sigrok.org/wiki/PulseView), not assumed from a case that says
"24 MHz" or resembles another product.

### Preferred seller type

Generic FX2 analysers are commonly cheapest from factory or warehouse sellers. Prefer
listings that identify the USB chipset/VID:PID, show the actual PCB and input network,
include the complete harness/hooks, and demonstrate PulseView or sigrok detection.
Reject listings that bundle unauthorised proprietary software or use only copied case
photos without the PCB.

## Starter recommendation

A verified sigrok-compatible 8-channel FX2 unit is useful for low-speed embedded
work when its limits are understood. Look for:

- USB identity/chipset confirmed in recent reviews
- Eight labelled channels plus ground leads
- Test hooks included or available separately
- Stated input-voltage range and digital thresholds
- Buffer/input protection information
- PulseView/sigrok evidence on the buyer's operating system
- A USB data cable and strain relief

Do not assume the advertised maximum sample rate is a reliable bus frequency. Capture
at several times the highest signal frequency and validate questionable results with
an oscilloscope. Cheap units commonly share USB bandwidth and provide little input
protection.

## Capability levels

| Level | Suitable work | Upgrade trigger |
| --- | --- | --- |
| Budget FX2, 8 channels | UART, I2C, slow SPI, GPIO timing | Higher speed, long captures, uncertain thresholds |
| Buffered USB analyser | Faster buses and deeper captures | Need analogue integrity or isolation |
| Mixed-signal oscilloscope | Correlating digital decode and analogue waveform | High channel count or specialised protocol work |

## Connection checklist

1. Measure the target voltage with a multimeter first.
2. Confirm the analyser's maximum input voltage and threshold compatibility.
3. Connect a common ground before signal channels.
4. Keep ground and signal leads short, especially as speed rises.
5. Start at the slowest target clock and a high sampling ratio.
6. Label channels in software before collecting evidence.
7. Save the raw capture, decoder settings, firmware version, and wiring notes.

Never connect a non-isolated USB logic analyser to mains-referenced, automotive,
industrial, negative-voltage, or unknown circuits. Use properly rated isolation and
instrumentation designed for the environment.

## TIGARD interaction

TIGARD has an LA header intended to expose traffic to an external logic analyser. Its
FT2232H can be used experimentally for capture, but the official TIGARD documentation
says that is not its intended purpose. Use a dedicated analyser when observing TIGARD
UART, SPI, JTAG, or I2C traffic. See [Adapters and cables](Adapters-And-Cables.md).

## Related software

- [sigrok supported hardware](https://sigrok.org/wiki/Supported_hardware)
- [PulseView](https://sigrok.org/wiki/PulseView)
- [fx2lafw firmware](https://sigrok.org/wiki/Fx2lafw)
