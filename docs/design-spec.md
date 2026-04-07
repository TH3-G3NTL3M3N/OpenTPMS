# OpenTPMS — Open-Source Bicycle Tire Pressure Monitoring Sensor

**Date:** 2026-04-01 (updated 2026-04-03)
**Status:** Design Approved (Rev 8 — no commercial battery holder, PCB pad + lid spring contact, Block B back to 5.5mm, 16mm board width preserved)
**Author:** simruelland + Claude

## Overview

An open-source, in-tire bicycle TPMS sensor with replaceable CR1225 battery, dual ANT+/BLE wireless, and temperature-compensated flat detection. Designed for small-batch production and community replication.

### Key Differentiators vs Outrider TL Pro/Mini

- **Replaceable battery** — user swaps CR1225 coin cell every 2-3 years during sealant refresh. No potted-forever design.
- **-30°C to +60°C operating range** — ceramic buffer capacitor enables reliable TX in cold weather. Note: MS5837-30BA pressure sensor is rated -20°C to +85°C; accuracy below -20°C is unguaranteed but sensor will function. Test during prototyping.
- **Dual ANT+ and BLE** — native Garmin/Wahoo support AND phone connectivity.
- **±1% accuracy at 20 PSI** — factory-calibrated MS5837-30BA sensor + Garmin/phone barometer for atmospheric. No user calibration. Plug and play.
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

- **Block A (Electronics):** Raytac MDBT42Q nRF52 module, LIS2DH12 accelerometer, 220µF ceramic buffer cap, DC-DC inductor, passives. Not user-serviceable (openable for advanced repair/reflash).
- **Block B (Battery + Pressure):** CR1225 sits negative-side-down on PCB copper pad (ENIG). Positive contact via BeCu leaf spring in lid, wired to PCB VCC pad. TE Connectivity MS5837-30BA pressure sensor (absolute, gel-filled) with ePTFE membrane in lid. User-serviceable for battery swap.
- **PCB strip:** Rigid 2-layer FR4, 0.8mm thick, conformal coated where exposed between blocks.
- **Mounting:** Valve stem passes through center hole in PCB. Rim seal sits on top of PCB strip.

### Dimensions

- **Total length:** ~42mm (across rim, along circumference)
- **Width:** ≤16mm (fits 19mm+ internal width rims)
- **Height Block A:** ~5.2mm (lid surface), ~6.6mm (incl. screw heads into tire cavity)
- **Height Block B:** ~5.5mm (lid surface), ~6.9mm (incl. screw heads into tire cavity)
- **Weight:** ~6.5g per sensor

### Pressure Measurement Architecture

The sensor measures **absolute pressure** (relative to vacuum). Conversion to gauge PSI happens on the receiving device, not on the sensor:

- **Garmin head units:** Custom Connect IQ data field reads `Activity.Info.rawAmbientPressure` from Garmin's built-in barometer and subtracts it from the sensor's absolute reading. Plug and play — no phone or calibration needed.
- **Phones:** Web Bluetooth app reads the phone's barometer via Web Sensor API and subtracts atmospheric. Same result.
- **Factory calibration:** Each sensor is calibrated at 3-5 known pressures during assembly. Correction polynomial stored in nRF52 flash. Removes systematic sensor error, leaving ±5-10 mbar residual.

**Accuracy budget at 20 PSI:**

| Error Source | Contribution |
|---|---|
| MS5837-30BA after factory cal | ±5-10 mbar (±0.07-0.15 PSI) |
| Garmin barometer | ±1 mbar (±0.015 PSI) |
| **Combined** | **±0.09-0.17 PSI (±0.4-0.8% at 20 PSI)** |

### Data Flow

