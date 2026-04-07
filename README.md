# OpenTPMS — Open-Source Bicycle Tire Pressure Monitoring Sensor

An open-source, in-tire TPMS sensor for tubeless bicycles with replaceable CR1225 battery, dual ANT+/BLE wireless, and temperature-compensated flat detection.

**Sponsored by [PCBWay](https://www.pcbway.com) — PCB Prototype the Easy Way**

![PCBWay Logo](https://www.pcbway.com/project/img/logo.png)

---

## Key Features

- **Replaceable battery** — CR1225 coin cell, 2-3 year life at 1hr/day riding
- **Dual ANT+ and BLE** — native Garmin/Wahoo support AND phone connectivity
- **Factory-calibrated accuracy** — within 1% at 20 PSI (vs 2% for commercial alternatives)
- **Temperature-compensated flat alerts** — eliminates false alarms from weather changes
- **Cold weather operation** — -30C with ceramic buffer cap + DC-DC converter
- **Universal fit** — 16mm wide PCB fits 19mm+ internal width rims (all modern tubeless)
- **Open-source** — CERN OHL v2 (hardware) + MIT (firmware/software)
- **Lightweight** — ~6.5g per sensor

## How It Works

The sensor mounts on the rim bed inside a tubeless tire, around the valve stem. It measures absolute pressure using a factory-calibrated TE MS5837-30BA sensor and transmits via ANT+ and BLE every 3 seconds. A Garmin Connect IQ data field or phone app reads the device's barometer and subtracts atmospheric pressure to display gauge PSI in real-time.

Temperature-compensated flat detection uses the ideal gas law (P1/T1 = P2/T2) to distinguish actual leaks from temperature-induced pressure changes — no false alerts riding from shade to sun.

## Architecture

```
        Valve stem
            |
 +----------+----------+
 | Block A  O  Block B |
 | (elec)   |  (batt)  |
 +----------+----------+
            |
         Rim bed
```

- **Block A:** Raytac MDBT42Q (nRF52832), LIS2DH12 accelerometer, passives
- **Block B:** MS5837-30BA pressure sensor, CR1225 battery on PCB contact pad
- **PCB:** 2-layer FR4, 16mm x 42mm, 0.8mm thick, ENIG finish
- **Enclosure:** 3D printed ABS, frame-on-PCB construction, O-ring sealed

## Specifications

| Spec | Value |
|------|-------|
| Pressure accuracy | ±0.4-0.8% at 20 PSI (factory calibrated) |
| Pressure range | 0-435 PSI |
| Operating temp | -30C to +60C |
| Battery | CR1225, replaceable, 2-3 year life |
| Weight | ~6.5g per sensor |
| Dimensions | 16mm x 42mm x 5.5mm (lid), 6.9mm (with screws) |
| Wireless | BLE 5.0 + ANT+ (concurrent) |
| Flat detection | Temperature-compensated, <30s detection |
| Rim compatibility | 19mm+ internal width (all modern tubeless) |

## Comparison vs SRAM TyreWiz 2.0

| Feature | OpenTPMS | TyreWiz 2.0 |
|---------|----------|-------------|
| Price | ~$44/sensor | $60+/sensor |
| Battery life | 2-3 years | 200-300 hours |
| Accuracy | ±0.4-0.8% | ±2% |
| Cold weather | -30C | -12C |
| Temp-compensated alerts | Yes | No |
| Open source | Yes | No |
| Weight | 6.5g | 10g |

## Project Status

- [x] Design specification (Rev 8)
- [x] Component selection and BOM
- [x] Schematic design (Fusion 360 Electronics)
- [x] PCB layout and routing
- [x] Gerber files exported for fabrication
- [ ] PCB fabrication (PCBWay sponsorship)
- [ ] Component sourcing (DigiKey + Mouser + Amazon + AliExpress)
- [ ] Firmware development
- [ ] Garmin Connect IQ data field
- [ ] Web Bluetooth configuration tool
- [ ] Enclosure CAD design
- [ ] Assembly and testing
- [ ] Field testing

## Repository Structure

```
OpenTPMS/
├── README.md
├── LICENSE-HARDWARE          # CERN OHL v2
├── LICENSE-SOFTWARE          # MIT
├── docs/
│   ├── design-spec.md        # Full design specification
│   ├── implementation-plan.md # Build plan with task tracking
│   ├── OpenTPMS_BOM.xlsx     # Bill of materials (all suppliers)
│   ├── opentpms-vs-tyrewiz.md # Competitive comparison
│   └── fusion360-schematic-guide.md
├── hardware/
│   ├── gerbers/              # Manufacturing files (Gerber + drill)
│   └── fusion360/            # Fusion 360 project files
├── firmware/
│   ├── src/                  # C source (nRF5 SDK)
│   ├── config/               # SDK and app configuration
│   └── tests/                # Unit tests
├── garmin/                   # Connect IQ data field (Monkey C)
├── web-config/               # Web Bluetooth configuration tool
└── tools/                    # Factory calibration scripts
```

## Bill of Materials (per sensor)

### Electronics (~$39 small quantity)
| Component | Part | Supplier |
|-----------|------|----------|
| MCU + Radio | Raytac MDBT42Q-512KV2 (nRF52832) | Mouser |
| Pressure Sensor | TE MS5837-30BA | DigiKey |
| Accelerometer | ST LIS2DH12TR | Mouser |
| TX Buffer Cap | Murata GRM31CR60J227ME11L (220uF 1206) | DigiKey |
| DC-DC Inductor | 10uH 0402 | DigiKey |
| Battery Contact | Custom PCB pad + lid spring | — |
| Passives | 100nF, 150pF, 4.7k, 100ohm (0402) | DigiKey/Mouser |

### Mechanical
| Component | Supplier |
|-----------|----------|
| CR1225 batteries | Amazon |
| M1.4 DIN912 Allen screws | AliExpress |
| M1.4 brass heat-set inserts | AliExpress |
| Silicone O-rings (12mm, 1mm) | Amazon |
| ePTFE vent membrane | AliExpress |
| MG Chemicals 422C conformal coating | Amazon |
| Permatex Clear RTV silicone | Amazon |
| Loctite 222 threadlocker | Amazon |

### PCB
- 2-layer, 0.8mm FR4, ENIG, 16mm x 42mm
- Fabrication: **PCBWay** (sponsored)

## Building

### Prerequisites
- Nordic nRF5 SDK 17.1
- GCC ARM toolchain
- nRF52-DK or J-Link programmer
- Fusion 360 (PCB/enclosure design)
- Pinecil V2 or similar soldering iron

### Assembly
1. Order PCBs from Gerber files in `hardware/gerbers/`
2. Order components per BOM
3. Solder SMD components with solder paste + hot air or iron
4. Flash firmware via SWD test pads
5. 3D print ABS enclosure
6. Assemble with RTV silicone, O-rings, M1.4 screws
7. Factory calibrate pressure sensor
8. Install in tire

## License

- **Hardware:** CERN Open Hardware Licence Version 2 — Strongly Reciprocal (CERN-OHL-S-2.0)
- **Firmware/Software:** MIT License

## Acknowledgments

- **PCBWay** — PCB fabrication sponsorship
- Built with Nordic nRF52832 via Raytac MDBT42Q module
- Designed in Autodesk Fusion 360

---

*OpenTPMS is an independent open-source project. Not affiliated with SRAM, Garmin, or any commercial TPMS manufacturer.*
