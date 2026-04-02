# OpenTPMS — Bill of Materials (BOM)

Build qty: 10 sensors (5 pairs)

---

## Electronics — Core Components

| # | Component | Part Number | Supplier | Qty | Unit Price | Subtotal | Notes |
|---|-----------|-------------|----------|-----|-----------|----------|-------|
| 1 | nRF52 BLE+ANT+ Module | MDBT42Q-512KV2 | [DigiKey](https://www.digikey.com/en/products/detail/raytac/MDBT42Q-512KV2/13677592) | 12 | $4.95 | $59.40 | 10 + 2 spares. Also on [LCSC](https://www.lcsc.com/product-detail/Bluetooth-Modules_RAYTAC-MDBT42Q-512KV2_C2828282.html) at $5.96 |
| 2 | Pressure Sensor (150 PSI, I2C) | ABPMANT150PGAA3 | [DigiKey](https://www.digikey.com/en/products/detail/honeywell-sensing-and-productivity-solutions/ABPMANT150PGAA3/6820591) | 12 | ~$8-12 | ~$96-144 | Honeywell ABP series, I2C, gauge, DIP-8. Check stock — alt: SSCDANT150PGAA5 |
| 3 | 3-Axis Accelerometer | LIS2DH12TR | [DigiKey](https://www.digikey.com/en/products/detail/stmicroelectronics/LIS2DH12TR/4899892) | 12 | $1.38 | $16.56 | ST Micro, LGA-12 (2x2x1mm). 75k+ in stock |
| 4 | Supercapacitor 47mF | BZ015B473ZNB | [DigiKey](https://www.digikey.com/product-detail/en/avx-corporation/BZ154B473ZNB/478-3520-ND/1014115) | 12 | ~$0.50-1.00 | ~$6-12 | AVX BestCap, 3.6V, ultra-low ESR. Any 47-100mF BestCap BZ series works |
| 5 | CR1225 Battery Holder (SMD) | BHSD-1225-SM | [DigiKey](https://www.digikey.com/en/product-highlight/m/mpd/cr1225-battery-holder-with-latching-cover) | 12 | $1.52 | $18.24 | MPD, SMD mount. Alt: Keystone 12mm coin cell retainer (~$0.40) |
| 6 | 100nF Ceramic Caps (0402) | CL05B104KO5NNNC | DigiKey / LCSC | 50 | $0.01 | $0.50 | Samsung MLCC, 16V. Any 100nF 0402 X7R works |
| 7 | 4.7kΩ Resistors (0402) | RC0402FR-074K7L | DigiKey / LCSC | 30 | $0.01 | $0.30 | I2C pull-ups. Any 0402 4.7kΩ 1% |
| 8 | 100Ω Resistors (0402) | RC0402FR-07100RL | DigiKey / LCSC | 20 | $0.01 | $0.20 | Supercap charge-limiting resistor |

**Electronics subtotal: ~$197-251**

---

## Batteries

| # | Component | Part Number | Supplier | Qty | Unit Price | Subtotal | Notes |
|---|-----------|-------------|----------|-----|-----------|----------|-------|
| 9 | CR1225 Coin Cell | Panasonic CR1225 | DigiKey / Amazon | 20 | $0.50-1.00 | $10-20 | Industrial grade preferred. 10 installed + 10 spares |

---

## PCB Fabrication

| # | Component | Spec | Supplier | Qty | Price | Notes |
|---|-----------|------|----------|-----|-------|-------|
| 10 | Custom PCB | 2-layer, 0.8mm FR4, ENIG, ~16x40mm | [JLCPCB](https://jlcpcb.com) / [PCBWay](https://pcbway.com) | 10 (min 5) | ~$5-10 total | Upload Gerbers after KiCad layout done. Optional: add SMT assembly (~$30-50 setup) |

---

## Mechanical — Enclosure Hardware

| # | Component | Description | Supplier | Qty | Price | Notes |
|---|-----------|-------------|----------|-----|-------|-------|
| 11 | M1.2 x 3mm Allen Screws | Stainless steel hex socket head | Amazon / AliExpress | 50 | ~$8-12 (kit) | Often sold in M1-M1.6 assortment kits |
| 12 | M1.2 Heat-Set Inserts | Brass knurled threaded inserts | [Amazon (ZWMSSLL kit)](https://www.amazon.com/Zwmssll-Threaded-Inserts-Assortment-Printing/dp/B0D3LF938N) | 50+ | ~$10-15 (1200pc assort.) | Kit includes M1, M1.2, M1.4, M1.6 sizes |
| 13 | Silicone O-Rings | ~12mm OD x ~10mm ID x 1mm cross-section | Amazon / AliExpress | 30 | ~$5-8 (assort.) | Buy silicone O-ring assortment kit. Exact size depends on final enclosure dims |
| 14 | ePTFE Vent Membrane | IP67 adhesive waterproof breathable disc | AliExpress / Amazon | 20 discs | ~$5-15 | Search "ePTFE waterproof vent adhesive" or "IP67 breathable membrane". 5-8mm adhesive discs. Alt: buy ePTFE sheet + punch 4mm discs |
| 15 | ASA Filament (1.75mm) | 1kg spool, black | Amazon | 1 | ~$25 | eSUN, Polymaker, or Prusament ASA. ~20g needed for 10 sensors |

**Mechanical subtotal: ~$53-75**

---

## Assembly Consumables

| # | Component | Part | Supplier | Qty | Price | Notes |
|---|-----------|------|----------|-----|-------|-------|
| 16 | Silicone Conformal Coating | MG Chemicals 422B spray | Amazon / DigiKey | 1 can | ~$15 | Spray can. One can lasts hundreds of units |
| 17 | RTV Silicone Adhesive | Permatex Clear RTV or Loctite SI 5910 | Amazon / hardware store | 1 tube | ~$8 | Clear, flexible. Bonds frame to PCB |
| 18 | Threadlocker | Loctite 222 (low-strength, purple) | Amazon | 1 bottle | ~$10 | Small bottle lasts forever at M1.2 scale |
| 19 | Silicone Grease | Super Lube silicone grease | Amazon | 1 tube | ~$6 | For screw hex sockets + O-ring lubrication |

**Consumables subtotal: ~$39**

---

## Dev Tools & Test Equipment (One-Time Purchase)

| # | Tool | Part | Supplier | Price | What it does |
|---|------|------|----------|-------|-------------|
| 20 | nRF52-DK Dev Board | PCA10040 | [DigiKey](https://www.digikey.com/en/product-highlight/n/nordic-semi/nrf52-development-kit) / [Nordic](https://www.nordicsemi.com/Products/Development-hardware/nRF52-DK) | ~$40-80 | Programs/debugs custom PCBs via built-in J-Link. Develop firmware before PCBs arrive |
| 21 | Power Profiler Kit II | NRF-PPK2 | [DigiKey](https://www.digikey.com/en/products/detail/nordic-semiconductor-asa/NRF-PPK2/13557476) | ~$90 | Measures nanoamp-level current draw. Validates battery life |
| 22 | Digital Pressure Gauge | Topeak SmartGauge D2 or similar | Amazon | ~$30 | Reference standard for accuracy testing |

**Dev tools subtotal: ~$160-200**

---

## Total Cost Summary

| Category | Cost |
|----------|------|
| Electronics (10 sensors + spares) | ~$197-251 |
| Batteries | ~$10-20 |
| PCBs | ~$5-10 |
| Mechanical (enclosure hardware) | ~$53-75 |
| Assembly consumables | ~$39 |
| **Recurring total (builds 10 sensors)** | **~$304-395** |
| Dev tools (one-time) | ~$160-200 |
| **Grand total (first build)** | **~$464-595** |

Per-sensor component cost (after dev tools): **~$16-23**

---

## Order Priority

### Order FIRST (longest lead time)
- [ ] MDBT42Q-512KV2 modules (DigiKey) — sometimes 1-2 week lead
- [ ] Honeywell ABP pressure sensor (DigiKey) — check stock, can have long lead times
- [ ] nRF52-DK dev board (DigiKey/Nordic) — needed for firmware dev while waiting for PCBs
- [ ] Power Profiler Kit II (DigiKey)

### Order AFTER KiCad schematic is done
- [ ] PCBs from JLCPCB — need final Gerber files first (5-10 day lead time)

### Order ANYTIME (always in stock, fast shipping)
- [ ] LIS2DH12TR accelerometer — 75k+ in stock
- [ ] Supercapacitors, battery holders, passives (caps, resistors)
- [ ] CR1225 batteries
- [ ] M1.2 screws + heat-set inserts
- [ ] O-rings, ePTFE membrane, ASA filament
- [ ] All consumables (conformal coat, RTV, Loctite, grease)
- [ ] Digital pressure gauge