1. Wheel spins → accelerometer detects motion → MCU wakes from deep sleep
2. Read pressure sensor (absolute) + temperature
3. Apply factory calibration polynomial to raw reading
4. Temperature-compensated flat detection logic evaluates readings (uses absolute pressure changes — no atmospheric needed for flat detection)
5. Transmit **absolute pressure + temperature** on ANT+ and BLE (every 3 seconds)
6. Receiving device (Garmin/phone) subtracts atmospheric to display gauge PSI
7. If pressure drop exceeds thermal explanation → trigger flat alert
8. MCU returns to sleep until next TX interval
9. 5 minutes no motion → full deep sleep (~3µA)

---

## 2. Component Selection & BOM

### Primary Components

| Component | Part | Key Specs | Package | Est. Cost |
|-----------|------|-----------|---------|-----------|
| MCU + Radio | Raytac MDBT42Q-512KV2 | nRF52832, BLE 5.0 + ANT+ + NFC-A (hardware ready), chip antenna, pre-certified FCC/CE/IC | 10.5 x 15.5 x 2.2mm | $3.50 |
| Pressure Sensor | TE Connectivity MS5837-30BA (MS583730BA01-50) | 0-30 bar absolute (435 PSI), ±50 mbar raw (±0.15 PSI after factory cal), I2C addr 0x76, built-in 24-bit temp, -20 to +85°C, gel-filled | 3.3 x 3.3 x 2.75mm | $11.09 |
| Accelerometer | ST LIS2DH12 | 3-axis, motion-detect IRQ wake, 1.8µA low-power, I2C, -40 to +85°C | 2 x 2 x 1mm LGA-12 | $0.80 |
| TX Buffer Cap | Murata GRM31CR60J227ME11L | 220µF, 6.3V, X5R ceramic, 1206. ~143µF effective after DC bias derating at 3V. Buffers 12mA TX bursts in cold weather. | 3.2 x 1.6 x 1.6mm | $0.20 |
| Battery | Panasonic CR1225 (industrial/extended temp) | 3V, 48mAh, -30 to +70°C | 12.5mm dia x 2.5mm | $0.50 |
| Battery Contact | Custom PCB pad + lid spring | Negative: ~10mm circular copper pad on PCB (ENIG finish). Positive: BeCu leaf spring inside lid presses on battery top, wire to VCC pad. No commercial holder needed. | ~12mm footprint | $0.10 |
| ePTFE Membrane | Porex PMV10 or generic ePTFE disc | Hydrophobic, air-permeable, sealant-resistant | ~4mm disc | $0.15 |
| Screws | M1.4 x 3mm stainless steel Allen (DIN912) | 2 per block, 4 total per sensor | 2.6mm head dia, 1.4mm head height | $0.20 |
| O-Rings | Silicone, custom size | Chemical-resistant, -60 to +200°C, 2 per sensor | ~12 x 1mm cross-section | $0.10 |
| DC-DC Inductor | 10µH 0402/0603 | Between MDBT42Q VDD (pin 11) and DCC (pin 10). Enables internal buck converter — reduces radio TX current ~30% (7.5mA → 5.3mA). | 0402 or 0603 | $0.05 |
| Passives | Various 0402 caps, resistors | 3x 100nF decoupling, 2x 4.7kΩ I2C pull-ups, 2x 150pF NFC tuning caps = 7 passive components | 0402 | $0.35 |
| NFC Antenna | PCB trace loop | ~10x15mm rectangular copper trace on bottom layer, connected to MDBT42Q NFC1/NFC2 pins. No component — just a routed trace. Ground pour gap required under antenna. | PCB trace | $0.00 |
| PCB | 2-layer FR4, 0.8mm, ENIG finish | ~16 x 42mm strip with valve hole, conformal coated | — | $1.00 |
| Enclosure | 3D printed ABS (FDM), frame-on-PCB | 2 frames + 2 lids, brass heat-set M1.4 inserts, ~5.5mm boss dia at screw locations | ~15 x 16mm each | $2.50 |
| Conformal Coating | Silicone spray | Applied to exposed PCB strip between blocks | — | $0.10 |

