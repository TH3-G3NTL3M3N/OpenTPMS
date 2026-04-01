# OpenTPMS — Open-Source Bicycle Tire Pressure Monitoring Sensor

**Date:** 2026-04-01
**Status:** Design Approved
**Author:** simruelland + Claude

## Overview

An open-source, in-tire bicycle TPMS sensor with replaceable CR1225 battery, dual ANT+/BLE wireless, and temperature-compensated flat detection. Designed for small-batch production and community replication.

### Key Differentiators vs Outrider TL Pro/Mini

- **Replaceable battery** — user swaps CR1225 coin cell every 2-3 years during sealant refresh. No potted-forever design.
- **-30°C to +60°C operating range** — supercapacitor buffer enables fat bike winter riding.
- **Dual ANT+ and BLE** — native Garmin/Wahoo support AND phone connectivity.
- **Temperature-compensated flat alerts** — eliminates false alarms from weather changes.
- **Open-source hardware and firmware** — community can build, modify, and improve.
- **One universal size** — fits 19-30mm internal width rims.

---

## 1. System Architecture

### Physical Layout

Two-block symmetric design. PCB strip spans across the rim width (~40mm) with the valve stem passing through a center hole. One enclosure block on each side of the valve.

```
        Valve stem
            │
 ┌──────────┼──────────┐
 │ Block A  ○  Block B │
 │ (elec)   │  (batt)  │
 └──────────┼──────────┘
            │
         Rim bed
```

- **Block A (Electronics):** Raytac MDBT42Q nRF52 module, LIS2DH12 accelerometer, supercapacitor, passives. Not user-serviceable (openable for advanced repair/reflash).
- **Block B (Battery + Pressure):** CR1225 in spring-clip holder, Honeywell ABP pressure sensor with ePTFE membrane in lid. User-serviceable for battery swap.
- **PCB strip:** Rigid 2-layer FR4, 0.8mm thick, conformal coated where exposed between blocks.
- **Mounting:** Valve stem passes through center hole in PCB. Rim seal sits on top of PCB strip.

### Dimensions

- **Total length:** ~40mm (across rim, along circumference)
- **Width:** ≤16mm (fits 19mm+ internal width rims)
- **Height Block A:** ~5.2mm
- **Height Block B:** ~5.5mm
- **Weight:** ~6.5g per sensor

### Data Flow

1. Wheel spins → accelerometer detects motion → MCU wakes from deep sleep
2. Read pressure sensor + temperature
3. Temperature-compensated flat detection logic evaluates readings
4. Transmit on ANT+ and BLE (every 3 seconds)
5. If pressure drop exceeds thermal explanation → trigger flat alert
6. MCU returns to sleep until next TX interval
7. 5 minutes no motion → full deep sleep (~3µA)

---

## 2. Component Selection & BOM

### Primary Components

| Component | Part | Key Specs | Package | Est. Cost |
|-----------|------|-----------|---------|-----------|
| MCU + Radio | Raytac MDBT42Q-512KV2 | nRF52832, BLE 5.0 + ANT+, chip antenna, pre-certified FCC/CE/IC | 10.5 x 15.5 x 2.2mm | $3.50 |
| Pressure Sensor | Honeywell ABP series (150 PSI variant) | 0-150 PSI gauge, ±0.25% accuracy, I2C, built-in temp, -40 to +85°C | 5 x 5 x 3.3mm | $7.00 |
| Accelerometer | ST LIS2DH12 | 3-axis, motion-detect IRQ wake, 1.8µA low-power, I2C, -40 to +85°C | 2 x 2 x 1mm LGA-12 | $0.80 |
| Supercapacitor | AVX BestCap or similar | 47-100mF, 3.5V, low ESR, -40 to +85°C | ~5 x 5 x 1.5mm | $0.40 |
| Battery | Panasonic CR1225 (industrial/extended temp) | 3V, 48mAh, -30 to +70°C | 12.5mm dia x 2.5mm | $0.50 |
| Battery Holder | Linx BAT-HLD-012-SMT or similar | CR1225 spring-clip, SMD, vibration-resistant | ~14 x 14 x 3mm | $0.30 |
| ePTFE Membrane | Porex PMV10 or generic ePTFE disc | Hydrophobic, air-permeable, sealant-resistant | ~4mm disc | $0.15 |
| Screws | M1.2 x 3mm stainless steel Allen | 2 per block, 4 total per sensor | 1.2mm head | $0.20 |
| O-Rings | Silicone, custom size | Chemical-resistant, -60 to +200°C, 2 per sensor | ~12 x 1mm cross-section | $0.10 |
| Passives | Various 0402 caps, resistors | Decoupling, pull-ups, filtering, ~10-15 components | 0402 | $0.30 |
| PCB | 2-layer FR4, 0.8mm, ENIG finish | ~16 x 40mm strip with valve hole, conformal coated | — | $1.00 |
| Enclosure | 3D printed ASA (FDM), frame-on-PCB | 2 frames + 2 lids, brass heat-set M1.2 inserts | ~15 x 16mm each | $2.50 |
| Conformal Coating | Silicone spray | Applied to exposed PCB strip between blocks | — | $0.10 |

