# Enclosure CAD Files

3D printed ABS enclosure — frame-on-PCB construction.

## Files (TODO — design in Fusion 360 Mechanical workspace)

- `block_a_frame.stl` — Electronics block frame (walls only, no floor)
- `block_a_lid.stl` — Electronics block lid with O-ring groove
- `block_b_frame.stl` — Battery block frame
- `block_b_lid.stl` — Battery block lid with ePTFE pressure port + BeCu spring pocket
- `assembly.step` — Full assembly STEP file

## Specifications

- Material: ABS (FDM, 0.1mm layers, 100% infill)
- Wall thickness: 1.2mm minimum, ~5.5mm bosses at M1.4 insert locations
- O-ring groove: 1mm cross-section, 0.75mm deep
- Post-processing: Acetone vapor smoothing on O-ring surfaces
- Hardware: M1.4 DIN912 screws + brass heat-set inserts

See docs/implementation-plan.md Task 14 for full design steps.