### BOM Summary Per Sensor

- **Electronics (est. at volume):** ~$23 (MCU $4.95 + MS5837 $11.09 + accel $2.47 + 220µF buffer cap $3.85 + passives $0.62 + battery contact $0.10)
- **Electronics (actual small-qty):** ~$39 (MCU $14.33 + MS5837 $17.22 + accel $1.99 + 220µF buffer cap $1.51 + passives $1.80 + battery contact $0.10)
- **Mechanical:** ~$4.35 (enclosure + PCB + screws + O-rings + membrane)
- **Total per sensor (volume est.):** ~$28
- **Total per sensor (actual small-qty):** ~$44
- **Total per pair (actual):** ~$88 (before assembly labor)

### Weight Breakdown Per Sensor

- MDBT42Q module: ~0.5g
- Pressure sensor: ~0.3g
- Accelerometer + passives + 220µF buffer cap + DC-DC inductor: ~0.5g
- CR1225 battery: ~0.9g
- PCB strip (FR4): ~0.8g
- Enclosures (2x ABS frames + lids): ~2.5g
- Screws (4x M1.4 stainless DIN912) + brass inserts: ~0.8g
- O-rings (2x silicone): ~0.2g
- **Total: ~6.5g**

---

## 3. Firmware Architecture

### Software Stack

- **Hardware:** nRF52832 ARM Cortex-M4F (via MDBT42Q module)
- **Protocol stack:** Nordic SoftDevice S332 (concurrent BLE + ANT+)
- **SDK:** Nordic nRF5 SDK 17.1 (or nRF Connect SDK / Zephyr RTOS)
- **Language:** C
- **IDE:** VS Code + nRF Connect extension (firmware), Fusion 360 Electronics + Mechanical (PCB + enclosure)
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

1. **Pressure Manager:** Reads TE MS5837-30BA via I2C. Applies factory calibration polynomial (stored in flash) to raw 24-bit reading. Returns absolute pressure (mbar) and temperature (°C). The sensor transmits absolute pressure — gauge conversion happens on the receiving device (Garmin/phone).

2. **Motion Manager:** Configures LIS2DH12 motion-detect interrupt. Wakes MCU on wheel rotation. Reports motion/no-motion state. Manages 5-minute inactivity timeout.

3. **Alert Engine:** Implements temperature-compensated flat detection using ideal gas law (P1/T1 = P2/T2). Compares current pressure against expected pressure at current temperature based on baseline. Triggers alert only when pressure drop exceeds thermal explanation. User-configurable threshold stored in flash.

4. **ANT+ Tire Pressure Profile:** Broadcasts standard tire pressure data page every 3 seconds. Fields: pressure (0.01 PSI resolution), temperature, sensor location (front/rear), battery status. Requires ANT+ adopter license (~$100/year).

5. **BLE Custom GATT Service:** Characteristics:
   - Pressure (Read, Notify) — int32, 0.01 mbar (absolute). Receiving device subtracts atmospheric.
   - Temperature (Read, Notify) — int32, 0.01°C
   - Battery Level (Read, Notify) — standard BLE Battery Service, uint8 0-100%
   - Sensor Location (Read, Write) — 0x01=Front, 0x02=Rear, stored in flash
   - Alert Threshold (Read, Write) — int32, 0.01 mbar, stored in flash
   - TX Interval (Read, Write) — uint8, 1-10 seconds, default 3, stored in flash
   - Baseline P/T (Read, Write) — reference pressure + temp for compensation

6. **OTA DFU:** Nordic secure DFU bootloader. Signed firmware images. Update via nRF Connect app on phone. Bootloader survives application firmware crashes.

7. **Non-Volatile Storage:** Uses Nordic FDS (Flash Data Storage) for: sensor location, alert threshold, TX interval, baseline P/T, BLE bonding keys. Persists across battery swaps.