### BOM Summary Per Sensor

- **Electronics:** ~$12.50 (MCU + sensors + passives + supercap)
- **Mechanical:** ~$4.35 (enclosure + PCB + screws + O-rings + membrane)
- **Total per sensor:** ~$16.85
- **Total per pair:** ~$33.70 (before assembly labor)

### Weight Breakdown Per Sensor

- MDBT42Q module: ~0.5g
- Pressure sensor: ~0.3g
- Accelerometer + passives + supercap: ~0.5g
- CR1225 battery: ~0.9g
- PCB strip (FR4): ~0.8g
- Enclosures (2x ASA frames + lids): ~2.5g
- Screws (4x M1.2 stainless) + brass inserts: ~0.8g
- O-rings (2x silicone): ~0.2g
- **Total: ~6.5g**

---

## 3. Firmware Architecture

### Software Stack

- **Hardware:** nRF52832 ARM Cortex-M4F (via MDBT42Q module)
- **Protocol stack:** Nordic SoftDevice S332 (concurrent BLE + ANT+)
- **SDK:** Nordic nRF5 SDK 17.1 (or nRF Connect SDK / Zephyr RTOS)
- **Language:** C
- **IDE:** VS Code + nRF Connect extension
- **Debugger:** Segger J-Link or nRF52-DK as programmer

### State Machine

```
DEEP_SLEEP (~3µA) ──[motion detected]──► WAKING (read sensors, init radio)
       ▲                                        │
       │                                        ▼
       │                                  ACTIVE (TX every 3s)
       │                                        │
       └──────[5 min no motion]──── SLEEPING (store last reading)
```

### Firmware Modules

1. **Pressure Manager:** Reads Honeywell ABP via I2C. Returns calibrated pressure (PSI) and temperature (°C). Handles sensor initialization and error recovery.

2. **Motion Manager:** Configures LIS2DH12 motion-detect interrupt. Wakes MCU on wheel rotation. Reports motion/no-motion state. Manages 5-minute inactivity timeout.

3. **Alert Engine:** Implements temperature-compensated flat detection using ideal gas law (P1/T1 = P2/T2). Compares current pressure against expected pressure at current temperature based on baseline. Triggers alert only when pressure drop exceeds thermal explanation. User-configurable threshold stored in flash.

4. **ANT+ Tire Pressure Profile:** Broadcasts standard tire pressure data page every 3 seconds. Fields: pressure (0.01 PSI resolution), temperature, sensor location (front/rear), battery status. Requires ANT+ adopter license (~$100/year).

5. **BLE Custom GATT Service:** Characteristics:
   - Pressure (Read, Notify) — uint16, 0.01 PSI
   - Temperature (Read, Notify) — int16, 0.01°C
   - Battery Level (Read, Notify) — standard BLE Battery Service, uint8 0-100%
   - Sensor Location (Read, Write) — 0x01=Front, 0x02=Rear, stored in flash
   - Alert Threshold (Read, Write) — uint16, 0.1 PSI, stored in flash
   - TX Interval (Read, Write) — uint8, 1-10 seconds, default 3, stored in flash
   - Baseline P/T (Read, Write) — reference pressure + temp for compensation

