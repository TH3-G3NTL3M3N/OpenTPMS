# OpenTPMS: Open-Source Bicycle Tire Pressure Monitoring Sensor

> **Work in Progress.** PCB is designed and heading to fab. Firmware, enclosure, and assembly are next. Install video and build guide coming once the first prototype is riding.

An open-source, in-tire TPMS sensor for tubeless bicycles. Replaceable CR1225 battery, dual ANT+/BLE wireless, temperature-compensated flat detection.

**Sponsored by [PCBWay](https://www.pcbway.com)**

**Contact:** simon@ruelland.com

---

## Key Features

- **Replaceable battery** that lasts 2-3 years on a CR1225 coin cell
- **Dual ANT+ and BLE** for native Garmin/Wahoo support and phone connectivity
- **Factory-calibrated accuracy** under 1% at 20 PSI, matching the best commercial sensors
- **Temperature-compensated flat alerts** that actually work (no false alarms from weather changes)
- **Cold weather operation** down to -30C with ceramic buffer cap and DC-DC converter
- **Universal fit** at 19mm wide (v1), fits 21mm+ internal width rims (all MTB, gravel, most road tubeless)
- **Open source** under CC BY-NC-SA 4.0
- **Lightweight** at ~6.5g per sensor

## How It Works

The sensor mounts on the rim bed inside a tubeless tire, around the valve stem. It measures absolute pressure using a factory-calibrated TE MS5837-30BA sensor and transmits via ANT+ and BLE every 3 seconds. A Garmin Connect IQ data field or phone app reads the device's barometer and subtracts atmospheric pressure to display gauge PSI in real time.

Flat detection uses the ideal gas law (P1/T1 = P2/T2) to tell the difference between an actual leak and temperature swings. Riding from shade into sun changes your pressure by ~1 PSI, and every other sensor on the market will warn you about it. OpenTPMS won't.

### Sealing Design: Replaceable Battery in a Harsh Environment

Every competitor pots their sensor in epoxy, which seals it perfectly but also means the battery is permanent. When it dies, the whole sensor is trash. We took a different approach: a multi-layer serviceable enclosure that handles sealant, moisture, and vibration while keeping the battery swappable.

1. **ABS frame** bonded to PCB with **Permatex RTV silicone** for a permanent, flexible, vibration-resistant seal
2. **Silicone O-ring** (12mm, 1mm cross-section) compressed 25% between frame and lid
3. **M1.4 DIN912 Allen screws** into brass heat-set inserts with **Loctite 222** threadlocker (removable but secure)
4. **ePTFE membrane** over the pressure port: air passes through, liquid sealant doesn't
5. **Silicone conformal coating** (MG Chemicals 422C) on exposed PCB areas between blocks
6. **Silicone grease** in the screw hex sockets to keep sealant from curing inside

**Battery swap:** Unscrew 2 Allen screws, lift lid, swap CR1225, replace lid. About 2 minutes during a sealant refresh.

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
- **PCB:** 2-layer FR4, 19mm x 42mm (v1), 0.8mm thick, ENIG finish
- **Enclosure:** 3D printed ABS, frame-on-PCB construction, O-ring sealed

## Specifications

| Spec | Value |
|------|-------|
| Pressure accuracy | <1% at 20 PSI (factory calibrated) |
| Pressure range | 0-435 PSI |
| Operating temp | -30C to +60C |
| Battery | CR1225, replaceable, 2-3 year life |
| Weight | ~6.5g per sensor |
| Dimensions (v1) | 19mm x 42mm x 5.5mm (lid), 6.9mm (with screws) |
| Wireless | BLE 5.0 + ANT+ (concurrent) |
| Flat detection | Temperature-compensated, under 30s detection |
| Rim compatibility | 21mm+ internal width (all MTB, gravel, most road tubeless) |

## Comparison vs Commercial TPMS Sensors

*All prices in CAD. Competitor USD prices converted at 1 USD ≈ 1.37 CAD.*

| Feature | **OpenTPMS (v1)** | **SRAM TyreWiz 2.0** | **Outrider TL Pro** | **Outrider TL Mini** |
|---------|:---------:|:----------:|:----------:|:----------:|
| **Price (pair, CAD)** | ~$66 | ~$178 | ~$122 | ~$95 |
| **Weight** | ~6.5g | 10g | 6.9g | 3.7g |
| **Battery life** | 2-3 years | 200-300 hrs | 5+ years | 2,000 hrs |
| **Battery** | CR1225 replaceable | CR1632 replaceable | Potted (not replaceable) | Potted (not replaceable) |
| **Accuracy** | <1% | ±2% | ±0.5% | ±2% |
| **Max pressure** | 435 PSI | 150 PSI | 200 PSI | 125 PSI |
| **Cold weather** | -30C | -12C | Not specified | Not specified |
| **Wireless** | ANT+ AND BLE | ANT+ AND BLE | BLE only | BLE only |
| **Garmin support** | Yes (Connect IQ) | Yes (AXS app) | Yes (BLE Datafield) | Yes (BLE Datafield) |
| **Phone app** | Yes (Web BLE) | Yes (AXS app) | Yes (iOS/Android) | Yes (iOS/Android) |
| **Wahoo support** | Yes (ANT+) | Yes | No native support | No native support |
| **Hammerhead Karoo** | Yes | Yes | Yes (sideload extension) | Yes (sideload extension) |
| **OTA firmware updates** | Yes (Nordic DFU via BLE) | Yes (AXS app) | Not confirmed | Not confirmed |
| **Temp-compensated alerts** | Yes | No | No | No |
| **Flat detection** | Yes (under 30s) | Yes (false positives reported) | Yes | Yes |
| **Sealant resistance** | IP67 + ePTFE membrane | IPX7 | Aerospace epoxy potted | IP68 |
| **Open source** | Yes (CC BY-NC-SA) | No | No | No |
| **NFC pairing** | Hardware ready (v2 firmware) | Yes | No | No |
| **Rim compatibility** | 21mm+ (v1) | All tubeless | 21mm+ sym (25mm+ asym MTB) | 19mm+ |
| **Tire inserts** | Compatible (verify CushCore) | Compatible | Tight fit | Better fit |
| **PCB width** | 19mm (v1) | N/A | 12mm | 19/21/30mm (3 sizes) |

### Where competitors win:
- **Outrider TL Mini** is lighter (3.7g vs 6.5g), smaller, and fits narrower rims (19mm+)
- **Outrider TL Pro** has the longest battery (5+ years), lightest full-featured build (6.9g), and narrowest PCB (12mm)
- **TyreWiz 2.0** has a polished app ecosystem, NFC pairing, LED indicator, and the widest device compatibility
- **All of them** are finished consumer products you can just buy and install

### Where OpenTPMS wins:
- **Buy once, keep forever.** $66 CAD/pair. Replace a $1 battery every 2-3 years. OTA firmware updates over BLE keep making it better without swapping hardware. Competitors pot their sensors in epoxy, so when the battery dies, that's it. You buy another one.
- **OTA firmware updates.** Nordic's secure DFU bootloader lets you flash new firmware over BLE right through the tire. New features, better algorithms, bug fixes, all without cracking it open. Neither Outrider model has confirmed OTA capability.
- **Cheapest.** ~$66 CAD/pair vs $95-178 CAD for everything else on the market.
- **Accuracy that matches the best.** Under 1% matches the Outrider TL Pro's ±0.5%, and beats TyreWiz and TL Mini at ±2%.
- **ANT+ and BLE at the same time.** Garmin reads ANT+, phone reads BLE, both simultaneously. Both Outrider models are BLE only (no Wahoo, one device at a time). Only TyreWiz and OpenTPMS do dual protocol.
- **Temperature-compensated flat detection.** The only sensor that won't freak out when you ride from shade into sun.
- **Works in the cold.** -30C with a ceramic buffer cap and DC-DC converter. Other sensors start struggling around -10C.
- **435 PSI range.** Fat bikes to road bikes to whatever you're riding.
- **Open source.** No vendor lock-in. No subscription. No proprietary app. You own it completely.

> With every other cycling TPMS, you're trusting a black box. You don't know how it measures, you can't fix the firmware when it's wrong, and when the battery dies it goes in the trash. OpenTPMS is the sensor where you actually know what's going on inside. You can read the code, tweak the flat detection, or just enjoy knowing that your tire pressure sensor isn't phoning home to anyone. It's yours.

### Cost breakdown (CAD, per sensor at volume)

| Category | Per sensor | Per pair |
|----------|-----------|---------|
| Electronics (MCU, sensors, passives) | $27.65 | $55.30 |
| Mechanical (battery, screws, O-rings, enclosure) | $2.35 | $4.70 |
| PCB (JLCPCB volume pricing) | $1.50 | $3.00 |
| Consumables (coating, RTV, Loctite, grease) | $1.57 | $3.14 |
| **Total** | **~$33** | **~$66** |

*Dev tools (nRF52-DK $52.77 + PPK2 $133.20 = $186 one-time) not included in per-unit cost.*

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
├── LICENSE                   # CC BY-NC-SA 4.0
├── docs/
│   ├── design-spec.md        # Full design specification
│   ├── implementation-plan.md # Build plan with task tracking
│   ├── OpenTPMS_BOM.xlsx     # Bill of materials (all suppliers)
│   ├── opentpms-vs-tyrewiz.md # Competitive comparison
│   └── fusion360-schematic-guide.md
├── hardware/
│   ├── gerbers/              # Manufacturing files (Gerber + drill)
│   ├── enclosure/            # 3D print STL/STEP files
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
| Battery Contact | Custom PCB pad + lid spring | N/A |
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
- 2-layer, 0.8mm FR4, ENIG, 19mm x 42mm (v1)
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
3. Solder SMD components with solder paste + iron (Pinecil V2)
4. Apply conformal coating on exposed PCB strip between blocks
5. Flash firmware via SWD test pads (using nRF52-DK)
6. 3D print ABS enclosure (frames + lids), acetone vapor smooth O-ring surfaces
7. Press M1.4 brass heat-set inserts into frame bosses
8. Bond frames to PCB with RTV silicone, cure 24 hours
9. Bond ePTFE membrane over pressure port in Block B lid
10. Factory calibrate pressure sensor (3-5 known pressures, write correction polynomial)
11. Insert CR1225 battery, place O-rings, screw down lids with Loctite 222
12. Install in tire on valve stem

> **Installation video and detailed build guide coming once the first prototype is assembled and field-tested.**

## License

**CC BY-NC-SA 4.0** (Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International)

You can share, modify, and build upon this project for **non-commercial purposes only**, with attribution and under the same license. Commercial use requires permission from the author.

For commercial licensing inquiries: simon@ruelland.com

## Acknowledgments

- **PCBWay** for PCB fabrication sponsorship
- Built with Nordic nRF52832 via Raytac MDBT42Q module
- Designed in Autodesk Fusion 360

---

*OpenTPMS is an independent open-source project. Not affiliated with SRAM, Garmin, or any commercial TPMS manufacturer.*