8. **Watchdog (WDT):** Hardware watchdog timer resets MCU if firmware hangs. Non-negotiable for a sealed, unserviceable field device — a crash without WDT means permanent death until battery swap. Configured to run during sleep. Negligible power draw.

9. **Battery Monitor (SAADC):** Reads CR1225 voltage via nRF52832's internal VDD/3 ADC channel. No external components. Reports battery level (0-100%) in BLE Battery Service and ANT+ battery status field. Triggers low-battery warning when voltage drops below ~2.4V. Measured once per TX cycle.

10. **Reset Diagnostics (RESETREAS):** Reads the RESETREAS register at boot to determine why the chip last reset (watchdog, brownout, pin reset, lockup, software). Logs reset reason to flash and optionally transmits via BLE diagnostic characteristic. Watchdog resets indicate firmware bugs; brownout resets indicate dying battery.

11. **DC-DC Buck Converter:** Enabled via `NRF_POWER->DCDCEN = 1` at init. Uses external 10µH inductor between VDD and DCC pins. Reduces radio TX current from ~7.5mA to ~5.3mA at 3.0V supply — a ~30% power reduction during active transmission. Significant battery life extension over 2-3 years of periodic TX.

### Power Optimization Features

- **RTC-based timing:** TX intervals use RTC (app_timer, 32.768kHz LFCLK) instead of hardware TIMER peripherals. RTC draws ~0.1µA vs ~8µA for TIMER — critical for a device that spends 99.9% of time in low-power states.
- **TWIM with DMA:** I2C transfers to MS5837-30BA and LIS2DH12 use TWIM (TWI Master with EasyDMA). CPU sleeps during multi-byte transfers instead of babysitting each byte. Free power savings.
- **PPI chains:** Programmable Peripheral Interconnect routes hardware events without CPU wake. Example: accelerometer INT → GPIOTE event → PPI → starts SAADC battery measurement → PPI → SAADC done flag. CPU only wakes to read the result.
- **On-die temperature sensor:** Built-in nRF52832 die temp sensor (0.25°C resolution, ~36µs read) available as backup/cross-check for MS5837 temperature reading. Firmware only, zero power cost.

### Power Budget

| State | Current (LDO) | Current (DC-DC) | Duration | Notes |
|-------|--------------|----------------|----------|-------|
| Deep sleep | ~3µA | ~3µA | 99% of time when not riding | MCU System OFF + accel motion-detect |
| Waking | ~5mA | ~3.5mA | ~2ms | Sensor read + radio init |
| Active TX (ANT+) | ~8mA | ~5.3mA | ~2ms per burst | Every 3 seconds |
| Active TX (BLE) | ~12mA | ~8mA | ~3ms per burst | Every 3 seconds |
| Active sleep between TX | ~5µA | ~3µA | ~2.99s | MCU idle, RTC timers running |
| SAADC battery read | ~1mA | ~0.7mA | ~40µs | Once per TX cycle, via PPI |

**Estimated battery life:** 2-3+ years at 1 hour/day riding with CR1225 (48mAh). DC-DC mode extends active-state battery life by ~30%.

---

## 4. Enclosure & Mechanical Design

### Frame-on-PCB Construction

The enclosure is a **frame** (walls only, no floor) bonded permanently to the PCB with RTV silicone adhesive. The PCB serves as the structural bottom. A removable lid screws down from the top with an O-ring seal.

