# Enclosure CAD Files

3D printed ABS enclosure — frame-on-PCB construction.

## Files (TODO — design in Fusion 360 Mechanical workspace)

- `block_a_frame.stl` — Electronics block frame (walls only, no floor)
- `block_a_lid.stl` — Electronics block lid with O-ring groove
- `block_b_frame.stl` — Battery block frame
- `block_b_lid.stl` — Battery block lid with ePTFE pressure port + BeCu spring pocket. Must include embossed "+" polarity marking / battery orientation diagram on the interior — there is no electrical reverse-battery protection, so the lid marking is the only guard against a flipped CR1225 (−3V on all ICs).
- `assembly.step` — Full assembly STEP file

## Specifications

- Material: ABS (FDM, 0.1mm layers, 100% infill)
- Wall thickness: 1.2mm minimum, ~5.5mm bosses at M1.4 insert locations
- O-ring groove: 1mm cross-section, 0.75mm deep. ⚠ When the CAD is done, verify groove centerline perimeter against the o-ring: a nominal "12mm OD" ring has ~34.5mm circumference — a rectangular groove around the block cavity will likely need a larger ring (target ≤5% install stretch). The BOM's assortment kit covers this; pick the size from the finished model, don't assume 12mm.
- Post-processing: Acetone vapor smoothing on O-ring surfaces
- Hardware: M1.4 DIN912 screws + brass heat-set inserts

See docs/implementation-plan.md Task 14 for full design steps.
