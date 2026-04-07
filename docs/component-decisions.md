# Component Decisions: Why We Chose What We Chose

This document explains the thinking behind every major component choice in OpenTPMS. Not just what we picked, but why, what we considered, and what we ruled out.

---

## MCU + Radio: Raytac MDBT42Q-512KV2 (nRF52832)

**Why this chip:** The nRF52832 is the only SoC that runs ANT+ and BLE simultaneously using Nordic's S332 SoftDevice. That's the core requirement. Every other BLE chip (ESP32, STM32WB, etc.) can do BLE but not ANT+. And ANT+ is how Garmin head units talk to sensors natively.

**Why a module instead of the bare chip:** The bare nRF52832 QFN is only 6x6mm (Outrider uses it), but it requires designing your own PCB antenna, crystal oscillators, and RF matching network, plus FCC/CE/IC certification ($5-10k). The MDBT42Q module is pre-certified and includes all of that. For a v1 prototype, the trade-off is worth it. Yes, it's 10.5x15.5mm (which is why our board is 19mm wide instead of 12mm), but we skip months of RF engineering and thousands in certification costs.

**What we get for free from this chip:**
- NFC-A hardware built in (we added a trace antenna and tuning caps for future firmware use)
- Internal DC-DC buck converter (we added a 10uH inductor, saves 30% radio power)
- SAADC for battery voltage monitoring via internal VDD/3 channel (no external components)
- Hardware watchdog timer (critical for a sealed, unserviceable device)
- 512KB flash / 64KB RAM (more than enough for our firmware + OTA DFU bootloader)

**v2 plan:** Switch to bare nRF52832 QFN. Drops the board width to 12mm. Requires RF design and certification.

---

## Pressure Sensor: TE Connectivity MS5837-30BA

**Why this sensor:** It's a gel-filled absolute pressure sensor with 24-bit resolution, built-in temperature measurement, and I2C interface. The gel filling is important because it protects the MEMS element from moisture and sealant without needing a separate membrane over the sensor itself.

