# ESD Mats

An ESD-controlled work area reduces the chance of damaging exposed semiconductors
with electrostatic discharge. A mat alone is not an ESD system: it needs a verified
dissipative surface, common-point ground, personnel grounding, and periodic checks.

Catalog guidance reviewed: **12 July 2026**.

## Budget sourcing

Shopee is the normal budget source in Southeast Asia for silicone or rubber ESD mats,
wrist straps, ground cords, common-point blocks, and basic testers.

- [Search Shopee Malaysia for ESD mat and wrist strap](https://shopee.com.my/search?keyword=esd%20mat%20wrist%20strap)
- [Search Shopee Malaysia for ESD common point ground](https://shopee.com.my/search?keyword=esd%20common%20point%20ground)
- [Search Shopee Malaysia for wrist strap tester](https://shopee.com.my/search?keyword=esd%20wrist%20strap%20tester)
- [Search Shopee Malaysia for replacement ESD wrist strap](https://shopee.com.my/search?keyword=esd%20wrist%20strap%201%20megaohm)

No seller is pinned because listings and quality change. Prefer a listing that states
surface resistance, resistance to ground, mat layers/material, dimensions, ground
hardware, and the wrist-cord resistor. Reject listings that only say "anti-static"
without measurable specifications.

### Preferred seller type

ESD mats and workstation accessories are commonly cheaper from factory-direct or
warehouse sellers on Shopee. Prefer established industrial/warehouse listings that
show the mat layers, resistance specification, grounding hardware, stocked dimensions,
and actual warehouse product photos. A large catalogue and high sales volume do not
replace electrical specifications or testing.

## Minimum workstation

- Static-dissipative bench mat large enough for the board and tools
- Common-point ground connection
- Adjustable wrist strap with a ground cord containing a 1 megohm safety resistor
- Known protective-earth connection installed according to local electrical practice
- ESD-safe containers or shielding bags for loose boards
- A method to verify continuity/resistance periodically

The ESD Association describes a typical workstation as a dissipative worksurface,
personnel grounding, and common-point ground. Its fundamentals guide gives a typical
worksurface resistance-to-ground range of `1 x 10^6` to `1 x 10^9` ohms and identifies
1 megohm as the common wrist-cord current-limiting resistance. See
[ESD control fundamentals](https://www.esda.org/esd-overview/esd-fundamentals/part-3-basic-esd-control-procedures-and-materials/).

## Buying checklist

1. Choose a two-layer dissipative mat when possible.
2. Verify the listed resistance range instead of relying on colour.
3. Confirm whether the ground cord and wrist strap are included.
4. Check snap size; common mat and strap hardware is not always interchangeable.
5. Prefer a wristband with conductive fabric contacting the skin around the wrist.
6. Confirm the ground lead includes resistance and is not a direct wire to earth.
7. Check recent customer photos for thin vinyl, damaged snaps, and missing cords.
8. Select dimensions that leave a clear, grounded work zone around the PCB.

## Setup and verification

1. Place the mat on a dry bench away from exposed mains wiring.
2. Connect mat, wrist strap, and ESD equipment to one common point.
3. Connect that point to a verified protective-earth arrangement.
4. Check mat-to-ground and wrist-cord resistance with suitable test equipment.
5. Wear the strap against skin while handling exposed ESD-sensitive assemblies.
6. Recheck after moving the bench, changing cords, or cleaning the mat.

Do not connect a wrist strap to an unknown socket pin, plumbing, neutral conductor,
or improvised mains wiring. Do not wear a grounded wrist strap while working on
energized high-voltage equipment. The ESD Association specifically warns against
wrist-strap use where personnel may be exposed to circuits at 250 V or higher.

## Care

- Use a cleaner approved for the mat material; residues can change resistance.
- Keep ordinary plastic, foam, tape, and packaging away from exposed boards.
- Replace cracked cords, loose snaps, worn wristbands, and curling mats.
- Test the wrist strap regularly; visual inspection cannot prove electrical continuity.
- Record checks if the bench is used for repeatable repair or production work.

Keep one tested spare wristband and one compatible 1 megohm cord. A spare is useful
only if its snap fits the workstation and it passes the same resistance/continuity
check as the primary strap. Replace intermittent cords rather than holding them at a
particular angle to make a tester pass.

## Related setup

- [Storage and maintenance](Storage-And-Maintenance.md)
- [Soldering equipment](Soldering.md)
- [Adapters and cables](Adapters-And-Cables.md)