6. **OTA DFU:** Nordic secure DFU bootloader. Signed firmware images. Update via nRF Connect app on phone. Bootloader survives application firmware crashes.

7. **Non-Volatile Storage:** Uses Nordic FDS (Flash Data Storage) for: sensor location, alert threshold, TX interval, baseline P/T, BLE bonding keys. Persists across battery swaps.

### Power Budget

| State | Current | Duration | Notes |
|-------|---------|----------|-------|
| Deep sleep | ~3µA | 99% of time when not riding | MCU System OFF + accel motion-detect |
| Waking | ~5mA | ~2ms | Sensor read + radio init |
| Active TX (ANT+) | ~8mA | ~2ms per burst | Every 3 seconds |
| Active TX (BLE) | ~12mA | ~3ms per burst | Every 3 seconds |
| Active sleep between TX | ~5µA | ~2.99s | MCU idle, timers running |

**Estimated battery life:** 2-3 years at 1 hour/day riding with CR1225 (48mAh).

---

## 4. Enclosure & Mechanical Design

### Frame-on-PCB Construction

The enclosure is a **frame** (walls only, no floor) bonded permanently to the PCB with RTV silicone adhesive. The PCB serves as the structural bottom. A removable lid screws down from the top with an O-ring seal.

```
Cross-section (side view):

    Screw    ╔══╗              ╔══╗    Screw
    M1.2     ║  ║  ┌────────┐  ║  ║    M1.2
             ║  ║  │  LID   │  ║  ║
             ║  ║  │ (ASA)  │  ║  ║
             ║  ║  ══════════  ║  ║    ← O-ring
             ║  ║              ║  ║
             ║  ║  components  ║  ║    ← FRAME (ASA walls, no floor)
             ║  ║  or battery  ║  ║
             ║  ║              ║  ║
    Brass    ╚══╝──────────────╚══╝
    inserts        PCB (floor)         ← PCB is the bottom surface
             ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
                   RIM BED
```

### Height Stack-Up

**Block A (Electronics):**

| Layer | Thickness |
|-------|-----------|
| PCB (is the floor) | 0.8mm |
| Tallest component (MDBT42Q = 2.2mm) | 2.2mm |
| Air gap above (antenna cavity) | 0.5mm |
| O-ring (compressed) | 0.5mm |
| Lid (ASA, 1.2mm) | 1.2mm |
| **Total** | **5.2mm** |

**Block B (Battery + Pressure Sensor):**

| Layer | Thickness |
|-------|-----------|
| PCB (is the floor) | 0.8mm |
| Tallest component (pressure sensor 3.3mm or battery holder+CR1225 3.0mm) | 3.3mm |
| O-ring (compressed) | 0.5mm |
| Lid (ASA, 1.2mm) | 1.2mm |
| **Total** | **5.8mm** |

Note: The Honeywell ABP pressure sensor (3.3mm) is the height-limiting component in Block B, not the battery. If 5.8mm is too tight for insert compatibility, a thinner pressure transducer (e.g., TE Connectivity MS5837, 3.3mm, or Bosch BMP388, 2.0mm at ±1% accuracy) can be substituted to bring height down to 5.5mm.

### 3D Printing Specifications

- **Material:** ASA (Acrylonitrile Styrene Acrylate) — UV and chemical resistant
- **Method:** FDM
- **Layer height:** 0.1mm (fine detail for O-ring groove)
- **Infill:** 100% (structural, waterproof walls)
- **Minimum wall thickness:** 1.2mm (all walls)
- **Post-processing:** Acetone vapor smoothing on O-ring groove surfaces for gas-tight seal
- **Scale-up path:** Injection molding in PA66 or ABS when volume exceeds ~500 units

### Sealing System