**Why absolute pressure instead of gauge:** The sensor reads pressure relative to vacuum, not relative to atmosphere. That sounds like a problem (you want gauge PSI on your screen), but it's actually better for our architecture. The sensor just transmits the raw absolute value. The Garmin head unit or phone has its own barometer, so it subtracts atmospheric pressure to get gauge. This means:
- No calibration drift from weather changes
- Flat detection works on absolute changes (doesn't need atmospheric reference)
- The sensor firmware is simpler (just read and transmit)

**Why not a gauge sensor:** Gauge sensors measure relative to atmosphere through a reference port on the back of the package. That port would need to be vented to outside air through the sealed enclosure, adding complexity and another failure point. Absolute sensors don't need external venting.

**Accuracy:** The raw sensor is ±50 mbar out of the box. After factory calibration (polynomial correction at 3-5 known pressures), we get it down to ±5-10 mbar, which is under 1% at 20 PSI. This matches the Outrider TL Pro's ±0.5% claim and beats TyreWiz's ±2%.

**Alternatives considered:**
- Honeywell ABP series: our first choice, but discontinued/obsolete during the design phase
- Bosch BMP390: great for weather stations, not rated for the pressure range we need
- Infineon DPS310: good accuracy but gauge-only variants have venting issues

---

## Accelerometer: ST LIS2DH12

**Why we need one:** The sensor needs to know when the wheel is spinning so it can wake up and start transmitting. Without an accelerometer, the MCU would need to stay awake polling the pressure sensor, which would drain the CR1225 in weeks instead of years.

**Why this specific part:** The LIS2DH12 has a hardware motion-detection interrupt that can wake the nRF52832 from deep sleep (System OFF mode, ~3uA). The MCU is completely powered down until the accelerometer's INT1 pin fires. At 1.8uA in low-power mode, the accelerometer barely touches our power budget.

**Why not use the nRF52832's built-in comparator for wake:** The LPCOMP could theoretically detect vibration on an analog pin, but it draws 0.5uA continuously and isn't as reliable as a purpose-built motion detector with configurable thresholds. The LIS2DH12 is the standard choice for this exact use case.

**Alternatives considered:**
- LIS2DW12: newer, lower power, but harder to source and more expensive
- ADXL362: great ultra-low-power accelerometer, but SPI only (we wanted I2C to share the bus with the pressure sensor)

---

## Battery: CR1225

**Why CR1225:** It's 12.5mm diameter and 2.5mm tall, which keeps Block B height at 5.5mm. A CR1632 (what TyreWiz and Outrider use) has 3x the capacity (140mAh vs 48mAh) but is 16mm diameter, which was too wide for our 16mm board target. Even at 19mm, the CR1632 holder footprints were all wider than the board.

**Why 48mAh is enough:** Our power budget is extremely low. Deep sleep draws ~3uA. Active TX (ANT+ + BLE every 3 seconds) averages under 20uA. At 1 hour of riding per day, the CR1225 lasts 2-3 years. That's longer than most people go between sealant refreshes, which is when you'd swap the battery anyway.

**Why replaceable matters:** The Outrider TL Pro has a 5+ year battery, which sounds great until you realize it's potted in epoxy. When it dies, the sensor is e-waste. Our CR1225 costs about $1 and takes 2 minutes to swap. Over 10 years, OpenTPMS costs ~$70 total. An Outrider TL Pro costs $122 every 5 years = $244.

**v2 plan:** Switch to CR1632 in a potted design (like Outrider). The extra capacity combined with ANT+-only mode could push battery life past 5 years. But that's a deliberate choice to make for v2 after we've validated the firmware.

---

## Battery Contact: Custom PCB Pad + Lid Spring

**The saga of battery holders:** This was the most painful component decision in the entire project.

We went through:
1. **MPD BHSD-1225-SM** (holder with snap cover): 4.83mm tall, needed a separate cover part. We realized the enclosure lid already provides retention, so the cover was redundant and wasted height.
2. **Renata SMTM1225** (spring clip): low profile at ~0.5mm, but the footprint was 20mm wide. Didn't fit our 16mm board.
3. **MPD BK-885** (retainer): spec said 13.2x13.2mm, but actual pad-to-pad measurement was 20.1mm. Also didn't fit.
4. **MPD BK-916**: same problem, too wide.

Every commercial CR1225 holder has side contacts that extend to or past the battery diameter (12.5mm), plus pad width, putting them at 16-20mm total. On a 16mm board (now 19mm), nothing fit.

**The solution:** No holder at all. A ~10mm circular ENIG copper pad on the PCB serves as the negative contact (battery sits flat on it). A BeCu leaf spring inside the enclosure lid presses down on the battery top for the positive contact, with a short wire soldered to a VCC pad on the PCB. The lid provides both electrical contact and mechanical retention.

This is how many small commercial devices handle coin cells. The enclosure is the holder.

---

## TX Buffer Capacitor: Murata GRM31CR60J227ME11L (220uF 1206)

**Why it exists:** When the nRF52832 transmits, it draws 8-12mA for a few milliseconds. A CR1225 has high internal resistance, especially in cold weather. Without a buffer cap, the voltage sags during TX and the radio can brown out or transmit with reduced power.

**Why 220uF:** We originally spec'd 100uF, but after DC bias derating (ceramic caps lose capacitance at operating voltage), a 100uF X5R cap at 3V only delivers ~65uF effective. The 220uF part derates to ~143uF, which exceeds our 120uF minimum requirement.

**Why 1206 and not something smaller:** 220uF in a ceramic cap only comes in 1206 or larger. This is the smallest package for this value. It's the biggest passive on the board at 3.2x1.6mm.

**Why ceramic and not a supercapacitor:** Ceramic caps handle the fast transient current (microsecond rise time) that radio TX demands. Supercapacitors have higher ESR and can't deliver current fast enough. The "supercap" label in early design docs was misleading; this was always a ceramic MLCC.

---

## DC-DC Inductor: 10uH 0402

**Why it exists:** The nRF52832 has an internal DC-DC buck converter that replaces the LDO for the 1.3V core supply. Enabling it reduces radio TX current by about 30% (12mA to 8mA on BLE, 8mA to 5.3mA on ANT+). Over 2-3 years of periodic transmissions, that's significant battery life extension.

**Why it almost didn't make the cut:** The DC-DC needs an external inductor between VDD and the DCC pin, which is one more component and one more trace to route. We added it after discovering the MDBT42Q module exposes the DCC pin (pin 10). One 0402 inductor, one trace, 30% power savings. Easy decision.

---

## Enclosure: 3D Printed ABS (not ASA)

**Why ABS over ASA:** Our original spec called for ASA (better UV resistance). Then we realized the sensor lives inside a tire. There's no UV exposure. ABS gives us reliable acetone vapor smoothing for the O-ring groove surfaces, which is critical for gas-tight sealing. ASA is harder to smooth with acetone.

**Why 3D printed and not injection molded:** For a prototype run of 3-50 units, 3D printing is the only sane option. Injection mold tooling costs $3-5k. FDM ABS at 0.1mm layers with 100% infill and acetone smoothing gives us structural, waterproof walls.

**v2 plan:** If production exceeds ~500 units, switch to injection-molded PA66 or ABS.

---

## Screws: M1.4 DIN912 (not M1.2)

**Why M1.4:** Our original spec called for M1.2 Allen screws. Turns out M1.2 hex socket cap screws don't exist as a standard. DIN912 starts at M1.4. We discovered this when trying to source them.

**Why Allen/hex socket:** The spec calls for dabbing silicone grease in the hex socket to prevent tire sealant from curing inside the screw head. Phillips or Torx heads don't have a cavity for this.

**Boss design:** M1.4 heat-set inserts (OD 2.3mm) need at least 5.5mm diameter bosses in the ABS frame for wall integrity. The enclosure walls are 1.2mm everywhere else, but thicken to 5.5mm at the four screw locations.

---

## NFC Antenna: PCB Trace Loop (hardware ready, firmware later)

**Why include it on v1:** The nRF52832 has NFC-A hardware built in, and the MDBT42Q exposes the NFC1/NFC2 pins. Adding NFC capability costs exactly two 150pF 0402 capacitors and a copper trace loop on the PCB bottom layer. Total cost: about $0.02. The firmware can enable it later via OTA update.

**What it enables:** Tap your phone against the sensor before installing in the tire. The phone reads an NDEF record with the sensor's serial number and auto-pairs via BLE. Same workflow as the SRAM TyreWiz. Useful for initial setup when you have multiple sensors.

**Why not enable it in v1 firmware:** One thing at a time. Get the core pressure sensing, BLE, and ANT+ working first. NFC pairing is a nice-to-have that can be added in a firmware update without touching hardware.

---

## Conformal Coating: MG Chemicals 422C (brush, not spray)

**Why brush instead of spray (422B):** The PCB strip has a small exposed section between the two enclosure blocks. A brush gives precise application on this narrow area without overspray on the enclosure bonding surfaces (which need to be bare for RTV adhesion). The 55mL bottle will coat hundreds of sensors.

**Why silicone-based:** Silicone conformal coating is flexible (survives vibration), chemically inert to tire sealant, and works across our full temperature range (-30C to +60C). The UV indicator lets you check coverage under a blacklight before sealing the enclosure.

---

## What We Considered and Rejected

| Component | Why rejected |
|-----------|-------------|
| ESP32 | No ANT+ support |
| STM32WB | No ANT+ support |
| Bare nRF52832 QFN | Requires RF design + FCC certification (v2) |
| Honeywell ABP pressure sensor | Discontinued |
| Gauge pressure sensor | Needs external vent port through sealed enclosure |
| CR1632 battery | 16mm diameter too wide for PCB |
| Any commercial CR1225 holder | All footprints wider than 16-19mm board |
| ASA enclosure | No UV in tire, ABS acetone smooths better |
| M1.2 screws | Don't exist as hex socket (DIN912) |
| Injection molded enclosure | Too expensive for prototype quantities |
| Supercapacitor | Too slow (high ESR) for radio TX transients |

---

*This document reflects decisions made during the v1 design process (Rev 1-8). Some choices were made, unmade, and remade multiple times as we discovered real-world constraints. That's prototyping.*
