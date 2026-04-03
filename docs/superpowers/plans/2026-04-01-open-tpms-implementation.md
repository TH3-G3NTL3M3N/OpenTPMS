# OpenTPMS Implementation Plan (Rev 2)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build an open-source bicycle tire pressure sensor with replaceable CR1225 battery, dual ANT+/BLE, ±1% accuracy at 20 PSI (factory-calibrated), and temperature-compensated flat detection. Plug and play — no user calibration.

**Architecture:** Two-block symmetric hardware (electronics + battery) on a PCB strip that mounts on a tubeless valve stem. nRF52832 (via Raytac MDBT42Q module) runs firmware in C using Nordic nRF5 SDK with S332 SoftDevice for concurrent BLE + ANT+. Sensor transmits absolute pressure in mbar — Garmin Connect IQ data field or phone app reads its own barometer and subtracts atmospheric to display gauge PSI. 3D printed ASA enclosures with frame-on-PCB construction.

**Tech Stack:** KiCad 8 (PCB), Nordic nRF5 SDK 17.1 + S332 SoftDevice (firmware), C + GCC ARM (language/compiler), Monkey C (Connect IQ data field), FreeCAD or Fusion 360 (enclosure CAD), HTML/JS + Web Bluetooth API + Web Sensor API (config tool), Python (factory calibration tool)

**Rev 2 changes:** Pressure sensor changed from Honeywell ABP (obsolete) to TE MS5837-30BA. All pressure values are now absolute mbar. Added Connect IQ data field and factory calibration tool tasks.

---

## Project Structure

```
bike-tpms-project/
├── docs/
├── hardware/
│   ├── kicad/                 # KiCad PCB project
│   └── enclosure/             # 3D print STL/STEP files
├── firmware/
│   ├── Makefile
│   ├── config/
│   │   ├── sdk_config.h       # Nordic SDK configuration
│   │   └── app_config.h       # Application-specific config
│   ├── src/
│   │   ├── main.c             # Entry point, state machine
│   │   ├── ms5837.c           # TE MS5837-30BA I2C driver
│   │   ├── ms5837.h
│   │   ├── calibration.c      # Factory calibration polynomial
│   │   ├── calibration.h
│   │   ├── motion.c           # LIS2DH12 accelerometer driver
│   │   ├── motion.h
│   │   ├── alert.c            # Temperature-compensated flat detection
│   │   ├── alert.h
│   │   ├── ble_tpms.c         # BLE GATT custom service
│   │   ├── ble_tpms.h
│   │   ├── ant_tpms.c         # ANT+ tire pressure profile
│   │   ├── ant_tpms.h
│   │   ├── storage.c          # NVS config persistence
│   │   └── storage.h
│   ├── tests/
│   │   ├── test_ms5837.c      # Unit tests for MS5837 conversion
│   │   ├── test_alert.c       # Unit tests for alert engine
│   │   ├── test_calibration.c # Unit tests for calibration polynomial
│   │   └── Makefile
│   └── dfu/
├── garmin/
│   ├── manifest.xml           # Connect IQ app manifest
│   ├── source/
│   │   └── OpenTPMSField.mc   # Data field source (Monkey C)
│   └── resources/
│       └── strings.xml
├── web-config/
│   ├── index.html
│   ├── app.js                 # BLE + phone barometer
│   └── style.css
├── tools/
│   └── calibrate.py           # Factory calibration script
├── .gitignore
├── LICENSE-HARDWARE
├── LICENSE-SOFTWARE
└── README.md
```

---

## Phase 1: Project Setup & Firmware Foundation

### Task 1: Initialize Project Repository and Toolchain

**Files:** `.gitignore`, `LICENSE-HARDWARE`, `LICENSE-SOFTWARE`, `README.md`, `firmware/Makefile`, `firmware/config/sdk_config.h`, `firmware/config/app_config.h`

- [ ] **Step 1:** Create .gitignore (include garmin/bin/, tools/__pycache__/)
- [ ] **Step 2:** Create license files (CERN OHL v2 + MIT)
- [ ] **Step 3:** Create README.md
- [ ] **Step 4:** Install Nordic nRF5 SDK 17.1 + GCC ARM toolchain
- [ ] **Step 5:** Create firmware Makefile
- [ ] **Step 6:** Create sdk_config.h (enable BLE, ANT+, TWI, GPIOTE, FDS, timers)