1. **Frame-to-PCB:** Permanent bond with RTV silicone adhesive. Flexible (survives vibration), chemically inert to tire sealant. PCB surface lightly roughened in bonding area for adhesion.
2. **Lid-to-frame:** Silicone O-ring (1mm cross-section) in rectangular groove in frame top rim. Lid compresses O-ring ~25% when screwed down.
3. **Screw retention:** 2x M1.2 stainless Allen screws per block into brass heat-set inserts. Loctite 222 threadlocker. Silicone grease dab on hex sockets to prevent sealant curing inside.
4. **Pressure port:** 2mm hole in Block B lid with ePTFE membrane bonded on inner surface. Air-permeable, liquid-proof. Inspectable during battery swap.
5. **Exposed PCB strip:** 2-3 coats silicone conformal coating between blocks. No exposed copper.

### Tire Insert Compatibility

| Insert | Height Clearance | Compatibility |
|--------|-----------------|---------------|
| Tannus Tubeless (Pro/Fusion) | Open valve channel | Compatible |
| Vittoria Air-Liner | Valve channel design | Compatible |
| CushCore XC | 6-8mm base at valve | Compatible |
| CushCore Trail | 6-8mm base at valve | Compatible |
| CushCore Pro | 8-10mm base, tight valve area | Verify during prototyping |

---

## 5. Communication Protocols & Compatibility

### ANT+ Tire Pressure Profile

Standard ANT+ broadcast. Native support on any ANT+ head unit with tire pressure capability. No app or data field required.

- Pressure: 0.01 PSI resolution
- Temperature: 0.01°C resolution
- Sensor location: Front/Rear
- Battery status: OK/Low/Critical
- Broadcast interval: 3 seconds (configurable 1-10s)
- **License:** ANT+ adopter license required (~$100/year for small volume)

### BLE Custom GATT Service

128-bit custom UUIDs. Supports read, write, and notify for configuration and live data. Used by Web Bluetooth config tool and future native app.

### v1 Configuration: Web Bluetooth

- Web page hosted on GitHub Pages, opened in Chrome
- Scans for and connects to sensor via Web Bluetooth API
- Displays: current pressure, temperature, battery level
- Configures: sensor location (F/R), alert threshold, TX interval, baseline P/T
- **iOS limitation:** Safari doesn't support Web Bluetooth. Use Bluefy browser (free) as workaround until native app is built.

### OTA Firmware Updates

- Nordic secure DFU bootloader
- Update via nRF Connect app (iOS/Android)
- Signed firmware images prevent unauthorized modification
- User must pull tire to get BLE connection (sensor inside tire has limited range when stationary)

### Device Compatibility

| Device | Live Pressure | Flat Alerts | Config | OTA | Protocol |
|--------|:---:|:---:|:---:|:---:|----------|
| Garmin Edge 530+ | ✓ | ✓ | — | — | ANT+ native |
| Garmin Fenix/Forerunner | ✓ | ✓ | — | — | ANT+ native |
| Wahoo Elemnt/Bolt/Roam | ✓ | ? | — | — | ANT+ (if TPMS supported) |
| Hammerhead Karoo 2/3 | ✓ | ✓ | — | — | ANT+ or BLE extension |
| Android Phone (Chrome) | ✓ | ✓ | ✓ | ✓ | BLE + Web Bluetooth |
| iPhone (Bluefy browser) | ✓ | ✓ | ✓ | — | BLE + Web BLE workaround |
| iPhone (nRF Connect) | — | — | — | ✓ | BLE DFU only |

---

## 6. Testing & Validation

### Test Phases

1. **Bench Testing:** Firmware flash, sensor accuracy vs reference gauge, BLE/ANT+ pairing, power consumption, sleep/wake cycle, OTA DFU, alert logic.
2. **In-Tire Testing:** Seal integrity (submersion test), sealant exposure, pressure hold over days, insert compatibility, RF range through tire/rim, battery swap procedure.
3. **Field Testing:** Multi-hour rides, temperature swings, vibration/impacts, flat detection real-world, battery drain rate, multi-bike pairing, head unit compatibility, cold weather (-30°C).