```
Cross-section (side view):

    Screw    ╔══╗              ╔══╗    Screw
    M1.4     ║  ║  ┌────────┐  ║  ║    M1.4
             ║  ║  │  LID   │  ║  ║
             ║  ║  │ (ABS)  │  ║  ║
             ║  ║  ══════════  ║  ║    ← O-ring
             ║  ║              ║  ║
             ║  ║  components  ║  ║    ← FRAME (ABS walls, no floor)
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
| Lid (ABS, 1.2mm) | 1.2mm |
| **Total (lid surface)** | **5.2mm** |
| M1.4 DIN912 screw head protrusion | +1.4mm |
| **Total (incl. screw heads)** | **6.6mm** |

**Block B (Battery + Pressure Sensor):**

| Layer | Thickness |
|-------|-----------|
| PCB (is the floor) | 0.8mm |
| Tallest component (CR1225 battery 2.5mm sitting on PCB pad; MS5837 is 2.75mm) | 2.75mm |
| Air gap + lid spring compression | 0.25mm |
| O-ring (compressed) | 0.5mm |
| Lid (ABS, 1.2mm) | 1.2mm |
| **Total (lid surface)** | **5.5mm** |
| M1.4 DIN912 screw head protrusion | +1.4mm |
| **Total (incl. screw heads)** | **6.9mm** |

Note: With no commercial battery holder, the MS5837-30BA (2.75mm) is now the tallest component in Block B, not the battery (2.5mm). The CR1225 sits directly on a ~10mm ENIG copper pad (negative contact). A BeCu leaf spring inside the lid presses on the battery top (positive contact) and is wired to a VCC pad on the PCB. The lid provides both retention and positive electrical contact. Battery cannot escape the sealed block.

Note: Screw heads (M1.4 DIN912, 1.4mm tall, 2.6mm dia) protrude above the lid surface into the tire cavity. This is acceptable — the tire-facing side has ample clearance. The rim-facing height (PCB bottom to lid surface) remains 5.2/5.5mm. If flush lids are desired, counterbore the screw heads into a thicker lid (1.6mm).

### 3D Printing Specifications

- **Material:** ABS (Acrylonitrile Butadiene Styrene) — reliable acetone vapor smoothing for O-ring seals, no UV exposure needed (inside tire)
- **Method:** FDM
- **Layer height:** 0.1mm (fine detail for O-ring groove)
- **Infill:** 100% (structural, waterproof walls)
- **Minimum wall thickness:** 1.2mm (all walls), ~5.5mm diameter bosses at M1.4 insert locations
- **Post-processing:** Acetone vapor smoothing on O-ring groove surfaces for gas-tight seal
- **Scale-up path:** Injection molding in PA66 or ABS when volume exceeds ~500 units (already ABS for FDM prototypes)

### Sealing System

1. **Frame-to-PCB:** Permanent bond with RTV silicone adhesive. Flexible (survives vibration), chemically inert to tire sealant. PCB surface lightly roughened in bonding area for adhesion.
2. **Lid-to-frame:** Silicone O-ring (1mm cross-section) in rectangular groove in frame top rim. Lid compresses O-ring ~25% when screwed down.
3. **Screw retention:** 2x M1.4 stainless Allen screws (DIN912) per block into brass heat-set inserts (~5.5mm boss diameter). Loctite 222 threadlocker. Silicone grease dab on hex sockets to prevent sealant curing inside.
4. **Pressure port:** 2mm hole in Block B lid with ePTFE membrane bonded on inner surface. Air-permeable, liquid-proof. Inspectable during battery swap.
5. **Exposed PCB strip:** 2-3 coats silicone conformal coating between blocks. No exposed copper.

### Tire Insert Compatibility

| Insert | Height Clearance | Compatibility |
|--------|-----------------|---------------|
| Tannus Tubeless (Pro/Fusion) | Open valve channel | Compatible |
| Vittoria Air-Liner | Valve channel design | Compatible |
| CushCore XC | 6-8mm base at valve | Compatible (6.9mm incl. screw heads — tight fit, verify) |
| CushCore Trail | 6-8mm base at valve | Compatible (6.9mm incl. screw heads — tight fit, verify) |
| CushCore Pro | 8-10mm base, tight valve area | Verify during prototyping |

---

## 5. Communication Protocols & Compatibility

### ANT+ Tire Pressure Profile

ANT+ broadcast of **absolute pressure** (mbar). A custom Connect IQ data field on the Garmin reads the sensor's absolute pressure AND the Garmin's built-in barometer (`Activity.Info.rawAmbientPressure`), subtracts atmospheric, and displays gauge PSI. No phone needed.

- Pressure: absolute, 0.01 mbar resolution
- Temperature: 0.01°C resolution
- Sensor location: Front/Rear
- Battery status: OK/Low/Critical
- Broadcast interval: 3 seconds (configurable 1-10s)
- **License:** ANT+ adopter license required (~$100/year for small volume)

### Garmin Connect IQ Data Field

Custom data field written in Monkey C. This is the primary user interface for Garmin users:
- Reads absolute tire pressure from sensor via ANT+
- Reads `Activity.Info.rawAmbientPressure` from Garmin's barometer (API Level 2.4.0+)
- Computes and displays: `gauge_psi = (tire_absolute - atmospheric) * 0.0145038`
- Shows front/rear pressure, temperature, battery status
- Triggers audible alert on flat detection
- Available on Connect IQ store for free download

### BLE Custom GATT Service

128-bit custom UUIDs. Transmits **absolute pressure** (mbar). Phone app reads phone barometer for atmospheric subtraction.

### v1 Configuration: Web Bluetooth

- Web page hosted on GitHub Pages, opened in Chrome
- Reads phone barometer (Web Sensor API / Generic Sensor API) for atmospheric reference
- Displays: gauge pressure (absolute - atmospheric), temperature, battery level
- Configures: sensor location (F/R), alert threshold, TX interval
- **iOS limitation:** Safari doesn't support Web Bluetooth. Use Bluefy browser (free) as workaround until native app is built.

### NFC Pairing (Hardware Ready, Firmware v2+)

The MDBT42Q's nRF52832 has built-in NFC-A tag hardware. The PCB includes a trace loop antenna and tuning capacitors, so NFC can be enabled via firmware update — no hardware change needed.

- **v1 firmware:** NFC disabled. Pairing via BLE scan ("OpenTPMS" device name).
- **v2 firmware:** Enable NFC. Phone tap → reads NDEF record with sensor serial number → app auto-pairs via BLE. Same workflow as SRAM TyreWiz. Useful for initial setup before sealing sensor in tire.

### OTA Firmware Updates

- Nordic secure DFU bootloader
- Update via nRF Connect app (iOS/Android)
- Signed firmware images prevent unauthorized modification
- User must pull tire to get BLE connection (sensor inside tire has limited range when stationary)

### Device Compatibility

| Device | Live Gauge PSI | Atmospheric Source | Flat Alerts | Config | OTA | Protocol |
|--------|:---:|---|:---:|:---:|:---:|----------|
| Garmin Edge 530+ | ✓ | Garmin barometer (rawAmbientPressure) | ✓ | — | — | ANT+ + Connect IQ data field |
| Garmin Fenix/Forerunner | ✓ | Garmin barometer | ✓ | — | — | ANT+ + Connect IQ data field |
| Wahoo Elemnt/Bolt/Roam | ✓ | Wahoo barometer (if API available) | ? | — | — | ANT+ |
| Hammerhead Karoo 2/3 | ✓ | Karoo barometer | ✓ | — | — | ANT+ or BLE extension |
| Android Phone (Chrome) | ✓ | Phone barometer (Web Sensor API) | ✓ | ✓ | ✓ | BLE + Web Bluetooth |
| iPhone (Bluefy browser) | ✓ | Phone barometer | ✓ | ✓ | — | BLE + Web BLE workaround |
| iPhone (nRF Connect) | — | — | — | — | ✓ | BLE DFU only |

---

## 6. Testing & Validation

### Test Phases

1. **Bench Testing:** Firmware flash, sensor accuracy vs reference gauge, BLE/ANT+ pairing, power consumption, sleep/wake cycle, OTA DFU, alert logic.
2. **In-Tire Testing:** Seal integrity (submersion test), sealant exposure, pressure hold over days, insert compatibility, RF range through tire/rim, battery swap procedure.
3. **Field Testing:** Multi-hour rides, temperature swings, vibration/impacts, flat detection real-world, battery drain rate, multi-bike pairing, head unit compatibility, cold weather (-30°C).

### Critical Test Procedures & Pass Criteria

1. **Pressure Accuracy:** After factory calibration, ±1% of reading at 20 PSI (±0.2 PSI) when paired with a barometer-equipped head unit (Garmin) or phone. Test at 10, 20, 30, 60, 100 PSI. Repeat at -20°C, 20°C, 60°C.
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
2. **SMT assembly:** Solder paste stencil + reflow oven (or order assembled from JLCPCB SMT service). Components: MDBT42Q, LIS2DH12, MS5837-30BA pressure sensor, SMTM1225 battery clip, 220µF buffer cap, 10µH DC-DC inductor, passives.
3. **Conformal coat:** Spray silicone conformal coating on exposed PCB strip (valve area), 2-3 coats.
4. **3D print enclosures:** FDM ABS, 0.1mm layers, 100% infill. 2 frames + 2 lids per sensor. Acetone vapor smooth O-ring surfaces.
5. **Heat-set inserts:** Press M1.4 brass inserts into frame wall bosses (~5.5mm dia) using soldering iron tip.
6. **Bond frames to PCB:** Apply RTV silicone to frame bottom, position on PCB, cure 24 hours.
7. **Install ePTFE membrane:** Bond ePTFE disc over pressure port hole in Block B lid with medical-grade adhesive.
8. **Install O-rings:** Place silicone O-rings in frame grooves.
9. **Insert battery:** Place CR1225 negative-side-down onto the ENIG copper pad on PCB. Lid spring (step 10) provides positive contact and retention.
10. **Close lids:** Screw down both lids with M1.4 DIN912 Allen screws + Loctite 222. Dab silicone grease on hex sockets.
11. **Flash firmware:** Connect J-Link to SWD pads on PCB strip. Flash bootloader + application firmware. Assign serial number and default F/R designation.
12. **Factory pressure calibration:** Using a hand pump + reference gauge, test sensor at 3-5 known absolute pressures (e.g., atmospheric, 1.5 bar, 3 bar, 5 bar, 8 bar). Record sensor reading vs reference at each point. Compute correction polynomial coefficients. Write to nRF52 flash via SWD. ~5 minutes per sensor.
13. **QC test:** Verify corrected pressure reading matches reference within ±0.15 PSI, BLE/ANT+ broadcast, motion wake, seal check (visual).

### Scale-Up Path

- **1-100 units:** Hand assembly, 3D printed enclosures, JLCPCB SMT assembly
- **100-500 units:** Same process, batch 3D printing, streamlined assembly jig
- **500+ units:** Transition to injection-molded enclosures (PA66/ABS), full SMT assembly line, automated testing fixture

---

## 8. Open-Source Deliverables

- **Hardware:** Fusion 360 Electronics project (schematic + layout), exported Gerber files, BOM
- **Firmware:** nRF5 SDK project (C), SoftDevice configuration, build scripts, factory calibration tool
- **Garmin Connect IQ Data Field:** Monkey C app that reads sensor absolute pressure + Garmin barometer, displays gauge PSI. Published on Connect IQ store.
- **Enclosure:** STEP/STL files for 3D printing, assembly drawings
- **Web Config Tool:** HTML/JS Web Bluetooth configuration page with phone barometer integration
- **Documentation:** Assembly guide, user manual, factory calibration procedure, testing procedures, insert compatibility results
- **License:** CERN Open Hardware License v2 (hardware), MIT (firmware/software/Connect IQ app)