- [ ] **Step 7: Create app_config.h**

```c
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// Pin Assignments (MDBT42Q)
#define PIN_I2C_SCL          27
#define PIN_I2C_SDA          26
#define PIN_ACCEL_INT1       11

// MS5837-30BA Pressure Sensor
#define MS5837_I2C_ADDR      0x76
#define MS5837_CMD_RESET     0x1E
#define MS5837_CMD_CONV_D1   0x48  // OSR=4096
#define MS5837_CMD_CONV_D2   0x58  // OSR=4096
#define MS5837_CMD_ADC_READ  0x00
#define MS5837_CMD_PROM_BASE 0xA0
#define MS5837_CONV_TIME_MS  10

// LIS2DH12 Accelerometer
#define ACCEL_I2C_ADDR       0x19
#define MOTION_THRESHOLD     16
#define MOTION_DURATION      0

// Timing
#define TX_INTERVAL_DEFAULT_S  3
#define TX_INTERVAL_MIN_S      1
#define TX_INTERVAL_MAX_S      10
#define SLEEP_TIMEOUT_S        300

// Alert (threshold in 0.01 mbar; 34500 = 345 mbar ≈ 5 PSI)
#define ALERT_THRESHOLD_DEFAULT_MBAR  34500
#define ALERT_COMPENSATION_ENABLED    1

// Factory Calibration
#define CAL_MAX_COEFFICIENTS  4
#define CAL_FLASH_FILE_ID     0x0002
#define CAL_FLASH_REC_KEY     0x0001

// BLE
#define DEVICE_NAME            "OpenTPMS"
#define APP_BLE_CONN_CFG_TAG   1
#define APP_ADV_INTERVAL       MSEC_TO_UNITS(1000, UNIT_0_625_MS)
#define APP_ADV_DURATION       0

// ANT+
#define ANT_TPMS_CHANNEL      0
#define ANT_TPMS_NETWORK      0
#define ANT_TPMS_FREQ         57
#define ANT_TPMS_PERIOD       8192

#endif
```

- [ ] **Step 8:** Commit

---

### Task 2: MS5837-30BA Pressure Sensor Driver

**Files:** `firmware/src/ms5837.h`, `firmware/src/ms5837.c`, `firmware/tests/test_ms5837.c`, `firmware/tests/Makefile`

The MS5837-30BA uses a completely different protocol from the old ABP: reset → read 6 PROM calibration coefficients → trigger D1/D2 conversions → read 24-bit ADC → apply compensation algorithm with second-order temperature correction. Reference: TE Connectivity official C driver.

- [ ] **Step 1: Create ms5837.h** — types: `pressure_mbar_t` (int32, 0.01 mbar), `temp_centideg_t` (int32, 0.01°C), `ms5837_reading_t`, `ms5837_prom_t`. Functions: `ms5837_init()`, `ms5837_read()`, `ms5837_calculate_pressure()`, `ms5837_calculate_temperature()`.

- [ ] **Step 2: Create test Makefile** — builds host-side tests with `-DUNIT_TEST -lm`

- [ ] **Step 3: Write test_ms5837.c** — tests conversion math with known PROM values: temperature range, pressure monotonicity, atmospheric pressure ballpark

- [ ] **Step 4: Run tests** — verify they fail (not implemented)

- [ ] **Step 5: Implement ms5837.c** — I2C: reset (0x1E) + PROM read (0xA0-0xAE) + D1/D2 convert + ADC read. Math: dT, TEMP, OFF, SENS, P with second-order compensation. Output: pressure in 0.01 mbar, temperature in 0.01°C.

Key conversion (from TE datasheet):
```c
dT = D2 - (C5 << 8)
TEMP = 2000 + (dT * C6) >> 23
OFF = (C2 << 16) + (C4 * dT) >> 7
SENS = (C1 << 15) + (C3 * dT) >> 8
P = ((D1 * SENS >> 21) - OFF) >> 13
// Second-order when TEMP < 2000
```

- [ ] **Step 6: Run tests** — all pass
- [ ] **Step 7: Commit**

---

### Task 3: Factory Calibration Module

**Files:** `firmware/src/calibration.h`, `firmware/src/calibration.c`, `firmware/tests/test_calibration.c`

Applies a polynomial correction to the MS5837's compensated output, using coefficients written during factory calibration. Stored in nRF52 flash (FDS).

