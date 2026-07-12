# Storage and Maintenance

Organised storage prevents ESD damage, mixed components, shorted batteries, lost
adapters, expired chemicals, and unrepeatable hardware experiments.

Catalog guidance reviewed: **12 July 2026**.

## Budget sourcing

- [Search Shopee Malaysia for electronic component organiser](https://shopee.com.my/search?keyword=electronic%20component%20organizer)
- [Search Shopee Malaysia for ESD shielding bags](https://shopee.com.my/search?keyword=esd%20shielding%20bag)
- [Search Shopee Malaysia for parts drawer cabinet](https://shopee.com.my/search?keyword=parts%20drawer%20cabinet)
- [Search Shopee Malaysia for cable organiser labels](https://shopee.com.my/search?keyword=cable%20organizer%20labels)
- [Search Shopee Malaysia for lithium battery storage bag](https://shopee.com.my/search?keyword=lithium%20battery%20storage%20bag)

Do not treat a marketplace "fireproof" battery bag as a certified fire enclosure.
Check the manufacturer's test evidence and follow battery/charger instructions.

### Preferred seller type

Drawer cabinets, compartment boxes, ESD bags, labels, cable ties, and bins are often
best-value from factory or warehouse sellers. Check external and internal dimensions,
plastic thickness, divider count, drawer stop design, stackability, and shipping
protection. Large cabinets with low listed prices can have costly shipping or arrive
cracked if the seller does not pack them adequately.

## Storage zones

| Zone | Contents | Preferred storage |
| --- | --- | --- |
| ESD-sensitive | Bare ICs, RAM, flash, modules, exposed boards | Shielding bags or ESD-safe boxes with labels |
| Passive parts | Resistors, capacitors, headers, fasteners | Compartment boxes or labelled drawers |
| Cables/adapters | USB, UART, JTAG, SWD, clips | Individually tied and labelled by connector/function |
| Tools | Hand tools, probes, spudgers | Foam, racks, drawers, or fitted cases |
| Chemicals | Flux, solvents, adhesives | Closed original containers, SDS-aware ventilated storage |
| Batteries | Cells, packs, power banks | Cool, dry, separated terminals, condition monitored |
| Evidence/targets | Challenge boards and dumps | Unique ID, sealed bag/box, hashes and notes |

## Maintenance and consumables inventory

The lab should maintain a small, labelled stock rather than discovering that a filter,
wick, lead, or wrist strap has failed halfway through a board repair.

| Item | Minimum spare stock | Reorder or replacement trigger | Source |
| --- | --- | --- | --- |
| HAKKO FA-400 filter | One unopened `A1001` five-filter set | Last set opened; filter loaded/damaged or performance declines | [Shopee A1001 search](https://shopee.com.my/search?keyword=hakko%20a1001%20filter) |
| FA-400-style filter | One verified pack matching exact unit/dimensions | Last pack opened or seller/model changes | [Shopee FA-400 filter search](https://shopee.com.my/search?keyword=fa-400%20replacement%20filter) |
| Electronics-cleaning IPA | One sealed bulk bottle plus small working bottle | Working bottle low, contaminated, unlabelled, or container damaged | [Shopee 99% IPA search](https://shopee.com.my/search?keyword=ipa%2099%20electronics%20cleaner) |
| IPA dispenser/wash bottle | One compatible labelled bottle | Pump leaks, bottle crazes, label fails, or liquid is contaminated | [Shopee IPA bottle search](https://shopee.com.my/search?keyword=esd%20ipa%20solvent%20bottle) |
| Desoldering wick | At least two widths, one unopened reel each | Oxidised, contaminated, or final reel opened | [Shopee wick search](https://shopee.com.my/search?keyword=desoldering%20wick) |
| Brass tip-cleaning wool | One refill | Compressed, contaminated, or shedding excessively | [Shopee brass wool search](https://shopee.com.my/search?keyword=soldering%20brass%20wool) |
| Soldering tips | One chisel plus spares for routinely used geometries | Plating damaged, poor wetting, deformation, or last spare installed | [Shopee tip search](https://shopee.com.my/search?keyword=replacement%20soldering%20tips) |
| Solder/flux | One sealed replacement of each approved type | Identity lost, expired/degraded, contaminated, or final pack opened | [MakerHub MY](https://shopee.com.my/makerhub) |
| Multimeter leads | One manufacturer-approved or correctly rated set | Insulation damage, loose shrouds, intermittent continuity, burnt probes | [Shopee lead search](https://shopee.com.my/search?keyword=multimeter%20test%20leads) |
| Multimeter fuses | Manufacturer-specified high-energy fuse where required | Fuse used; always replenish immediately | Meter manufacturer/distributor |
| Multimeter battery | One correct chemistry/size with checked date | Low-battery indication or leakage/age concern | Local verified battery seller |
| ESD wristband and cord | One tested compatible spare of each | Intermittent cord, worn band, failed tester, or spare placed in service | [Shopee wrist-strap search](https://shopee.com.my/search?keyword=esd%20wrist%20strap%201%20megaohm) |
| Probe hooks and clips | One spare set | Weak spring, broken insulation, corrosion, or unreliable contact | [Shopee test-hook search](https://shopee.com.my/search?keyword=electronics%20test%20hook%20clip) |
| Dupont leads | Separate M-M, M-F, and F-F packs | Loose contacts, damaged insulation, intermittent continuity | [MakerHub MY](https://shopee.com.my/makerhub) |

Quantities should scale with actual use. Record the approved part/model beside each
bin so a low-cost but incompatible substitute is not ordered accidentally.

## IPA handling and bottles

For flux and electronics cleaning, use IPA of known composition/purity compatible
with the assembly and contaminants. Higher-water formulations can dry more slowly and
may not suit moisture-sensitive cleaning. Test plastics, labels, coatings, displays,
adhesives, and elastomers before broad use.

- Keep bulk IPA in its original, tightly closed, clearly labelled container.
- Decant only a small working quantity into an IPA-compatible labelled wash or pump bottle.
- Never use drink bottles or an unlabelled spray container.
- Keep IPA away from irons, hot air, sparks, flames, and other ignition sources.
- Use ventilation, avoid skin/eye contact, and follow the supplier SDS.
- Allow the board to dry fully and remove vapour before applying power.
- Replace a dispenser that leaks, self-pumps, becomes brittle, or has a failed label.

Shopee examples exist for
[labelled IPA wash bottles](https://shopee.com.my/IPA-Bottle-500ml-Capacity-Labelled-Wash-Bottle-for-Isopropanol-Self-Venting-Low-Density-Polyethylene-i.767985379.19412252087)
and [ESD solvent dispensers](https://shopee.com.my/ESD-IPA-Solvent-Dispenser-ESD-IPA-Solvent-Bottle-ESD-Alcohol-Bottle-100ml-200ml-i.64875274.8789400617),
but compatibility and ESD claims must still be verified from material/specification data.

Pink anti-static bags reduce charge generation but are not the same as static-shielding
bags. Store sensitive loose boards in appropriate closed shielding packaging.

## Labelling standard

Each board or target should have:

- Unique asset/project identifier
- Exact model and hardware revision
- Known-good or unknown/faulty state
- Logic and supply voltage
- Firmware/build identifier when known
- Date acquired and source
- Storage location
- Link to notes, photos, dumps, and hashes

Label cable voltage and pinout adapters. Never store an unlabelled custom cable that
could place power onto a signal pin.

## Battery storage

Lithium batteries can present fire and explosion hazards when damaged, defective,
improperly charged, or exposed to unsuitable temperatures. OSHA recommends cool, dry
storage, following manufacturer instructions, limiting stored quantities, and using
designated recycling facilities. See
[OSHA lithium battery guidance](https://www.osha.gov/publications/bytopic/battery-manufacturing).

- Insulate exposed terminals and prevent metal objects from bridging them.
- Separate swollen, punctured, hot, leaking, or recalled packs from normal stock.
- Do not charge unattended or with an incompatible charger.
- Do not store loose cells in pockets, toolboxes, or mixed component drawers.
- Use local approved collection/recycling routes; do not place lithium batteries in
  ordinary rubbish.

## Maintenance schedule

### Before each session

- Inspect mains cables, USB leads, probes, and iron condition.
- Check extraction airflow and working-area clearance.
- Verify ESD connections when handling sensitive assemblies.
- Confirm test-equipment leads are in the correct sockets and ranges.
- Check that IPA and flux containers are closed, labelled, and away from heat.
- Confirm the correct filter, wick, tips, and leads are available before disassembly.

### Monthly

- Test wrist straps and mat grounding with suitable equipment.
- Inspect fume filters and clean extractor inlets.
- Check battery condition and remove damaged cells.
- Inventory high-use consumables and replacement tips.
- Back up project notes, firmware dumps, photos, and checksums.

### Periodically

- Calibrate or compare measurement equipment against a trusted reference.
- Review chemical expiry, labels, SDS, and container condition.
- Exercise connectors and inspect clips for corrosion or poor spring pressure.
- Archive completed targets and securely erase reusable storage when appropriate.
- Review marketplace tools for recalls or safety notices.

## Data and evidence

Keep at least two verified copies of original firmware dumps. Use read-only copies for
analysis, hash every acquisition, and record tool versions, voltage, pinout, and read
commands. Do not overwrite the only known-good image while experimenting.

## Related setup

- [Cleaning and chemicals](Cleaning-And-Chemicals.md)
- [ESD mats](ESD-Mats.md)
- [Adapters and cables](Adapters-And-Cables.md)
- [Soldering equipment](Soldering.md)