### Critical Test Procedures & Pass Criteria

1. **Pressure Accuracy:** ±0.5% of calibrated reference at 10 PSI increments (0-125 PSI). Repeat at -30°C, 20°C, 60°C.
2. **Seal Integrity:** Zero moisture ingress after 30 min at 1m depth (IP67). No sealant inside enclosure after 100km ride with sealant.
3. **RF Range:** Zero dropouts over 1-hour ride on aluminum and carbon rims. Connection maintained to ≥3m line-of-sight.
4. **Battery Life:** Deep sleep <5µA, average riding current <20µA, calculated life >2 years at 1hr/day.
5. **Vibration & Shock:** No damage after 20+ hours aggressive MTB trails. Pressure accuracy unchanged.
6. **Flat Detection:** Real leak detected within 30 seconds. Zero false alerts from temperature changes across 3 test rides.
7. **Cold Weather:** Successful wake, transmission, and accurate reading within 10 seconds at -30°C.
8. **Insert Compatibility:** Tire seats normally, sensor undamaged, readings accurate for each tested insert.

### Required Test Equipment

- Segger J-Link or nRF52-DK (~$60-300)
- Nordic Power Profiler Kit II (~$80)
- Calibrated digital pressure gauge (~$30-50)
- Garmin Edge 530+ or similar ANT+ head unit (~$200 used)
- Android phone with Chrome
- Freezer reaching -30°C (deep freezer or dry ice box)
- MTB + road bike with tubeless rims

---

## 7. Manufacturing & Assembly

### Per-Sensor Assembly Steps

1. **PCB fabrication:** Order from JLCPCB or PCBWay (2-layer, 0.8mm FR4, ENIG, panel of 10+)
2. **SMT assembly:** Solder paste stencil + reflow oven (or order assembled from JLCPCB SMT service). Components: MDBT42Q, LIS2DH12, ABP pressure sensor, supercap, passives.
3. **Conformal coat:** Spray silicone conformal coating on exposed PCB strip (valve area), 2-3 coats.
4. **3D print enclosures:** FDM ASA, 0.1mm layers, 100% infill. 2 frames + 2 lids per sensor. Acetone vapor smooth O-ring surfaces.
5. **Heat-set inserts:** Press M1.2 brass inserts into frame walls using soldering iron tip.
6. **Bond frames to PCB:** Apply RTV silicone to frame bottom, position on PCB, cure 24 hours.
7. **Install ePTFE membrane:** Bond ePTFE disc over pressure port hole in Block B lid with medical-grade adhesive.
8. **Install O-rings:** Place silicone O-rings in frame grooves.
9. **Insert battery:** Place CR1225 in Block B spring-clip holder.
10. **Close lids:** Screw down both lids with M1.2 Allen screws + Loctite 222. Dab silicone grease on hex sockets.
11. **Flash firmware:** Connect J-Link to SWD pads on PCB strip. Flash bootloader + application firmware. Assign serial number and default F/R designation.
12. **QC test:** Verify pressure reading, BLE/ANT+ broadcast, motion wake, seal check (visual).

### Scale-Up Path

- **1-100 units:** Hand assembly, 3D printed enclosures, JLCPCB SMT assembly
- **100-500 units:** Same process, batch 3D printing, streamlined assembly jig
- **500+ units:** Transition to injection-molded enclosures (PA66/ABS), full SMT assembly line, automated testing fixture

---

## 8. Open-Source Deliverables

- **Hardware:** KiCad PCB design files (schematic + layout), BOM, Gerber files
- **Firmware:** nRF5 SDK project (C), SoftDevice configuration, build scripts
- **Enclosure:** STEP/STL files for 3D printing, assembly drawings
- **Web Config Tool:** HTML/JS Web Bluetooth configuration page
- **Documentation:** Assembly guide, user manual, testing procedures, insert compatibility results
- **License:** CERN Open Hardware License v2 (hardware), MIT (firmware/software)