- [ ] **Step 1: Create calibration.h** — `cal_data_t` (4 float coefficients + valid flag), `cal_init()`, `cal_apply()`, `cal_store()`, `cal_poly_correct()`
- [ ] **Step 2: Write test_calibration.c** — identity, offset, gain, passthrough tests
- [ ] **Step 3: Implement calibration.c** — polynomial eval + FDS load/store
- [ ] **Step 4: Run tests** — all pass
- [ ] **Step 5: Commit**

---

### Task 4: Temperature-Compensated Alert Engine

**Files:** `firmware/src/alert.h`, `firmware/src/alert.c`, `firmware/tests/test_alert.c`

Works with absolute mbar — flat detection uses pressure *changes* which are identical in absolute and gauge. Ideal gas law: P2 = P1 * T2/T1.

- [ ] **Step 1: Create alert.h** — all types use `pressure_mbar_t` and `temp_centideg_t`. Threshold in mbar (34500 = ~5 PSI).
- [ ] **Step 2: Write test_alert.c** — test values in absolute mbar (e.g., 294400 = ~28 PSI gauge at sea level). Test: same temp, temp increase, temp decrease, no flat, temp-explained drop, real flat, no baseline.
- [ ] **Step 3: Implement alert.c** — same ideal gas law math, mbar units
- [ ] **Step 4: Run tests** — all pass
- [ ] **Step 5: Commit**

---

### Task 5: Accelerometer Motion Driver (LIS2DH12)

**Files:** `firmware/src/motion.h`, `firmware/src/motion.c`

No changes from original design. LIS2DH12 at I2C 0x19, motion-detect interrupt on GPIO 11.

- [ ] **Step 1: Create motion.h** — `motion_init()`, `motion_enable()`, `motion_disable()`, `motion_is_active()`
- [ ] **Step 2: Implement motion.c** — configure LIS2DH12 low-power mode, GPIOTE interrupt handler
- [ ] **Step 3: Commit**

---

### Task 6: Non-Volatile Storage

**Files:** `firmware/src/storage.h`, `firmware/src/storage.c`

- [ ] **Step 1: Create storage.h** — `storage_config_t` with `pressure_mbar_t` types for threshold/baseline
- [ ] **Step 2: Implement storage.c** — FDS load/save, defaults with mbar threshold
- [ ] **Step 3: Commit**

---

### Task 7: BLE Custom GATT Service

**Files:** `firmware/src/ble_tpms.h`, `firmware/src/ble_tpms.c`

Transmits **absolute pressure** in 0.01 mbar (int32). Phone app subtracts atmospheric.

- [ ] **Step 1: Create ble_tpms.h** — characteristics: pressure (int32 notify), temperature (int32 notify), location (uint8 r/w), threshold (int32 r/w), tx_interval (uint8 r/w), cal_status (uint8 read), cal_coefficients (16 bytes write — for factory tool)
- [ ] **Step 2: Implement ble_tpms.c** — GATT service with int32 pressure/temp, write handler for cal coefficients stores to flash via `cal_store()`
- [ ] **Step 3: Commit**

---

### Task 8: ANT+ Tire Pressure Profile

**Files:** `firmware/src/ant_tpms.h`, `firmware/src/ant_tpms.c`

Broadcasts **absolute pressure** in 0.01 mbar. Connect IQ data field on Garmin handles gauge conversion.

- [ ] **Step 1: Create ant_tpms.h** — `ant_tpms_update(pressure_mbar_t, temp_centideg_t, location, battery)`
- [ ] **Step 2: Implement ant_tpms.c** — data page: byte 0 = page#, byte 1 = location, bytes 2-5 = absolute mbar (int32 LE), bytes 6-7 = temp (int16 LE)
- [ ] **Step 3: Commit**

---

### Task 9: Main Application — State Machine

**Files:** `firmware/src/main.c`

- [ ] **Step 1: Implement main.c**

Init sequence: power mgmt → timers → BLE stack → TWI → storage → cal_init → ms5837_init → alert_init → ant_tpms_init → motion_init → deep sleep.

TX cycle:
```c
ms5837_reading_t reading = ms5837_read();
pressure_mbar_t calibrated = cal_apply(reading.pressure);
ble_tpms_pressure_update(&m_ble_tpms, calibrated);
ant_tpms_update(calibrated, reading.temperature, location, battery);
alert_check_flat(&m_alert_config, calibrated, reading.temperature);
```

