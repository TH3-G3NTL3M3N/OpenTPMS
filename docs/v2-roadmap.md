# OpenTPMS v2 Roadmap

Notes for future revision based on competitive analysis (Outrider TL Pro teardown).

## Goal: 12mm wide x 53mm long strip (matches Outrider form factor)

### Hardware Changes

1. **Bare nRF52832 QFN (6x6mm) instead of MDBT42Q module (10.5x15.5mm)**
   - Saves ~4.5mm width — critical for hitting 12mm board width
   - Requires designing PCB trace antenna, 32MHz crystal, 32.768kHz crystal, RF matching network
   - Requires FCC/CE/IC re-certification (~$5-10k for small volume)
   - Lose pre-certification advantage of the module

2. **1.6mm PCB thickness (up from 0.8mm)**
   - Structural rigidity for longer, narrower strip
   - Outrider uses 1.6mm at 53mm length — proven to work
   - Still 2-layer FR4 ENIG

3. **Linear layout — single strip instead of two blocks**
   - Components on one end, battery on the other, valve hole in center
   - Target: 12mm x 53mm x 1.6mm (PCB only), 5.5mm at battery end
   - Valve hole: Ø6.70mm (Outrider standard)
   - Eliminates separate enclosure blocks

4. **CR1632 battery (up from CR1225)**
   - 140mAh vs 48mAh — 3x capacity
   - 16mm diameter — overhangs 12mm PCB (potted over)
   - 3.2mm tall — fits in 5.5mm total height (1.6mm PCB + 3.2mm battery + 0.7mm clip)
   - Battery clip: use stamped retainer similar to Outrider design

5. **Epoxy potting instead of 3D printed enclosure**
   - Aerospace-grade epoxy with chemically resistant membrane over pressure sensor
   - Eliminates: ABS frames, lids, O-rings, screws, heat-set inserts
   - Better sealing (monolithic block vs multi-part assembly)
   - Trade-off: battery not replaceable (potted permanently)

6. **Optimized antenna placement**
   - PCB trace antenna on the thin end (1.6mm section, away from battery)
   - Ground pour keep-out zone designed into the linear layout
   - No module antenna constraints

### Firmware Changes

7. **ANT+ only option (drop concurrent BLE for power savings)**
   - Outrider TL Pro is ANT+ only — achieves 5+ year battery life
   - Could offer two firmware variants: ANT+ only (max battery) or ANT+/BLE (more compatible)

8. **NFC pairing (already hardware-ready from v1)**
   - Enable NFC in firmware for tap-to-pair setup before potting

### What Stays the Same

- MS5837-30BA pressure sensor (or equivalent) with factory calibration
- LIS2DH12 accelerometer
- Temperature-compensated flat detection algorithm
- DC-DC buck converter
- WDT, SAADC battery monitor, RESETREAS diagnostics
- Garmin Connect IQ data field
- Web Bluetooth configuration tool (if BLE enabled)

### Cost Estimate (v2 at volume)

| Component | v1 Cost | v2 Cost | Notes |
|---|---|---|---|
| MCU | $14.33 (module) | ~$3.50 (bare QFN) | Biggest savings |
| PCB | ~$1.00 | ~$1.50 | Thicker, longer |
| Enclosure | ~$2.50 | ~$0.50 | Epoxy potting vs 3D print |
| Battery | CR1225 $0.50 | CR1632 $0.80 | Larger battery |
| Screws/inserts/O-rings | ~$0.60 | $0.00 | Eliminated |
| **Total per sensor** | **~$44** | **~$20-25** | At small qty |

### Prerequisites Before v2

- [ ] v1 prototype tested and validated in real-world riding
- [ ] Firmware stable and feature-complete
- [ ] RF design expertise (or hire consultant for PCB antenna design)
- [ ] FCC/CE certification budget allocated
- [ ] Epoxy potting process validated (seal integrity, sealant resistance)
- [ ] Battery life testing confirms CR1632 lasts 3+ years

### Timeline

v2 design should not begin until v1 has been field-tested for at least 3-6 months. The firmware, calibration process, and alert algorithm need real-world validation before committing to a non-repairable potted design.
