# OpenTPMS vs SRAM TyreWiz 2.0 — Competitive Comparison

**Date:** 2026-04-05

---

## Head-to-Head Comparison

| Feature | **OpenTPMS (ours)** | **TyreWiz 2.0** | Winner |
|---|---|---|---|
| **Price** | ~$44/sensor ($88/pair) small-qty, ~$28 at volume | $120-130/pair + valve stems sold separately | **OpenTPMS** |
| **Weight** | ~6.5g | ~10g | **OpenTPMS** |
| **Height** | 5.5mm (lid), 6.9mm (w/ screws) | ~5-7mm (estimated, SRAM doesn't publish) | Tie |
| **Battery** | CR1225 (48mAh), replaceable, easy snap-clip | CR1632 (140mAh), replaceable, improved in 2.0 | TyreWiz (bigger battery) |
| **Battery life** | 2-3 years (1hr/day) | 200-300 hours (~1 year at 1hr/day) | **OpenTPMS** (10x longer) |
| **Cold weather** | -30°C (supercap buffer) | ~-12°C (struggles below) | **OpenTPMS** |
| **Wireless** | ANT+ AND BLE simultaneously | ANT+ AND BLE + NFC for pairing | Tie |
| **Accuracy** | ±0.4-0.8% at 20 PSI (after factory cal) | ±2% | **OpenTPMS** (2-5x better) |
| **Temp compensation** | Yes — ideal gas law flat detection | No — raw pressure only | **OpenTPMS** |
| **Flat detection** | Yes, temp-compensated (no false alerts) | Yes, but false positives reported | **OpenTPMS** |
| **Pressure range** | 0-435 PSI (MS5837-30BA) | 0-150 PSI | **OpenTPMS** |
| **Device support** | Garmin (Connect IQ), Wahoo, phones (Web BLE) | Garmin, Wahoo, Karoo, SRAM AXS app | TyreWiz (polished app) |
| **Mounting** | PCB strip on valve stem, inside tire | Ring around valve base, inside tire | Tie |
| **Sealant** | Sealed enclosure + ePTFE pressure port | IPX7, sealant compatible | Tie |
| **Insert compat.** | CushCore XC/Trail (tight fit), Tannus, Vittoria | CushCore eMTB explicit, others likely | Tie |
| **OTA updates** | Yes (Nordic DFU via nRF Connect) | Yes (via SRAM AXS app) | Tie |
| **Open source** | Fully open (CERN OHL v2 + MIT) | Proprietary, closed | **OpenTPMS** |
| **LED indicator** | No | Green/Red pressure status LED | TyreWiz |
| **NFC pairing** | No | Yes (tap phone to sensor before install) | TyreWiz |
| **Sold individually** | Yes | Pair only | **OpenTPMS** |

---

## How TyreWiz Reads Pressure (Both Sensors Are INSIDE the Tire)

**Important clarification:** TyreWiz does NOT mount on the outside of the tire. Like OpenTPMS, it sits on the rim bed INSIDE the tire, sealed under the tire bead. Both designs are submerged in the pressurized air (and sealant) inside a tubeless tire.

The difference is HOW they protect the pressure sensor from sealant while still reading air pressure:

### TyreWiz approach:
- The sensor body sits directly on the rim bed, wrapped around the valve stem base
- The pressure sensor element is likely exposed to tire air through a small port or membrane
- The unit is IPX7 rated (waterproof to 1m for 30 min) to handle sealant
- The electronics are sealed/potted, but the pressure sensor must have air access
- **Limitation:** SRAM explicitly warns you CANNOT inject sealant through the TyreWiz — you must remove the sensor, add sealant through the bare valve, then reinstall. This suggests the pressure port can be clogged by direct sealant injection.

### OpenTPMS approach:
- The sensor sits on the rim bed on a PCB strip, inside a sealed ABS enclosure
- The MS5837-30BA pressure sensor is gel-filled (inherently sealant-resistant) and sits inside sealed Block B
- Air pressure reaches the sensor through a **2mm ePTFE membrane** in the Block B lid — air passes through, liquid sealant does not
- The ePTFE membrane is inspectable during battery swaps
- **Advantage:** The sealed enclosure + ePTFE membrane is a more robust sealant barrier than TyreWiz's approach. No restriction on sealant injection method.

### Why both must be inside the tire:
A sensor mounted on the outside (e.g., on the valve cap like cheap car TPMS) cannot accurately measure tire pressure in a bicycle context:
- External cap sensors measure pressure at the valve stem, which can differ from tire body pressure when the valve core is closed
- They add rotating mass on the valve, causing imbalance
- They're exposed to impacts, theft, and weather
- They can't detect rapid pressure changes (flat detection) because the valve core acts as a restriction

Both TyreWiz and OpenTPMS solve this by being inside the tire, directly bathed in the pressurized air. This gives real-time, unrestricted pressure readings.

---

## What NFC Pairing Is and Why TyreWiz Uses It

### What NFC is:
NFC (Near Field Communication) is an extremely short-range wireless protocol (~1-4cm). You literally tap your phone against the sensor. It's the same technology used for contactless payments (Apple Pay, Google Pay) and transit cards.

### How TyreWiz uses NFC:
1. **Before installing the sensor in the tire**, you open the SRAM AXS app on your phone
2. You tap the phone against the TyreWiz sensor (NFC range = a few centimeters)
3. The app instantly identifies the sensor, reads its serial number, and pairs it
4. You assign it as Front or Rear, set your target pressure
5. Then you install the sensor inside the tire

### Why NFC is useful:
- **Convenience:** No BLE scanning, no searching for "TyreWiz_XXXX" in a list of Bluetooth devices. Just tap and pair.
- **Works before installation:** Once the sensor is sealed inside the tire, BLE range is very limited (tire + rim act as a Faraday cage at close range). NFC lets you do the initial setup easily before sealing it up.
- **Sensor identification:** If you have multiple sensors, NFC tells you exactly which physical sensor you're configuring — no guessing which BLE device name corresponds to which sensor.

### Does OpenTPMS need NFC?
Not really. Here's why:
- Our BLE pairing is simple — scan for "OpenTPMS" in the web config tool or nRF Connect app
- We assign Front/Rear via BLE characteristic write (stored in flash)
- Initial setup happens before tire installation anyway (flash firmware, factory calibrate, configure)
- NFC adds hardware cost (antenna + NFC controller) and PCB space for a feature used once per sensor lifetime
- The nRF52832 (via MDBT42Q) actually has NFC capability built-in, so we COULD add it later via firmware update if desired — the hardware supports it, we just don't use it in v1

---

## Where OpenTPMS Wins Big

1. **Battery life** — 2-3 years vs ~200 hours. This is the killer feature. Set it, forget it for years. TyreWiz users replace batteries every few months.
2. **Cold weather** — -30°C vs -12°C. The 220µF ceramic supercap buffers TX current bursts that a cold CR1225 alone couldn't sustain. TyreWiz has no buffer cap.
3. **Accuracy** — ±0.4-0.8% vs ±2%. Factory calibration polynomial on each sensor removes systematic error.
4. **Temp-compensated alerts** — TyreWiz users report false pressure alerts from temperature changes (riding from shade to sun = ~1 PSI swing). OpenTPMS uses ideal gas law to distinguish temperature effects from actual leaks.
5. **Price** — Even at prototype pricing, a pair costs $88 vs $120-130. At volume, ~$56/pair.
6. **Open source** — Community can modify, improve, and build their own. No SRAM lock-in.

## Where TyreWiz 2.0 Wins

1. **App ecosystem** — SRAM AXS app is polished, NFC pairing is slick. OpenTPMS v1 uses a Web Bluetooth page.
2. **LED indicator** — Green/Red LED shows pressure status visually without a phone or head unit. Useful for pre-ride pressure check.
3. **Consumer product** — Buy, install, ride. No soldering, no assembly, no firmware flashing.
4. **Established brand** — SRAM/Quarq has cycling industry credibility.

## TyreWiz Weaknesses That OpenTPMS Exploits

- **$120+ price** for "what a $5 gauge does" (common user complaint)
- **False pressure alerts** from temperature swings (no temp compensation)
- **Battery life only 200-300 hours** — frequent replacement
- **Can't inject sealant through sensor** — must remove first
- **Valve stems sold separately** — adds to total cost
- **Proprietary** — no community improvements, stuck with SRAM's firmware
- **Low-pressure accuracy issues** — users report "random number generator" behavior at MTB pressures on 1.0; 2.0 claims improvement but ±2% at 20 PSI = ±0.4 PSI vs our ±0.17 PSI
- **No cold weather optimization** — struggles below -12°C

## Note on NFC Capability

The MDBT42Q-512KV2 module uses the nRF52832 SoC which has built-in NFC-A tag functionality. The NFC antenna pads (NFC1/NFC2) are available on the module pinout. If we decide to add NFC pairing in a future revision, it requires only a simple PCB loop antenna (copper trace) — no additional chips. This is a firmware-only upgrade path. For v1, we skip it to keep the design simple and the PCB small.