State machine: DEEP_SLEEP → WAKING → ACTIVE → SLEEPING → DEEP_SLEEP (same logic as original).

- [ ] **Step 2: Commit**

---

## Phase 2: Display & Configuration

### Task 10: Garmin Connect IQ Data Field

**Files:** `garmin/manifest.xml`, `garmin/source/OpenTPMSField.mc`, `garmin/resources/strings.xml`

This is the primary user interface. Reads sensor absolute pressure via ANT+, reads Garmin's `Activity.Info.rawAmbientPressure`, displays gauge PSI. No phone needed.

- [ ] **Step 1: Create manifest.xml** — data field type, API Level 2.4.0+, Sensor + Ant permissions, Edge/Fenix/Forerunner targets
- [ ] **Step 2: Create OpenTPMSField.mc** — ANT+ channel listener for sensor data page, `compute()` reads `info.rawAmbientPressure` (Pascals), computes `gauge_psi = (tire_abs - atm) / 100 * 0.0145038`
- [ ] **Step 3: Create strings.xml**
- [ ] **Step 4: Build + test in Connect IQ simulator**
- [ ] **Step 5: Commit**

---

### Task 11: Web Bluetooth Configuration Page

**Files:** `web-config/index.html`, `web-config/app.js`, `web-config/style.css`

Phone app reads sensor absolute mbar via BLE, reads phone barometer via Web Sensor API (`Barometer` interface), displays gauge PSI.

- [ ] **Step 1: Create index.html**
- [ ] **Step 2: Create app.js** — BLE connect, read int32 pressure, read phone barometer (`new Barometer()`), compute gauge = absolute - atmospheric, display PSI
- [ ] **Step 3: Create style.css**
- [ ] **Step 4: Test locally, commit**

---

## Phase 3: Hardware Design

### Task 12: KiCad Schematic

Components: MDBT42Q-512KV2, **MS5837-30BA** (3.3x3.3mm LGA-8, I2C 0x76), LIS2DH12TR (I2C 0x19), FC0H473ZFTBR24 (supercap), BHSD-1225-SM (CR1225 holder), passives (100nF 0402, 4.7kΩ 0402, 100Ω 0402), SWD test pads.

- [ ] **Step 1-5:** Create project, place components, wire (I2C bus, power, interrupts), ERC, commit

### Task 13: KiCad PCB Layout

- [ ] **Step 1-6:** Board outline (16x40mm, valve hole), place components (MS5837 in Block B, MDBT42Q in Block A with antenna at board edge), route, DRC, generate Gerbers, commit

### Task 14: Enclosure CAD Design

- [ ] **Step 1-6:** Frame (walls only, 1.2mm ASA), standard lid (Block A), pressure lid with ePTFE port (Block B), export STL, test print, commit

---

## Phase 4: Integration & Testing

### Task 15: Factory Calibration Tool

**Files:** `tools/calibrate.py`

Python script using `bleak` (BLE) + `numpy` (polynomial fit).

- [ ] **Step 1: Create calibrate.py** — scan for sensor, connect via BLE, prompt operator for 3-5 known pressures, read sensor at each point, compute 3rd-degree polynomial fit, write coefficients to sensor via BLE cal_coefficients characteristic, verify
- [ ] **Step 2: Install deps** — `pip install bleak numpy`
- [ ] **Step 3: Commit**

### Task 16: Order, Assemble, and Test

- [ ] **Step 1:** Order components per BOM
- [ ] **Step 2:** Order PCBs from JLCPCB
- [ ] **Step 3:** Solder PCBs (MDBT42Q, **MS5837-30BA**, LIS2DH12, supercap, holder, passives)
- [ ] **Step 4:** Flash SoftDevice S332 + application firmware
- [ ] **Step 5:** Verify basic operation (nRF Connect → "OpenTPMS" → pressure reads ~101300 at atmospheric)
- [ ] **Step 6:** Run factory calibration (`python3 tools/calibrate.py`)
- [ ] **Step 7:** Assemble enclosure (conformal coat, frames, ePTFE, O-rings, battery, lids)
- [ ] **Step 8:** Test with Garmin (Connect IQ data field, gauge PSI display, ride test)
- [ ] **Step 9:** Run all 8 test procedures from design spec
- [ ] **Step 10:** Commit test results
