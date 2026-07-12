# Multimeters and Electrical Measurement

A multimeter is a core embedded-workbench tool for checking supply voltage, ground,
continuity, resistance, current, and component faults. Meter safety depends on the
instrument, leads, internal fuses, measurement category, and how it is used.

Catalog guidance reviewed: **12 July 2026**.

## Budget sourcing

- [Search Shopee Malaysia for digital multimeter](https://shopee.com.my/search?keyword=digital%20multimeter)
- [Search Shopee Malaysia for multimeter test leads](https://shopee.com.my/search?keyword=multimeter%20test%20leads)
- [Search Shopee Malaysia for multimeter fuse](https://shopee.com.my/search?keyword=multimeter%20ceramic%20fuse)
- [Fluke Official Store on Shopee](https://shopee.com.my/fluke.os)

A cheap meter can be useful for battery-powered, extra-low-voltage embedded work when
its limitations are explicit. Do not use an unknown or falsely rated meter on mains,
distribution panels, high-energy batteries, inverters, or industrial equipment.

## Minimum embedded-workbench features

- DC voltage and resistance ranges appropriate for the projects
- Fast audible continuity test
- Fused current input with the fuse type documented
- Clearly separated and labelled input jacks
- Replaceable test leads with shrouded plugs
- Low-battery indication and accessible battery compartment
- Capacitance, frequency, diode test, and temperature only where genuinely needed

## Safety ratings

For any measurement beyond isolated low-voltage electronics, select a meter and leads
with an appropriate IEC 61010 measurement-category and voltage rating, backed by an
independent certification mark. A printed `CAT III` label alone is not proof that a
marketplace meter was tested.

Fluke's [multimeter safety guide](https://www.fluke.com/en-us/learn/blog/safety/multimeter-guide)
explains CAT II, CAT III, and CAT IV environments. The leads and accessories must have
a rating equal to or greater than the meter for the measurement environment.

## Before every use

1. Inspect the case, display, input jacks, leads, insulation, and probe guards.
2. Confirm the black lead is in `COM` and the red lead is in the correct jack.
3. Select the expected function/range before touching the circuit.
4. Verify the meter on a known source before and after a safety-critical voltage test.
5. Connect common first and disconnect the live lead first where applicable.
6. Never measure voltage with the red lead left in a current input.

## Replacement leads and fuses

Do not replace a high-energy fuse with a glass fuse, wire, foil, or a cheaper fuse of
the same current value. Use the exact manufacturer-approved type, voltage, interrupt
rating, dimensions, and speed. Fluke notes that inadequate replacement fuses can fail
to contain fault energy.

Replacement leads must match the connector, insulation, current rating, and CAT/voltage
rating. Inspect and continuity-test them before use. See Fluke's
[test-lead inspection guidance](https://www.fluke.com/en-gb/learn/blog/digital-multimeters/testing-your-test-leads).

Keep one approved spare lead set, the specified fuses, and the correct battery. Label
them with the meter model so they are not consumed by another instrument.

## Maintenance

- Remove leaking batteries immediately using appropriate precautions.
- Clean only as specified by the manufacturer; contamination can affect insulation.
- Do not use a meter with a cracked case, damaged jack, intermittent lead, or abnormal reading.
- Compare low-voltage readings periodically against a trusted reference.
- Follow the manufacturer calibration interval when measurements must be traceable.
- Store leads loosely coiled and protect probe tips.
- Record meter model, serial number, fuse type, lead model, and last check date.

## Related setup

- [Storage and maintenance](Storage-And-Maintenance.md)
- [Adapters and cables](Adapters-And-Cables.md)
- [ESD mats](ESD-Mats.md)
