# OpenTPMS Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build an open-source bicycle tire pressure sensor with replaceable CR1225 battery, dual ANT+/BLE, and temperature-compensated flat detection.

**Architecture:** Two-block symmetric hardware (electronics + battery) on a PCB strip that mounts on a tubeless valve stem. nRF52832 (via Raytac MDBT42Q module) runs firmware in C using Nordic nRF5 SDK with S332 SoftDevice for concurrent BLE + ANT+. Web Bluetooth page for v1 configuration. 3D printed ASA enclosures with frame-on-PCB construction.

**Tech Stack:** KiCad 8 (PCB), Nordic nRF5 SDK 17.1 + S332 SoftDevice (firmware), C + GCC ARM (language/compiler), FreeCAD or Fusion 360 (enclosure CAD), HTML/JS + Web Bluetooth API (config tool)

---

## Project Structure

```
bike-tpms-project/
├── docs/
│   └── superpowers/
│       ├── specs/
│       │   └── 2026-04-01-open-tpms-design.md
│       └── plans/
│           └── 2026-04-01-open-tpms-implementation.md
├── hardware/
│   ├── kicad/
│   │   ├── open-tpms.kicad_pro
│   │   ├── open-tpms.kicad_sch
│   │   ├── open-tpms.kicad_pcb
│   │   ├── symbols/          # Custom symbol library
│   │   ├── footprints/       # Custom footprint library
│   │   └── fabrication/      # Gerbers, BOM, pick-and-place
│   └── enclosure/
│       ├── frame.step         # Enclosure frame (no floor)
│       ├── lid.step           # Screw-down lid with O-ring groove
│       ├── lid-pressure.step  # Block B lid variant with ePTFE port
│       └── stl/               # Export STL for 3D printing
├── firmware/
│   ├── CMakeLists.txt
│   ├── Makefile
│   ├── config/
│   │   ├── sdk_config.h       # Nordic SDK configuration
│   │   └── app_config.h       # Application-specific config
│   ├── src/
│   │   ├── main.c             # Entry point, state machine
│   │   ├── pressure.c         # Honeywell ABP I2C driver
│   │   ├── pressure.h
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
│   │   ├── test_alert.c       # Unit tests for alert engine
│   │   ├── test_pressure.c    # Unit tests for pressure conversion
│   │   └── Makefile           # Host-side test build (no hardware)
│   └── dfu/
│       ├── private_key.pem    # DFU signing key (gitignored)
│       └── dfu_settings.hex   # Bootloader settings
├── web-config/
│   ├── index.html             # Single-page Web Bluetooth config tool
│   ├── app.js                 # BLE connection + GATT operations
│   └── style.css              # Minimal styling
├── .gitignore
├── LICENSE-HARDWARE            # CERN OHL v2
├── LICENSE-SOFTWARE            # MIT
└── README.md
```

---

## Phase 1: Project Setup & Firmware Foundation

### Task 1: Initialize Project Repository and Toolchain

**Files:**
- Create: `.gitignore`
- Create: `LICENSE-HARDWARE`
- Create: `LICENSE-SOFTWARE`
- Create: `README.md`
- Create: `firmware/Makefile`
- Create: `firmware/config/sdk_config.h`
- Create: `firmware/config/app_config.h`

- [ ] **Step 1: Create .gitignore**

```gitignore
# Build artifacts
firmware/_build/
firmware/*.hex
firmware/*.bin
firmware/*.elf
firmware/*.map

# DFU keys
firmware/dfu/private_key.pem

# KiCad backups
hardware/kicad/*-backups/
hardware/kicad/*.kicad_sch-bak
hardware/kicad/fp-info-cache

# 3D printing sliced files
hardware/enclosure/stl/*.gcode

# IDE
.vscode/
*.swp
*.swo
.DS_Store

# Superpowers brainstorm sessions
.superpowers/
```

- [ ] **Step 2: Create license files**

Create `LICENSE-HARDWARE` with CERN Open Hardware Licence v2 — Permissive text from https://ohwr.org/cern_ohl_p_v2.txt

Create `LICENSE-SOFTWARE` with MIT License, copyright holder: `simruelland`

- [ ] **Step 3: Create initial README.md**

```markdown
# OpenTPMS

Open-source bicycle tire pressure monitoring sensor.

- Replaceable CR1225 battery (2-3 year life)
- Dual ANT+ / BLE (nRF52832)
- Temperature-compensated flat detection
- -30°C to +60°C operating range
- Fits 19-30mm internal width tubeless rims
- ~6.5g per sensor

## Project Status

🚧 In development

## Documentation

- [Design Spec](docs/superpowers/specs/2026-04-01-open-tpms-design.md)

## License

- Hardware: CERN Open Hardware Licence v2 — Permissive
- Software: MIT
```

- [ ] **Step 4: Install Nordic nRF5 SDK**

Download nRF5 SDK 17.1.0 from Nordic's website (or use `nRF Connect SDK` via `west init`). For nRF5 SDK approach:

```bash
# Download and extract SDK (adjust path as needed)
cd ~/
wget https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/sdks/nrf5/binaries/nrf5_sdk_17.1.0_ddde560.zip
unzip nrf5_sdk_17.1.0_ddde560.zip -d nrf5_sdk_17.1.0

# Install GCC ARM toolchain
brew install gcc-arm-none-eabi

# Set SDK path
export NRF5_SDK_PATH=~/nrf5_sdk_17.1.0
```

Verify toolchain:
```bash
arm-none-eabi-gcc --version
```
Expected: version 10.x or newer

- [ ] **Step 5: Create firmware Makefile**

Create `firmware/Makefile`:

```makefile
PROJECT_NAME     := open_tpms
TARGETS          := nrf52832_xxaa
OUTPUT_DIRECTORY := _build

# SDK and toolchain paths
SDK_ROOT ?= $(NRF5_SDK_PATH)
PROJ_DIR := .

# Source files
SRC_FILES += \
  $(PROJ_DIR)/src/main.c \
  $(PROJ_DIR)/src/pressure.c \
  $(PROJ_DIR)/src/motion.c \
  $(PROJ_DIR)/src/alert.c \
  $(PROJ_DIR)/src/ble_tpms.c \
  $(PROJ_DIR)/src/ant_tpms.c \
  $(PROJ_DIR)/src/storage.c \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52.S \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/timer/app_timer2.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_default_backends.c \
  $(SDK_ROOT)/components/libraries/fds/fds.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage_sd.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ant.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
  $(SDK_ROOT)/components/ble/common/ble_advdata.c \
  $(SDK_ROOT)/components/ble/common/ble_srv_common.c \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
  $(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c \
  $(SDK_ROOT)/components/ant/ant_channel_config/ant_channel_config.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_twi.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_twi.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c \

# Include paths
INC_FOLDERS += \
  $(PROJ_DIR)/config \
  $(PROJ_DIR)/src \
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/components/softdevice/s332/headers \
  $(SDK_ROOT)/components/softdevice/s332/headers/nrf52 \
  $(SDK_ROOT)/components/softdevice/common \
  $(SDK_ROOT)/components/ble/common \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt \
  $(SDK_ROOT)/components/ble/ble_advertising \
  $(SDK_ROOT)/components/ant/ant_channel_config \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/libraries/fds \
  $(SDK_ROOT)/components/libraries/fstorage \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/components/libraries/atomic_fifo \
  $(SDK_ROOT)/components/libraries/sortlist \
  $(SDK_ROOT)/integration/nrfx \
  $(SDK_ROOT)/integration/nrfx/legacy \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/modules/nrfx/mdk \
  $(SDK_ROOT)/modules/nrfx/hal \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/external/fprintf \

# Libraries
LIB_FILES += \

# C flags
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DBOARD_CUSTOM
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DS332
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DANT_STACK_SUPPORT_REQD
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -Wall -Werror
CFLAGS += -O3 -g3

# Linker flags
LDFLAGS += -mthumb -mabi=aapcs -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -Wl,--gc-sections
LDFLAGS += --specs=nano.specs

# Linker script (SoftDevice S332)
LINKER_SCRIPT := $(SDK_ROOT)/config/nrf52832/armgcc/generic_gcc_nrf52.ld

include $(SDK_ROOT)/components/toolchain/gcc/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))
```

- [ ] **Step 6: Create sdk_config.h**

Create `firmware/config/sdk_config.h` — this is a large Nordic config file. Start from the SDK example `examples/ble_peripheral/ble_app_template/pca10040/s132/config/sdk_config.h` and modify:

Key settings to enable:
```c
// In sdk_config.h, ensure these are set:
#define NRF_SDH_ENABLED 1
#define NRF_SDH_BLE_ENABLED 1
#define NRF_SDH_ANT_ENABLED 1
#define NRF_SDH_SOC_ENABLED 1
#define NRFX_TWI_ENABLED 1          // I2C for pressure sensor + accelerometer
#define NRFX_TWI0_ENABLED 1
#define NRF_DRV_TWI_ENABLED 1
#define NRFX_GPIOTE_ENABLED 1       // GPIO interrupt for accelerometer wake
#define APP_TIMER_ENABLED 1
#define FDS_ENABLED 1               // Flash data storage for config
#define NRF_FSTORAGE_ENABLED 1
#define BLE_ADVERTISING_ENABLED 1
#define NRF_BLE_GATT_ENABLED 1
```

- [ ] **Step 7: Create app_config.h**

Create `firmware/config/app_config.h`:

```c
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// --- Pin Assignments (MDBT42Q GPIO mapping) ---
#define PIN_I2C_SCL          27
#define PIN_I2C_SDA          26
#define PIN_ACCEL_INT1       11    // LIS2DH12 interrupt 1 (motion wake)

// --- Pressure Sensor ---
#define ABP_I2C_ADDR         0x28  // Honeywell ABP default I2C address
#define PRESSURE_MAX_PSI     150   // Sensor full-scale range
#define PRESSURE_MIN_PSI     0

// --- Accelerometer ---
#define ACCEL_I2C_ADDR       0x19  // LIS2DH12 default (SDO/SA0 high)
#define MOTION_THRESHOLD     16    // Motion detect threshold (mg)
#define MOTION_DURATION      0     // Minimum motion duration (samples)

// --- Timing ---
#define TX_INTERVAL_DEFAULT_S  3   // Default broadcast interval (seconds)
#define TX_INTERVAL_MIN_S      1
#define TX_INTERVAL_MAX_S      10
#define SLEEP_TIMEOUT_S        300 // 5 minutes no motion -> deep sleep

// --- Alert ---
#define ALERT_THRESHOLD_DEFAULT_PSI  200  // 20.0 PSI (stored as 0.1 PSI units)
#define ALERT_COMPENSATION_ENABLED   1

// --- BLE ---
#define DEVICE_NAME            "OpenTPMS"
#define APP_BLE_CONN_CFG_TAG   1
#define APP_ADV_INTERVAL       MSEC_TO_UNITS(1000, UNIT_0_625_MS)
#define APP_ADV_DURATION       0   // Advertise indefinitely when active

// --- ANT+ ---
#define ANT_TPMS_CHANNEL      0
#define ANT_TPMS_NETWORK      0
#define ANT_TPMS_FREQ         57   // 2457 MHz (ANT+ frequency)
#define ANT_TPMS_PERIOD       8192 // ~4 Hz channel period

#endif // APP_CONFIG_H
```

- [ ] **Step 8: Commit project setup**

```bash
cd /Users/simruelland/bike-tpms-project
git add .gitignore LICENSE-HARDWARE LICENSE-SOFTWARE README.md firmware/
git commit -m "feat: initialize project structure with firmware toolchain setup"
```

---

### Task 2: Pressure Sensor Driver (Honeywell ABP)

**Files:**
- Create: `firmware/src/pressure.h`
- Create: `firmware/src/pressure.c`
- Create: `firmware/tests/test_pressure.c`
- Create: `firmware/tests/Makefile`

- [ ] **Step 1: Create pressure.h interface**

Create `firmware/src/pressure.h`:

```c
#ifndef PRESSURE_H
#define PRESSURE_H

#include <stdint.h>
#include <stdbool.h>

// Pressure reading in 0.01 PSI units (e.g., 2850 = 28.50 PSI)
typedef uint16_t pressure_psi_hundredths_t;

// Temperature reading in 0.01 °C units (e.g., 2350 = 23.50 °C)
typedef int16_t temp_celsius_hundredths_t;

typedef struct {
    pressure_psi_hundredths_t pressure;
    temp_celsius_hundredths_t temperature;
    bool valid;
} pressure_reading_t;

/**
 * Initialize the pressure sensor over I2C.
 * Must be called after I2C peripheral is initialized.
 * Returns 0 on success, non-zero on failure.
 */
int pressure_init(void);

/**
 * Read pressure and temperature from the sensor.
 * Blocks for ~5ms while sensor completes measurement.
 * Returns reading with valid=true on success.
 */
pressure_reading_t pressure_read(void);

/**
 * Convert raw 14-bit sensor output to PSI hundredths.
 * Exposed for unit testing.
 * raw_pressure: 14-bit value (0-16383)
 * p_min: sensor minimum pressure in PSI (0 for gauge sensors)
 * p_max: sensor maximum pressure in PSI (150 for our sensor)
 */
pressure_psi_hundredths_t pressure_raw_to_psi(uint16_t raw_pressure,
                                               uint16_t p_min,
                                               uint16_t p_max);

/**
 * Convert raw 11-bit sensor temperature output to °C hundredths.
 * Exposed for unit testing.
 * raw_temp: 11-bit value (0-2047)
 */
temp_celsius_hundredths_t pressure_raw_to_temp(uint16_t raw_temp);

#endif // PRESSURE_H
```

- [ ] **Step 2: Create host-side test framework**

Create `firmware/tests/Makefile`:

```makefile
CC = gcc
CFLAGS = -Wall -Werror -I../src -I../config -DUNIT_TEST
LDFLAGS =

TESTS = test_pressure test_alert

all: $(TESTS)
	@for t in $(TESTS); do echo "--- Running $$t ---"; ./$$t; done

test_pressure: test_pressure.c ../src/pressure.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_alert: test_alert.c ../src/alert.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TESTS)

.PHONY: all clean
```

- [ ] **Step 3: Write failing unit tests for pressure conversion**

Create `firmware/tests/test_pressure.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pressure.h"

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_EQ(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) == (expected)) { \
        tests_passed++; \
    } else { \
        printf("FAIL: %s: expected %d, got %d\n", msg, (int)(expected), (int)(actual)); \
    } \
} while(0)

#define ASSERT_NEAR(actual, expected, tolerance, msg) do { \
    tests_run++; \
    if (abs((int)(actual) - (int)(expected)) <= (tolerance)) { \
        tests_passed++; \
    } else { \
        printf("FAIL: %s: expected %d ±%d, got %d\n", msg, (int)(expected), (int)(tolerance), (int)(actual)); \
    } \
} while(0)

void test_raw_to_psi_zero(void) {
    // Raw 0x0666 (1638) = 10% of range = 0 PSI for gauge sensor
    // Honeywell ABP: output 10%-90% maps to Pmin-Pmax
    pressure_psi_hundredths_t result = pressure_raw_to_psi(1638, 0, 150);
    ASSERT_EQ(result, 0, "0 PSI at 10% raw output");
}

void test_raw_to_psi_mid(void) {
    // Raw 8192 (50% of 16384) = 50% of range = 75 PSI
    pressure_psi_hundredths_t result = pressure_raw_to_psi(8192, 0, 150);
    ASSERT_NEAR(result, 7500, 5, "75 PSI at 50% raw output");
}

void test_raw_to_psi_max(void) {
    // Raw 14745 (90% of 16384) = 100% of range = 150 PSI
    pressure_psi_hundredths_t result = pressure_raw_to_psi(14745, 0, 150);
    ASSERT_NEAR(result, 15000, 5, "150 PSI at 90% raw output");
}

void test_raw_to_psi_typical_mtb(void) {
    // ~28 PSI, typical MTB pressure
    // 28/150 = 18.67% of pressure range
    // 10% + 18.67% * 80% = 24.93% of raw range
    // 0.2493 * 16384 = 4086
    pressure_psi_hundredths_t result = pressure_raw_to_psi(4086, 0, 150);
    ASSERT_NEAR(result, 2800, 10, "~28 PSI typical MTB");
}

void test_raw_to_temp_zero(void) {
    // Raw 0 = -40°C per Honeywell ABP datasheet
    temp_celsius_hundredths_t result = pressure_raw_to_temp(0);
    ASSERT_EQ(result, -4000, "0 raw = -40.00°C");
}

void test_raw_to_temp_room(void) {
    // 25°C: (25 - (-40)) / 125 * 2047 = 65/125 * 2047 = 1064.44
    temp_celsius_hundredths_t result = pressure_raw_to_temp(1064);
    ASSERT_NEAR(result, 2500, 10, "~25°C room temperature");
}

void test_raw_to_temp_max(void) {
    // Raw 2047 = 85°C
    temp_celsius_hundredths_t result = pressure_raw_to_temp(2047);
    ASSERT_NEAR(result, 8500, 10, "2047 raw = ~85°C");
}

int main(void) {
    test_raw_to_psi_zero();
    test_raw_to_psi_mid();
    test_raw_to_psi_max();
    test_raw_to_psi_typical_mtb();
    test_raw_to_temp_zero();
    test_raw_to_temp_room();
    test_raw_to_temp_max();

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
```

- [ ] **Step 4: Run tests to verify they fail**

```bash
cd /Users/simruelland/bike-tpms-project/firmware/tests
make test_pressure
./test_pressure
```

Expected: Linker error — `pressure_raw_to_psi` and `pressure_raw_to_temp` not yet implemented.

- [ ] **Step 5: Implement pressure conversion functions**

Create `firmware/src/pressure.c`:

```c
#include "pressure.h"

#ifndef UNIT_TEST
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "app_config.h"

static const nrf_drv_twi_t *m_twi;
#endif

// Honeywell ABP transfer function:
// Output is 14-bit, 10% to 90% of 2^14 maps to Pmin-Pmax
// Raw 1638 (10%) = Pmin, Raw 14745 (90%) = Pmax
#define ABP_OUTPUT_MIN  1638   // 10% of 16384
#define ABP_OUTPUT_MAX  14745  // 90% of 16384

// Temperature: 11-bit, 0 = -40°C, 2047 = 85°C
#define ABP_TEMP_MIN_C  (-40)
#define ABP_TEMP_MAX_C  85
#define ABP_TEMP_RANGE  (ABP_TEMP_MAX_C - ABP_TEMP_MIN_C)  // 125

pressure_psi_hundredths_t pressure_raw_to_psi(uint16_t raw_pressure,
                                               uint16_t p_min,
                                               uint16_t p_max)
{
    if (raw_pressure <= ABP_OUTPUT_MIN) return p_min * 100;
    if (raw_pressure >= ABP_OUTPUT_MAX) return p_max * 100;

    // PSI = (raw - 1638) / (14745 - 1638) * (Pmax - Pmin) + Pmin
    // In hundredths to avoid floating point:
    // psi_100 = (raw - 1638) * (Pmax - Pmin) * 100 / (14745 - 1638) + Pmin * 100
    uint32_t numerator = (uint32_t)(raw_pressure - ABP_OUTPUT_MIN) * (p_max - p_min) * 100;
    uint32_t denominator = ABP_OUTPUT_MAX - ABP_OUTPUT_MIN;  // 13107
    return (pressure_psi_hundredths_t)(numerator / denominator + p_min * 100);
}

temp_celsius_hundredths_t pressure_raw_to_temp(uint16_t raw_temp)
{
    if (raw_temp > 2047) raw_temp = 2047;

    // temp_C = raw / 2047 * 125 - 40
    // In hundredths: temp_100 = raw * 12500 / 2047 - 4000
    int32_t temp_100 = ((int32_t)raw_temp * 12500) / 2047 - 4000;
    return (temp_celsius_hundredths_t)temp_100;
}

#ifndef UNIT_TEST

int pressure_init(void)
{
    // Store TWI instance — caller must have initialized it
    extern const nrf_drv_twi_t m_twi_instance;
    m_twi = &m_twi_instance;

    // Send a measurement request to verify sensor is present
    uint8_t cmd = 0x00;
    ret_code_t err = nrf_drv_twi_tx(m_twi, ABP_I2C_ADDR, &cmd, 0, false);
    if (err != NRF_SUCCESS) return -1;

    nrf_delay_ms(5);  // Wait for measurement

    uint8_t buf[4];
    err = nrf_drv_twi_rx(m_twi, ABP_I2C_ADDR, buf, 4);
    if (err != NRF_SUCCESS) return -1;

    // Check status bits (top 2 bits of byte 0)
    uint8_t status = (buf[0] >> 6) & 0x03;
    if (status == 0x03) return -1;  // Diagnostic fault

    return 0;
}

pressure_reading_t pressure_read(void)
{
    pressure_reading_t reading = { .pressure = 0, .temperature = 0, .valid = false };

    // Trigger measurement (send empty write)
    uint8_t cmd = 0x00;
    ret_code_t err = nrf_drv_twi_tx(m_twi, ABP_I2C_ADDR, &cmd, 0, false);
    if (err != NRF_SUCCESS) return reading;

    nrf_delay_ms(5);  // Measurement time

    // Read 4 bytes: [status+pressure_hi, pressure_lo, temp_hi, temp_lo]
    uint8_t buf[4];
    err = nrf_drv_twi_rx(m_twi, ABP_I2C_ADDR, buf, 4);
    if (err != NRF_SUCCESS) return reading;

    // Check status
    uint8_t status = (buf[0] >> 6) & 0x03;
    if (status != 0x00) return reading;  // 0x00 = normal, fresh data

    // Extract 14-bit pressure (bits 13:0 of bytes 0-1)
    uint16_t raw_pressure = ((uint16_t)(buf[0] & 0x3F) << 8) | buf[1];

    // Extract 11-bit temperature (bits 10:0 of bytes 2-3, right-aligned)
    uint16_t raw_temp = ((uint16_t)buf[2] << 3) | (buf[3] >> 5);

    reading.pressure = pressure_raw_to_psi(raw_pressure,
                                            PRESSURE_MIN_PSI,
                                            PRESSURE_MAX_PSI);
    reading.temperature = pressure_raw_to_temp(raw_temp);
    reading.valid = true;

    return reading;
}

#endif // UNIT_TEST
```

- [ ] **Step 6: Run tests to verify they pass**

```bash
cd /Users/simruelland/bike-tpms-project/firmware/tests
make test_pressure
./test_pressure
```

Expected: `7/7 tests passed`

- [ ] **Step 7: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add firmware/src/pressure.h firmware/src/pressure.c firmware/tests/
git commit -m "feat: add Honeywell ABP pressure sensor driver with unit tests"
```

---

### Task 3: Temperature-Compensated Alert Engine

**Files:**
- Create: `firmware/src/alert.h`
- Create: `firmware/src/alert.c`
- Create: `firmware/tests/test_alert.c`

- [ ] **Step 1: Create alert.h interface**

Create `firmware/src/alert.h`:

```c
#ifndef ALERT_H
#define ALERT_H

#include <stdint.h>
#include <stdbool.h>
#include "pressure.h"

typedef struct {
    pressure_psi_hundredths_t baseline_pressure;  // Pressure at inflation
    temp_celsius_hundredths_t baseline_temp;       // Temperature at inflation
    pressure_psi_hundredths_t threshold;           // Alert threshold in 0.01 PSI
    bool baseline_set;
} alert_config_t;

/**
 * Initialize alert engine with default config.
 */
void alert_init(alert_config_t *config);

/**
 * Set baseline pressure and temperature (called at inflation/setup).
 */
void alert_set_baseline(alert_config_t *config,
                        pressure_psi_hundredths_t pressure,
                        temp_celsius_hundredths_t temperature);

/**
 * Set alert threshold in 0.01 PSI units.
 */
void alert_set_threshold(alert_config_t *config,
                         pressure_psi_hundredths_t threshold);

/**
 * Calculate expected pressure at current temperature using ideal gas law.
 * P2 = P1 * T2 / T1 (temperatures in Kelvin)
 * Returns expected pressure in 0.01 PSI units.
 * Exposed for unit testing.
 */
pressure_psi_hundredths_t alert_expected_pressure(
    pressure_psi_hundredths_t baseline_pressure,
    temp_celsius_hundredths_t baseline_temp,
    temp_celsius_hundredths_t current_temp);

/**
 * Check if current reading indicates a flat (real pressure loss).
 * Returns true if pressure is below expected-threshold (real air loss).
 * Returns false if pressure drop is explained by temperature change.
 */
bool alert_check_flat(const alert_config_t *config,
                      pressure_psi_hundredths_t current_pressure,
                      temp_celsius_hundredths_t current_temp);

#endif // ALERT_H
```

- [ ] **Step 2: Write failing unit tests**

Create `firmware/tests/test_alert.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include "alert.h"

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_TRUE(expr, msg) do { \
    tests_run++; \
    if (expr) { tests_passed++; } \
    else { printf("FAIL: %s\n", msg); } \
} while(0)

#define ASSERT_FALSE(expr, msg) do { \
    tests_run++; \
    if (!(expr)) { tests_passed++; } \
    else { printf("FAIL: %s (expected false)\n", msg); } \
} while(0)

#define ASSERT_NEAR(actual, expected, tolerance, msg) do { \
    tests_run++; \
    if (abs((int)(actual) - (int)(expected)) <= (tolerance)) { \
        tests_passed++; \
    } else { \
        printf("FAIL: %s: expected %d ±%d, got %d\n", msg, (int)(expected), (int)(tolerance), (int)(actual)); \
    } \
} while(0)

void test_expected_pressure_same_temp(void) {
    // Same temperature -> same pressure
    uint16_t result = alert_expected_pressure(2800, 2500, 2500);
    ASSERT_NEAR(result, 2800, 1, "Same temp gives same pressure");
}

void test_expected_pressure_temp_increase(void) {
    // 28.00 PSI at 25°C (298.15K), now 35°C (308.15K)
    // P2 = 28.00 * 308.15 / 298.15 = 28.94 PSI
    uint16_t result = alert_expected_pressure(2800, 2500, 3500);
    ASSERT_NEAR(result, 2894, 10, "Higher temp gives higher expected pressure");
}

void test_expected_pressure_temp_decrease(void) {
    // 28.00 PSI at 25°C (298.15K), now 5°C (278.15K)
    // P2 = 28.00 * 278.15 / 298.15 = 26.12 PSI
    uint16_t result = alert_expected_pressure(2800, 2500, 500);
    ASSERT_NEAR(result, 2612, 10, "Lower temp gives lower expected pressure");
}

void test_expected_pressure_cold_extreme(void) {
    // 28.00 PSI at 20°C (293.15K), now -30°C (243.15K)
    // P2 = 28.00 * 243.15 / 293.15 = 23.22 PSI
    uint16_t result = alert_expected_pressure(2800, 2000, -3000);
    ASSERT_NEAR(result, 2322, 15, "Extreme cold expected pressure");
}

void test_no_flat_normal_pressure(void) {
    // Pressure is normal, no alert
    alert_config_t config;
    alert_init(&config);
    alert_set_baseline(&config, 2800, 2500);  // 28 PSI at 25°C
    alert_set_threshold(&config, 500);         // Alert if 5 PSI below expected

    // Current: 27.50 PSI at 24°C — slight drop, within threshold
    ASSERT_FALSE(alert_check_flat(&config, 2750, 2400),
                 "Slight pressure drop within threshold");
}

void test_no_flat_temp_drop(void) {
    // Pressure dropped but temperature explains it — no alert
    alert_config_t config;
    alert_init(&config);
    alert_set_baseline(&config, 2800, 2500);  // 28 PSI at 25°C
    alert_set_threshold(&config, 500);         // 5 PSI threshold

    // Current: 26.00 PSI at 5°C — pressure dropped ~2 PSI
    // Expected at 5°C: ~26.12 PSI — so 26.00 is within threshold
    ASSERT_FALSE(alert_check_flat(&config, 2600, 500),
                 "Pressure drop explained by temperature");
}

void test_flat_detected(void) {
    // Real pressure loss — alert should fire
    alert_config_t config;
    alert_init(&config);
    alert_set_baseline(&config, 2800, 2500);  // 28 PSI at 25°C
    alert_set_threshold(&config, 500);         // 5 PSI threshold

    // Current: 20.00 PSI at 25°C — 8 PSI loss at same temp = real flat
    ASSERT_TRUE(alert_check_flat(&config, 2000, 2500),
                "8 PSI loss at same temp is a flat");
}

void test_flat_detected_despite_warm(void) {
    // Temperature went up but pressure still way below expected
    alert_config_t config;
    alert_init(&config);
    alert_set_baseline(&config, 2800, 2500);  // 28 PSI at 25°C
    alert_set_threshold(&config, 500);         // 5 PSI threshold

    // At 35°C, expected pressure is ~28.94 PSI
    // Current: 18.00 PSI — way below. Real flat.
    ASSERT_TRUE(alert_check_flat(&config, 1800, 3500),
                "Low pressure despite warming = real flat");
}

void test_no_baseline_no_alert(void) {
    // No baseline set — can't compensate, don't alert
    alert_config_t config;
    alert_init(&config);
    alert_set_threshold(&config, 500);

    ASSERT_FALSE(alert_check_flat(&config, 1500, 2500),
                 "No baseline = no alert (can't compensate)");
}

int main(void) {
    test_expected_pressure_same_temp();
    test_expected_pressure_temp_increase();
    test_expected_pressure_temp_decrease();
    test_expected_pressure_cold_extreme();
    test_no_flat_normal_pressure();
    test_no_flat_temp_drop();
    test_flat_detected();
    test_flat_detected_despite_warm();
    test_no_baseline_no_alert();

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
```

- [ ] **Step 3: Run tests to verify they fail**

```bash
cd /Users/simruelland/bike-tpms-project/firmware/tests
make test_alert
```

Expected: Linker error — alert functions not implemented.

- [ ] **Step 4: Implement alert engine**

Create `firmware/src/alert.c`:

```c
#include "alert.h"

// Convert 0.01°C to Kelvin (as integer hundredths)
// K = C + 273.15, in hundredths: K_100 = C_100 + 27315
#define CELSIUS_TO_KELVIN_100(c_100) ((int32_t)(c_100) + 27315)

void alert_init(alert_config_t *config)
{
    config->baseline_pressure = 0;
    config->baseline_temp = 0;
    config->threshold = 500;  // Default 5.00 PSI
    config->baseline_set = false;
}

void alert_set_baseline(alert_config_t *config,
                        pressure_psi_hundredths_t pressure,
                        temp_celsius_hundredths_t temperature)
{
    config->baseline_pressure = pressure;
    config->baseline_temp = temperature;
    config->baseline_set = true;
}

void alert_set_threshold(alert_config_t *config,
                         pressure_psi_hundredths_t threshold)
{
    config->threshold = threshold;
}

pressure_psi_hundredths_t alert_expected_pressure(
    pressure_psi_hundredths_t baseline_pressure,
    temp_celsius_hundredths_t baseline_temp,
    temp_celsius_hundredths_t current_temp)
{
    // Ideal gas law: P2 = P1 * T2 / T1 (absolute temperatures)
    int32_t t1_kelvin = CELSIUS_TO_KELVIN_100(baseline_temp);
    int32_t t2_kelvin = CELSIUS_TO_KELVIN_100(current_temp);

    if (t1_kelvin <= 0) t1_kelvin = 1;  // Prevent division by zero

    // P2_100 = P1_100 * T2_K100 / T1_K100
    int32_t expected = ((int32_t)baseline_pressure * t2_kelvin) / t1_kelvin;

    if (expected < 0) expected = 0;
    return (pressure_psi_hundredths_t)expected;
}

bool alert_check_flat(const alert_config_t *config,
                      pressure_psi_hundredths_t current_pressure,
                      temp_celsius_hundredths_t current_temp)
{
    if (!config->baseline_set) return false;

    pressure_psi_hundredths_t expected = alert_expected_pressure(
        config->baseline_pressure,
        config->baseline_temp,
        current_temp);

    // Alert if current pressure is more than threshold below expected
    if (expected > config->threshold) {
        return current_pressure < (expected - config->threshold);
    }

    // If expected pressure is already below threshold, alert if any real loss
    return current_pressure == 0;
}
```

- [ ] **Step 5: Run tests to verify they pass**

```bash
cd /Users/simruelland/bike-tpms-project/firmware/tests
make test_alert
./test_alert
```

Expected: `9/9 tests passed`

- [ ] **Step 6: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add firmware/src/alert.h firmware/src/alert.c firmware/tests/test_alert.c firmware/tests/Makefile
git commit -m "feat: add temperature-compensated flat detection alert engine with unit tests"
```

---

### Task 4: Accelerometer Motion Driver (LIS2DH12)

**Files:**
- Create: `firmware/src/motion.h`
- Create: `firmware/src/motion.c`

- [ ] **Step 1: Create motion.h interface**

Create `firmware/src/motion.h`:

```c
#ifndef MOTION_H
#define MOTION_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*motion_event_handler_t)(bool motion_detected);

/**
 * Initialize LIS2DH12 accelerometer for motion detection.
 * Configures low-power mode with interrupt on motion.
 * handler: called on motion detect / motion stop events.
 * Returns 0 on success, non-zero on failure.
 */
int motion_init(motion_event_handler_t handler);

/**
 * Enable motion detection interrupt.
 * Accelerometer wakes MCU via GPIO when motion is detected.
 */
void motion_enable(void);

/**
 * Disable motion detection interrupt.
 */
void motion_disable(void);

/**
 * Check if motion is currently detected.
 */
bool motion_is_active(void);

#endif // MOTION_H
```

- [ ] **Step 2: Implement motion driver**

Create `firmware/src/motion.c`:

```c
#include "motion.h"
#include "app_config.h"

#ifndef UNIT_TEST

#include "nrf_drv_twi.h"
#include "nrfx_gpiote.h"
#include "nrf_delay.h"

// LIS2DH12 register addresses
#define LIS2DH12_WHO_AM_I       0x0F
#define LIS2DH12_CTRL_REG1      0x20
#define LIS2DH12_CTRL_REG2      0x21
#define LIS2DH12_CTRL_REG3      0x22
#define LIS2DH12_CTRL_REG4      0x23
#define LIS2DH12_CTRL_REG5      0x24
#define LIS2DH12_CTRL_REG6      0x25
#define LIS2DH12_INT1_CFG       0x30
#define LIS2DH12_INT1_THS       0x32
#define LIS2DH12_INT1_DURATION  0x33
#define LIS2DH12_INT1_SRC       0x31

#define LIS2DH12_WHO_AM_I_VALUE 0x33

static const nrf_drv_twi_t *m_twi;
static motion_event_handler_t m_handler;
static volatile bool m_motion_active;

static ret_code_t lis2dh12_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return nrf_drv_twi_tx(m_twi, ACCEL_I2C_ADDR, buf, 2, false);
}

static ret_code_t lis2dh12_read_reg(uint8_t reg, uint8_t *val)
{
    ret_code_t err = nrf_drv_twi_tx(m_twi, ACCEL_I2C_ADDR, &reg, 1, true);
    if (err != NRF_SUCCESS) return err;
    return nrf_drv_twi_rx(m_twi, ACCEL_I2C_ADDR, val, 1);
}

static void gpiote_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (pin == PIN_ACCEL_INT1) {
        m_motion_active = true;
        if (m_handler) {
            m_handler(true);
        }
    }
}

int motion_init(motion_event_handler_t handler)
{
    extern const nrf_drv_twi_t m_twi_instance;
    m_twi = &m_twi_instance;
    m_handler = handler;
    m_motion_active = false;

    // Verify WHO_AM_I
    uint8_t who;
    ret_code_t err = lis2dh12_read_reg(LIS2DH12_WHO_AM_I, &who);
    if (err != NRF_SUCCESS || who != LIS2DH12_WHO_AM_I_VALUE) return -1;

    // Configure for low-power motion detection:
    // CTRL_REG1: Low-power mode, 10 Hz ODR, all axes enabled
    lis2dh12_write_reg(LIS2DH12_CTRL_REG1, 0x2F);  // 0b00101111

    // CTRL_REG2: High-pass filter for interrupt 1
    lis2dh12_write_reg(LIS2DH12_CTRL_REG2, 0x01);  // HP filter on INT1

    // CTRL_REG3: INT1 on IA1 (interrupt activity 1)
    lis2dh12_write_reg(LIS2DH12_CTRL_REG3, 0x40);  // I1_IA1

    // CTRL_REG4: ±2g full scale
    lis2dh12_write_reg(LIS2DH12_CTRL_REG4, 0x00);

    // CTRL_REG5: Latch interrupt on INT1
    lis2dh12_write_reg(LIS2DH12_CTRL_REG5, 0x08);  // LIR_INT1

    // INT1_THS: Motion threshold
    lis2dh12_write_reg(LIS2DH12_INT1_THS, MOTION_THRESHOLD);

    // INT1_DURATION: Minimum duration
    lis2dh12_write_reg(LIS2DH12_INT1_DURATION, MOTION_DURATION);

    // INT1_CFG: OR combination of X,Y,Z high events
    lis2dh12_write_reg(LIS2DH12_INT1_CFG, 0x2A);  // XHIE | YHIE | ZHIE

    // Configure GPIOTE for INT1 pin (rising edge)
    if (!nrfx_gpiote_is_init()) {
        nrfx_gpiote_init();
    }

    nrfx_gpiote_in_config_t gpio_config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    gpio_config.pull = NRF_GPIO_PIN_PULLDOWN;

    err = nrfx_gpiote_in_init(PIN_ACCEL_INT1, &gpio_config, gpiote_handler);
    if (err != NRF_SUCCESS) return -1;

    return 0;
}

void motion_enable(void)
{
    nrfx_gpiote_in_event_enable(PIN_ACCEL_INT1, true);
}

void motion_disable(void)
{
    nrfx_gpiote_in_event_disable(PIN_ACCEL_INT1);
}

bool motion_is_active(void)
{
    // Read INT1_SRC to check and clear interrupt
    uint8_t src;
    lis2dh12_read_reg(LIS2DH12_INT1_SRC, &src);
    bool active = (src & 0x40) != 0;  // IA bit
    m_motion_active = active;
    return active;
}

#endif // UNIT_TEST
```

- [ ] **Step 3: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add firmware/src/motion.h firmware/src/motion.c
git commit -m "feat: add LIS2DH12 accelerometer motion detection driver"
```

---

### Task 5: Non-Volatile Storage

**Files:**
- Create: `firmware/src/storage.h`
- Create: `firmware/src/storage.c`

- [ ] **Step 1: Create storage.h interface**

Create `firmware/src/storage.h`:

```c
#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "pressure.h"

typedef struct {
    uint8_t  sensor_location;     // 0x01=Front, 0x02=Rear
    uint16_t alert_threshold;     // 0.01 PSI units
    uint8_t  tx_interval_s;       // Seconds (1-10)
    pressure_psi_hundredths_t baseline_pressure;
    temp_celsius_hundredths_t baseline_temp;
    bool     baseline_valid;
} storage_config_t;

/**
 * Initialize FDS and load stored config.
 * Returns 0 on success, non-zero on failure.
 */
int storage_init(void);

/**
 * Get current config (loaded from flash or defaults).
 */
const storage_config_t *storage_get_config(void);

/**
 * Save config to flash. Persists across power cycles / battery swaps.
 * Returns 0 on success.
 */
int storage_save_config(const storage_config_t *config);

/**
 * Reset config to factory defaults.
 */
void storage_reset_defaults(storage_config_t *config);

#endif // STORAGE_H
```

- [ ] **Step 2: Implement storage**

Create `firmware/src/storage.c`:

```c
#include "storage.h"

#ifndef UNIT_TEST

#include "fds.h"
#include "app_config.h"
#include <string.h>

#define STORAGE_FILE_ID    0x0001
#define STORAGE_REC_KEY    0x0001

static storage_config_t m_config;
static volatile bool m_fds_initialized;
static volatile bool m_fds_write_done;

static void fds_event_handler(fds_evt_t const *p_evt)
{
    switch (p_evt->id) {
        case FDS_EVT_INIT:
            m_fds_initialized = true;
            break;
        case FDS_EVT_WRITE:
        case FDS_EVT_UPDATE:
            m_fds_write_done = true;
            break;
        default:
            break;
    }
}

void storage_reset_defaults(storage_config_t *config)
{
    config->sensor_location = 0x01;  // Front
    config->alert_threshold = ALERT_THRESHOLD_DEFAULT_PSI;
    config->tx_interval_s = TX_INTERVAL_DEFAULT_S;
    config->baseline_pressure = 0;
    config->baseline_temp = 0;
    config->baseline_valid = false;
}

int storage_init(void)
{
    ret_code_t err;

    err = fds_register(fds_event_handler);
    if (err != NRF_SUCCESS) return -1;

    err = fds_init();
    if (err != NRF_SUCCESS) return -1;

    // Wait for init
    while (!m_fds_initialized) {
        __WFE();
    }

    // Try to load existing config
    fds_record_desc_t desc = {0};
    fds_find_token_t  token = {0};

    err = fds_record_find(STORAGE_FILE_ID, STORAGE_REC_KEY, &desc, &token);
    if (err == NRF_SUCCESS) {
        fds_flash_record_t record = {0};
        err = fds_record_open(&desc, &record);
        if (err == NRF_SUCCESS) {
            memcpy(&m_config, record.p_data, sizeof(storage_config_t));
            fds_record_close(&desc);
            return 0;
        }
    }

    // No existing config — use defaults
    storage_reset_defaults(&m_config);
    return 0;
}

const storage_config_t *storage_get_config(void)
{
    return &m_config;
}

int storage_save_config(const storage_config_t *config)
{
    memcpy(&m_config, config, sizeof(storage_config_t));

    fds_record_t record = {
        .file_id = STORAGE_FILE_ID,
        .key     = STORAGE_REC_KEY,
        .data    = {
            .p_data         = &m_config,
            .length_words   = (sizeof(storage_config_t) + 3) / 4,
        }
    };

    m_fds_write_done = false;

    // Check if record exists
    fds_record_desc_t desc = {0};
    fds_find_token_t  token = {0};
    ret_code_t err = fds_record_find(STORAGE_FILE_ID, STORAGE_REC_KEY, &desc, &token);

    if (err == NRF_SUCCESS) {
        err = fds_record_update(&desc, &record);
    } else {
        err = fds_record_write(&desc, &record);
    }

    if (err != NRF_SUCCESS) return -1;

    // Wait for write
    while (!m_fds_write_done) {
        __WFE();
    }

    return 0;
}

#endif // UNIT_TEST
```

- [ ] **Step 3: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add firmware/src/storage.h firmware/src/storage.c
git commit -m "feat: add non-volatile config storage using Nordic FDS"
```

---

### Task 6: BLE Custom GATT Service

**Files:**
- Create: `firmware/src/ble_tpms.h`
- Create: `firmware/src/ble_tpms.c`

- [ ] **Step 1: Create ble_tpms.h interface**

Create `firmware/src/ble_tpms.h`:

```c
#ifndef BLE_TPMS_H
#define BLE_TPMS_H

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "pressure.h"

// Custom 128-bit base UUID: 4f70454e-5450-4d53-xxxx-000000000000
// "OpenTPMS" in ASCII hex
#define BLE_TPMS_UUID_BASE {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                            0x53, 0x4D, 0x50, 0x54, \
                            0x4E, 0x45, 0x70, 0x4F, 0x00, 0x00}

#define BLE_TPMS_UUID_SERVICE          0x0001
#define BLE_TPMS_UUID_PRESSURE_CHAR    0x0002
#define BLE_TPMS_UUID_TEMP_CHAR        0x0003
#define BLE_TPMS_UUID_LOCATION_CHAR    0x0004
#define BLE_TPMS_UUID_THRESHOLD_CHAR   0x0005
#define BLE_TPMS_UUID_TX_INTERVAL_CHAR 0x0006
#define BLE_TPMS_UUID_BASELINE_CHAR    0x0007

// Event types from BLE service to application
typedef enum {
    BLE_TPMS_EVT_LOCATION_WRITTEN,
    BLE_TPMS_EVT_THRESHOLD_WRITTEN,
    BLE_TPMS_EVT_TX_INTERVAL_WRITTEN,
    BLE_TPMS_EVT_BASELINE_WRITTEN,
} ble_tpms_evt_type_t;

typedef struct {
    ble_tpms_evt_type_t type;
    union {
        uint8_t  location;
        uint16_t threshold;
        uint8_t  tx_interval;
        struct {
            pressure_psi_hundredths_t pressure;
            temp_celsius_hundredths_t temperature;
        } baseline;
    } data;
} ble_tpms_evt_t;

typedef void (*ble_tpms_evt_handler_t)(ble_tpms_evt_t *p_evt);

typedef struct {
    uint16_t                 service_handle;
    ble_gatts_char_handles_t pressure_handles;
    ble_gatts_char_handles_t temp_handles;
    ble_gatts_char_handles_t location_handles;
    ble_gatts_char_handles_t threshold_handles;
    ble_gatts_char_handles_t tx_interval_handles;
    ble_gatts_char_handles_t baseline_handles;
    uint16_t                 conn_handle;
    uint8_t                  uuid_type;
    ble_tpms_evt_handler_t   evt_handler;
} ble_tpms_t;

/**
 * Initialize the BLE TPMS custom service.
 */
uint32_t ble_tpms_init(ble_tpms_t *p_tpms, ble_tpms_evt_handler_t evt_handler);

/**
 * Handle BLE stack events (call from ble_evt_handler).
 */
void ble_tpms_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context);

/**
 * Update pressure characteristic and notify connected client.
 */
uint32_t ble_tpms_pressure_update(ble_tpms_t *p_tpms,
                                   pressure_psi_hundredths_t pressure);

/**
 * Update temperature characteristic and notify connected client.
 */
uint32_t ble_tpms_temp_update(ble_tpms_t *p_tpms,
                               temp_celsius_hundredths_t temperature);

#endif // BLE_TPMS_H
```

- [ ] **Step 2: Implement BLE service**

Create `firmware/src/ble_tpms.c`:

```c
#include "ble_tpms.h"
#include "ble_srv_common.h"
#include <string.h>

static uint32_t char_add_readonly_notify(ble_tpms_t *p_tpms,
                                          uint16_t uuid,
                                          ble_gatts_char_handles_t *p_handles,
                                          uint8_t *p_init_value,
                                          uint16_t init_len)
{
    ble_gatts_char_md_t char_md = {0};
    ble_gatts_attr_t    attr = {0};
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md = {0};

    char_md.char_props.read   = 1;
    char_md.char_props.notify = 1;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;

    ble_uuid.type = p_tpms->uuid_type;
    ble_uuid.uuid = uuid;

    attr.p_uuid    = &ble_uuid;
    attr.p_attr_md = &attr_md;
    attr.init_len  = init_len;
    attr.max_len   = init_len;
    attr.p_value   = p_init_value;
    attr.init_offs = 0;

    return sd_ble_gatts_characteristic_add(p_tpms->service_handle,
                                            &char_md, &attr, p_handles);
}

static uint32_t char_add_readwrite(ble_tpms_t *p_tpms,
                                    uint16_t uuid,
                                    ble_gatts_char_handles_t *p_handles,
                                    uint8_t *p_init_value,
                                    uint16_t init_len)
{
    ble_gatts_char_md_t char_md = {0};
    ble_gatts_attr_t    attr = {0};
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md = {0};

    char_md.char_props.read  = 1;
    char_md.char_props.write = 1;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;

    ble_uuid.type = p_tpms->uuid_type;
    ble_uuid.uuid = uuid;

    attr.p_uuid    = &ble_uuid;
    attr.p_attr_md = &attr_md;
    attr.init_len  = init_len;
    attr.max_len   = init_len;
    attr.p_value   = p_init_value;
    attr.init_offs = 0;

    return sd_ble_gatts_characteristic_add(p_tpms->service_handle,
                                            &char_md, &attr, p_handles);
}

uint32_t ble_tpms_init(ble_tpms_t *p_tpms, ble_tpms_evt_handler_t evt_handler)
{
    uint32_t err_code;
    ble_uuid_t ble_uuid;
    ble_uuid128_t base_uuid = {BLE_TPMS_UUID_BASE};

    p_tpms->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_tpms->evt_handler = evt_handler;

    // Add custom UUID base
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_tpms->uuid_type);
    if (err_code != NRF_SUCCESS) return err_code;

    // Add service
    ble_uuid.type = p_tpms->uuid_type;
    ble_uuid.uuid = BLE_TPMS_UUID_SERVICE;
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                         &ble_uuid,
                                         &p_tpms->service_handle);
    if (err_code != NRF_SUCCESS) return err_code;

    // Add characteristics
    uint16_t zero16 = 0;
    int16_t  zero16s = 0;
    uint8_t  location_init = 0x01;  // Front
    uint16_t threshold_init = 500;  // 5.00 PSI
    uint8_t  tx_int_init = 3;       // 3 seconds
    uint8_t  baseline_init[4] = {0};

    err_code = char_add_readonly_notify(p_tpms, BLE_TPMS_UUID_PRESSURE_CHAR,
                                         &p_tpms->pressure_handles,
                                         (uint8_t *)&zero16, 2);
    if (err_code != NRF_SUCCESS) return err_code;

    err_code = char_add_readonly_notify(p_tpms, BLE_TPMS_UUID_TEMP_CHAR,
                                         &p_tpms->temp_handles,
                                         (uint8_t *)&zero16s, 2);
    if (err_code != NRF_SUCCESS) return err_code;

    err_code = char_add_readwrite(p_tpms, BLE_TPMS_UUID_LOCATION_CHAR,
                                   &p_tpms->location_handles,
                                   &location_init, 1);
    if (err_code != NRF_SUCCESS) return err_code;

    err_code = char_add_readwrite(p_tpms, BLE_TPMS_UUID_THRESHOLD_CHAR,
                                   &p_tpms->threshold_handles,
                                   (uint8_t *)&threshold_init, 2);
    if (err_code != NRF_SUCCESS) return err_code;

    err_code = char_add_readwrite(p_tpms, BLE_TPMS_UUID_TX_INTERVAL_CHAR,
                                   &p_tpms->tx_interval_handles,
                                   &tx_int_init, 1);
    if (err_code != NRF_SUCCESS) return err_code;

    err_code = char_add_readwrite(p_tpms, BLE_TPMS_UUID_BASELINE_CHAR,
                                   &p_tpms->baseline_handles,
                                   baseline_init, 4);
    if (err_code != NRF_SUCCESS) return err_code;

    return NRF_SUCCESS;
}

void ble_tpms_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context)
{
    ble_tpms_t *p_tpms = (ble_tpms_t *)p_context;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            p_tpms->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            p_tpms->conn_handle = BLE_CONN_HANDLE_INVALID;
            break;
        case BLE_GATTS_EVT_WRITE: {
            ble_gatts_evt_write_t const *p_write = &p_ble_evt->evt.gatts_evt.params.write;
            ble_tpms_evt_t evt;

            if (p_write->handle == p_tpms->location_handles.value_handle && p_write->len == 1) {
                evt.type = BLE_TPMS_EVT_LOCATION_WRITTEN;
                evt.data.location = p_write->data[0];
                p_tpms->evt_handler(&evt);
            }
            else if (p_write->handle == p_tpms->threshold_handles.value_handle && p_write->len == 2) {
                evt.type = BLE_TPMS_EVT_THRESHOLD_WRITTEN;
                memcpy(&evt.data.threshold, p_write->data, 2);
                p_tpms->evt_handler(&evt);
            }
            else if (p_write->handle == p_tpms->tx_interval_handles.value_handle && p_write->len == 1) {
                evt.type = BLE_TPMS_EVT_TX_INTERVAL_WRITTEN;
                evt.data.tx_interval = p_write->data[0];
                p_tpms->evt_handler(&evt);
            }
            else if (p_write->handle == p_tpms->baseline_handles.value_handle && p_write->len == 4) {
                evt.type = BLE_TPMS_EVT_BASELINE_WRITTEN;
                memcpy(&evt.data.baseline.pressure, p_write->data, 2);
                memcpy(&evt.data.baseline.temperature, p_write->data + 2, 2);
                p_tpms->evt_handler(&evt);
            }
            break;
        }
        default:
            break;
    }
}

uint32_t ble_tpms_pressure_update(ble_tpms_t *p_tpms,
                                   pressure_psi_hundredths_t pressure)
{
    if (p_tpms->conn_handle == BLE_CONN_HANDLE_INVALID) return NRF_SUCCESS;

    uint16_t len = sizeof(pressure);
    ble_gatts_hvx_params_t params = {
        .handle = p_tpms->pressure_handles.value_handle,
        .type   = BLE_GATT_HVX_NOTIFICATION,
        .offset = 0,
        .p_len  = &len,
        .p_data = (uint8_t *)&pressure,
    };
    return sd_ble_gatts_hvx(p_tpms->conn_handle, &params);
}

uint32_t ble_tpms_temp_update(ble_tpms_t *p_tpms,
                               temp_celsius_hundredths_t temperature)
{
    if (p_tpms->conn_handle == BLE_CONN_HANDLE_INVALID) return NRF_SUCCESS;

    uint16_t len = sizeof(temperature);
    ble_gatts_hvx_params_t params = {
        .handle = p_tpms->temp_handles.value_handle,
        .type   = BLE_GATT_HVX_NOTIFICATION,
        .offset = 0,
        .p_len  = &len,
        .p_data = (uint8_t *)&temperature,
    };
    return sd_ble_gatts_hvx(p_tpms->conn_handle, &params);
}
```

- [ ] **Step 3: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add firmware/src/ble_tpms.h firmware/src/ble_tpms.c
git commit -m "feat: add BLE custom GATT service for TPMS data and config"
```

---

### Task 7: ANT+ Tire Pressure Profile

**Files:**
- Create: `firmware/src/ant_tpms.h`
- Create: `firmware/src/ant_tpms.c`

- [ ] **Step 1: Create ant_tpms.h interface**

Create `firmware/src/ant_tpms.h`:

```c
#ifndef ANT_TPMS_H
#define ANT_TPMS_H

#include <stdint.h>
#include "pressure.h"

/**
 * Initialize ANT+ tire pressure channel.
 * Returns 0 on success.
 */
int ant_tpms_init(void);

/**
 * Open the ANT+ channel and start broadcasting.
 */
int ant_tpms_start(void);

/**
 * Stop broadcasting and close channel.
 */
void ant_tpms_stop(void);

/**
 * Update pressure/temperature data and broadcast on next TX event.
 */
void ant_tpms_update(pressure_psi_hundredths_t pressure,
                     temp_celsius_hundredths_t temperature,
                     uint8_t location,
                     uint8_t battery_status);

/**
 * Handle ANT+ stack events (call from ant_evt_handler).
 */
void ant_tpms_on_ant_evt(uint8_t event, uint8_t *p_event_message_buffer);

#endif // ANT_TPMS_H
```

- [ ] **Step 2: Implement ANT+ broadcasting**

Create `firmware/src/ant_tpms.c`:

```c
#include "ant_tpms.h"
#include "app_config.h"

#ifndef UNIT_TEST

#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_channel_config.h"
#include "nrf_sdh_ant.h"
#include <string.h>

// ANT+ Tire Pressure data page layout
// Based on ANT+ Bicycle Tire Pressure device profile
#define TPMS_DATA_PAGE_PRESSURE  0x01

static uint8_t m_tx_buffer[8];
static bool    m_channel_open;

int ant_tpms_init(void)
{
    // Configure ANT channel
    ant_channel_config_t channel_config = {
        .channel_number    = ANT_TPMS_CHANNEL,
        .channel_type      = CHANNEL_TYPE_MASTER,
        .ext_assign        = 0,
        .rf_freq           = ANT_TPMS_FREQ,
        .transmission_type = 0x05,    // Global data pages
        .device_type       = 0x30,    // Tire pressure sensor device type
        .device_number     = NRF_FICR->DEVICEADDR[0] & 0xFFFF, // Unique from chip ID
        .channel_period    = ANT_TPMS_PERIOD,
        .network_number    = ANT_TPMS_NETWORK,
    };

    uint32_t err = ant_channel_init(&channel_config);
    if (err != NRF_SUCCESS) return -1;

    // Set network key (ANT+ public network key)
    static const uint8_t ant_plus_network_key[] = {
        0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45
    };
    err = sd_ant_network_address_set(ANT_TPMS_NETWORK, ant_plus_network_key);
    if (err != NRF_SUCCESS) return -1;

    memset(m_tx_buffer, 0, sizeof(m_tx_buffer));
    m_channel_open = false;

    return 0;
}

int ant_tpms_start(void)
{
    uint32_t err = sd_ant_channel_open(ANT_TPMS_CHANNEL);
    if (err != NRF_SUCCESS) return -1;
    m_channel_open = true;
    return 0;
}

void ant_tpms_stop(void)
{
    if (m_channel_open) {
        sd_ant_channel_close(ANT_TPMS_CHANNEL);
        m_channel_open = false;
    }
}

void ant_tpms_update(pressure_psi_hundredths_t pressure,
                     temp_celsius_hundredths_t temperature,
                     uint8_t location,
                     uint8_t battery_status)
{
    // Build data page
    // Byte 0: Data page number
    m_tx_buffer[0] = TPMS_DATA_PAGE_PRESSURE;

    // Byte 1: Sensor location (0x01=front, 0x02=rear)
    m_tx_buffer[1] = location;

    // Bytes 2-3: Pressure in 0.01 PSI (little-endian)
    m_tx_buffer[2] = pressure & 0xFF;
    m_tx_buffer[3] = (pressure >> 8) & 0xFF;

    // Bytes 4-5: Temperature in 0.01°C (little-endian, signed)
    m_tx_buffer[4] = temperature & 0xFF;
    m_tx_buffer[5] = (temperature >> 8) & 0xFF;

    // Byte 6: Battery status (0=OK, 1=Low, 2=Critical)
    m_tx_buffer[6] = battery_status;

    // Byte 7: Reserved
    m_tx_buffer[7] = 0xFF;

    // Queue broadcast
    if (m_channel_open) {
        sd_ant_broadcast_message_tx(ANT_TPMS_CHANNEL, 8, m_tx_buffer);
    }
}

void ant_tpms_on_ant_evt(uint8_t event, uint8_t *p_event_message_buffer)
{
    switch (event) {
        case EVENT_TX:
            // Data was transmitted — next TX will use updated buffer
            break;
        case EVENT_CHANNEL_CLOSED:
            m_channel_open = false;
            break;
        default:
            break;
    }
}

#endif // UNIT_TEST
```

- [ ] **Step 3: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add firmware/src/ant_tpms.h firmware/src/ant_tpms.c
git commit -m "feat: add ANT+ tire pressure profile broadcasting"
```

---

### Task 8: Main Application — State Machine

**Files:**
- Create: `firmware/src/main.c`

- [ ] **Step 1: Implement main.c with full state machine**

Create `firmware/src/main.c`:

```c
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_ant.h"
#include "nrf_sdh_soc.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "nrf_pwr_mgmt.h"
#include "app_timer.h"
#include "ble_advertising.h"
#include "nrf_ble_gatt.h"

#include "app_config.h"
#include "pressure.h"
#include "motion.h"
#include "alert.h"
#include "ble_tpms.h"
#include "ant_tpms.h"
#include "storage.h"

// --- State Machine ---
typedef enum {
    STATE_DEEP_SLEEP,
    STATE_WAKING,
    STATE_ACTIVE,
    STATE_SLEEPING,
} app_state_t;

static app_state_t m_state = STATE_DEEP_SLEEP;

// --- Peripherals ---
static const nrf_drv_twi_t m_twi_instance = NRF_DRV_TWI_INSTANCE(0);
NRF_BLE_GATT_DEF(m_gatt);
BLE_ADVERTISING_DEF(m_advertising);

static ble_tpms_t    m_ble_tpms;
static alert_config_t m_alert_config;

// --- Timers ---
APP_TIMER_DEF(m_tx_timer);
APP_TIMER_DEF(m_sleep_timer);

static volatile bool m_tx_pending;
static volatile bool m_sleep_timeout;

// --- Forward declarations ---
static void state_enter_deep_sleep(void);
static void state_enter_waking(void);
static void state_enter_active(void);
static void state_enter_sleeping(void);

// --- Timer callbacks ---
static void tx_timer_handler(void *p_context)
{
    m_tx_pending = true;
}

static void sleep_timer_handler(void *p_context)
{
    m_sleep_timeout = true;
}

// --- Motion callback ---
static void motion_handler(bool motion_detected)
{
    if (motion_detected && m_state == STATE_DEEP_SLEEP) {
        state_enter_waking();
    }
    if (motion_detected && m_state == STATE_ACTIVE) {
        // Reset sleep timeout
        app_timer_stop(m_sleep_timer);
        app_timer_start(m_sleep_timer,
                        APP_TIMER_TICKS(SLEEP_TIMEOUT_S * 1000),
                        NULL);
    }
}

// --- BLE event handlers ---
static void ble_tpms_evt_handler(ble_tpms_evt_t *p_evt)
{
    storage_config_t config = *storage_get_config();

    switch (p_evt->type) {
        case BLE_TPMS_EVT_LOCATION_WRITTEN:
            config.sensor_location = p_evt->data.location;
            storage_save_config(&config);
            break;
        case BLE_TPMS_EVT_THRESHOLD_WRITTEN:
            config.alert_threshold = p_evt->data.threshold;
            alert_set_threshold(&m_alert_config, p_evt->data.threshold);
            storage_save_config(&config);
            break;
        case BLE_TPMS_EVT_TX_INTERVAL_WRITTEN:
            if (p_evt->data.tx_interval >= TX_INTERVAL_MIN_S &&
                p_evt->data.tx_interval <= TX_INTERVAL_MAX_S) {
                config.tx_interval_s = p_evt->data.tx_interval;
                storage_save_config(&config);
                // Restart TX timer with new interval
                app_timer_stop(m_tx_timer);
                app_timer_start(m_tx_timer,
                                APP_TIMER_TICKS(config.tx_interval_s * 1000),
                                NULL);
            }
            break;
        case BLE_TPMS_EVT_BASELINE_WRITTEN:
            config.baseline_pressure = p_evt->data.baseline.pressure;
            config.baseline_temp = p_evt->data.baseline.temperature;
            config.baseline_valid = true;
            alert_set_baseline(&m_alert_config,
                               p_evt->data.baseline.pressure,
                               p_evt->data.baseline.temperature);
            storage_save_config(&config);
            break;
    }
}

static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    ble_tpms_on_ble_evt(p_ble_evt, &m_ble_tpms);

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
            break;
        default:
            break;
    }
}

NRF_SDH_BLE_OBSERVER(m_ble_observer, 3, ble_evt_handler, NULL);

// --- ANT event handler ---
static void ant_evt_handler(ant_evt_t *p_ant_evt, void *p_context)
{
    ant_tpms_on_ant_evt(p_ant_evt->event, p_ant_evt->message.aucMessage);
}

NRF_SDH_ANT_OBSERVER(m_ant_observer, 1, ant_evt_handler, NULL);

// --- Initialization ---
static void twi_init(void)
{
    const nrf_drv_twi_config_t twi_config = {
        .scl                = PIN_I2C_SCL,
        .sda                = PIN_I2C_SDA,
        .frequency          = NRF_DRV_TWI_FREQ_400K,
        .interrupt_priority = APP_IRQ_PRIORITY_MID,
    };
    nrf_drv_twi_init(&m_twi_instance, &twi_config, NULL, NULL);
    nrf_drv_twi_enable(&m_twi_instance);
}

static void ble_stack_init(void)
{
    nrf_sdh_enable_request();

    uint32_t ram_start = 0;
    nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    nrf_sdh_ble_enable(&ram_start);
}

static void gap_params_init(void)
{
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    sd_ble_gap_device_name_set(&sec_mode,
                                (uint8_t *)DEVICE_NAME,
                                strlen(DEVICE_NAME));
}

static void advertising_init(void)
{
    ble_advertising_init_t init = {0};
    init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    init.advdata.flags     = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    ble_advertising_init(&m_advertising, &init);
}

static void services_init(void)
{
    ble_tpms_init(&m_ble_tpms, ble_tpms_evt_handler);
}

// --- State Machine Implementation ---
static void do_tx_cycle(void)
{
    pressure_reading_t reading = pressure_read();
    if (!reading.valid) return;

    const storage_config_t *config = storage_get_config();

    // Update BLE
    ble_tpms_pressure_update(&m_ble_tpms, reading.pressure);
    ble_tpms_temp_update(&m_ble_tpms, reading.temperature);

    // Update ANT+
    uint8_t battery_status = 0;  // TODO: measure actual battery voltage
    ant_tpms_update(reading.pressure, reading.temperature,
                    config->sensor_location, battery_status);

    // Check for flat
    if (alert_check_flat(&m_alert_config, reading.pressure, reading.temperature)) {
        // Alert is embedded in the ANT+/BLE data — head units interpret it
        // Future: could trigger a specific alert characteristic
    }
}

static void state_enter_deep_sleep(void)
{
    m_state = STATE_DEEP_SLEEP;

    // Stop timers
    app_timer_stop(m_tx_timer);
    app_timer_stop(m_sleep_timer);

    // Stop ANT+
    ant_tpms_stop();

    // Disable TWI to save power
    nrf_drv_twi_disable(&m_twi_instance);

    // Enable motion detect interrupt
    motion_enable();

    // Enter System ON low power (SoftDevice manages sleep)
    // MCU will wake on accelerometer interrupt
}

static void state_enter_waking(void)
{
    m_state = STATE_WAKING;

    // Re-enable TWI
    nrf_drv_twi_enable(&m_twi_instance);

    // Initialize sensors
    pressure_init();

    // Start ANT+ broadcasting
    ant_tpms_start();

    // Start BLE advertising
    ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);

    // Immediately transition to active
    state_enter_active();
}

static void state_enter_active(void)
{
    m_state = STATE_ACTIVE;

    const storage_config_t *config = storage_get_config();

    // Start TX timer
    m_tx_pending = false;
    app_timer_start(m_tx_timer,
                    APP_TIMER_TICKS(config->tx_interval_s * 1000),
                    NULL);

    // Start sleep timeout timer
    m_sleep_timeout = false;
    app_timer_start(m_sleep_timer,
                    APP_TIMER_TICKS(SLEEP_TIMEOUT_S * 1000),
                    NULL);

    // Do first TX immediately
    do_tx_cycle();
}

static void state_enter_sleeping(void)
{
    m_state = STATE_SLEEPING;

    // One final pressure reading for baseline comparison on next wake
    pressure_reading_t reading = pressure_read();
    if (reading.valid) {
        // Store as baseline if not already set
        const storage_config_t *config = storage_get_config();
        if (!config->baseline_valid) {
            storage_config_t new_config = *config;
            new_config.baseline_pressure = reading.pressure;
            new_config.baseline_temp = reading.temperature;
            new_config.baseline_valid = true;
            storage_save_config(&new_config);
            alert_set_baseline(&m_alert_config, reading.pressure, reading.temperature);
        }
    }

    state_enter_deep_sleep();
}

// --- Main ---
int main(void)
{
    // Initialize power management
    nrf_pwr_mgmt_init();

    // Initialize timers
    app_timer_init();

    // Initialize BLE stack (includes SoftDevice for both BLE + ANT+)
    ble_stack_init();
    gap_params_init();
    nrf_ble_gatt_init(&m_gatt, NULL);
    services_init();
    advertising_init();

    // Initialize I2C
    twi_init();

    // Initialize storage and load config
    storage_init();
    const storage_config_t *config = storage_get_config();

    // Initialize alert engine from stored config
    alert_init(&m_alert_config);
    alert_set_threshold(&m_alert_config, config->alert_threshold);
    if (config->baseline_valid) {
        alert_set_baseline(&m_alert_config,
                           config->baseline_pressure,
                           config->baseline_temp);
    }

    // Initialize ANT+
    ant_tpms_init();

    // Initialize motion detection
    motion_init(motion_handler);

    // Start in deep sleep — wait for motion
    state_enter_deep_sleep();

    // Main loop
    for (;;) {
        if (m_state == STATE_ACTIVE) {
            if (m_tx_pending) {
                m_tx_pending = false;
                do_tx_cycle();
            }
            if (m_sleep_timeout) {
                m_sleep_timeout = false;
                state_enter_sleeping();
            }
        }

        // Let SoftDevice handle power management
        nrf_pwr_mgmt_run();
    }
}
```

- [ ] **Step 2: Verify build compiles (requires SDK installed)**

```bash
cd /Users/simruelland/bike-tpms-project/firmware
make
```

Expected: Build succeeds (or SDK path errors if SDK not yet installed — that's OK at this stage).

- [ ] **Step 3: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add firmware/src/main.c
git commit -m "feat: add main application with state machine (deep sleep / wake / active / sleep)"
```

---

## Phase 2: Web Configuration Tool

### Task 9: Web Bluetooth Configuration Page

**Files:**
- Create: `web-config/index.html`
- Create: `web-config/app.js`
- Create: `web-config/style.css`

- [ ] **Step 1: Create index.html**

Create `web-config/index.html`:

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>OpenTPMS Configuration</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="container">
        <h1>OpenTPMS</h1>
        <p class="subtitle">Tire Pressure Sensor Configuration</p>

        <div id="connect-section">
            <button id="btn-connect" onclick="connectSensor()">Connect to Sensor</button>
            <p id="status">Not connected</p>
        </div>

        <div id="data-section" class="hidden">
            <div class="card">
                <h2>Live Data</h2>
                <div class="reading">
                    <span class="label">Pressure</span>
                    <span class="value" id="pressure">--</span>
                    <span class="unit">PSI</span>
                </div>
                <div class="reading">
                    <span class="label">Temperature</span>
                    <span class="value" id="temperature">--</span>
                    <span class="unit">&deg;C</span>
                </div>
                <div class="reading">
                    <span class="label">Battery</span>
                    <span class="value" id="battery">--</span>
                    <span class="unit">%</span>
                </div>
            </div>

            <div class="card">
                <h2>Configuration</h2>
                <div class="field">
                    <label for="location">Sensor Location</label>
                    <select id="location" onchange="writeLocation()">
                        <option value="1">Front</option>
                        <option value="2">Rear</option>
                    </select>
                </div>
                <div class="field">
                    <label for="threshold">Alert Threshold (PSI)</label>
                    <input type="number" id="threshold" step="0.1" min="1" max="50" value="5.0">
                    <button onclick="writeThreshold()">Set</button>
                </div>
                <div class="field">
                    <label for="tx-interval">TX Interval (seconds)</label>
                    <input type="number" id="tx-interval" step="1" min="1" max="10" value="3">
                    <button onclick="writeTxInterval()">Set</button>
                </div>
                <div class="field">
                    <label>Baseline</label>
                    <button onclick="setBaseline()">Set Current as Baseline</button>
                    <span id="baseline-status"></span>
                </div>
            </div>

            <button id="btn-disconnect" onclick="disconnectSensor()">Disconnect</button>
        </div>
    </div>
    <script src="app.js"></script>
</body>
</html>
```

- [ ] **Step 2: Create app.js**

Create `web-config/app.js`:

```javascript
const TPMS_SERVICE_UUID = '4f70454e-5450-4d53-0001-000000000000';
const PRESSURE_CHAR     = '4f70454e-5450-4d53-0002-000000000000';
const TEMP_CHAR         = '4f70454e-5450-4d53-0003-000000000000';
const LOCATION_CHAR     = '4f70454e-5450-4d53-0004-000000000000';
const THRESHOLD_CHAR    = '4f70454e-5450-4d53-0005-000000000000';
const TX_INTERVAL_CHAR  = '4f70454e-5450-4d53-0006-000000000000';
const BASELINE_CHAR     = '4f70454e-5450-4d53-0007-000000000000';
const BATTERY_SERVICE   = 'battery_service';
const BATTERY_LEVEL     = 'battery_level';

let device = null;
let server = null;
let tpmsService = null;
let chars = {};
let lastPressure = 0;
let lastTemp = 0;

async function connectSensor() {
    try {
        document.getElementById('status').textContent = 'Scanning...';

        device = await navigator.bluetooth.requestDevice({
            filters: [{ namePrefix: 'OpenTPMS' }],
            optionalServices: [TPMS_SERVICE_UUID, BATTERY_SERVICE]
        });

        device.addEventListener('gattserverdisconnected', onDisconnected);

        document.getElementById('status').textContent = 'Connecting...';
        server = await device.gatt.connect();

        tpmsService = await server.getPrimaryService(TPMS_SERVICE_UUID);

        chars.pressure   = await tpmsService.getCharacteristic(PRESSURE_CHAR);
        chars.temp       = await tpmsService.getCharacteristic(TEMP_CHAR);
        chars.location   = await tpmsService.getCharacteristic(LOCATION_CHAR);
        chars.threshold  = await tpmsService.getCharacteristic(THRESHOLD_CHAR);
        chars.txInterval = await tpmsService.getCharacteristic(TX_INTERVAL_CHAR);
        chars.baseline   = await tpmsService.getCharacteristic(BASELINE_CHAR);

        // Subscribe to notifications
        chars.pressure.addEventListener('characteristicvaluechanged', onPressure);
        await chars.pressure.startNotifications();

        chars.temp.addEventListener('characteristicvaluechanged', onTemperature);
        await chars.temp.startNotifications();

        // Read current config
        const locVal = await chars.location.readValue();
        document.getElementById('location').value = locVal.getUint8(0);

        const threshVal = await chars.threshold.readValue();
        document.getElementById('threshold').value = (threshVal.getUint16(0, true) / 100).toFixed(1);

        const txVal = await chars.txInterval.readValue();
        document.getElementById('tx-interval').value = txVal.getUint8(0);

        // Try battery service
        try {
            const battService = await server.getPrimaryService(BATTERY_SERVICE);
            const battChar = await battService.getCharacteristic(BATTERY_LEVEL);
            battChar.addEventListener('characteristicvaluechanged', onBattery);
            await battChar.startNotifications();
            const battVal = await battChar.readValue();
            document.getElementById('battery').textContent = battVal.getUint8(0);
        } catch (e) {
            document.getElementById('battery').textContent = 'N/A';
        }

        document.getElementById('status').textContent = 'Connected to ' + device.name;
        document.getElementById('connect-section').querySelector('button').textContent = 'Connected';
        document.getElementById('data-section').classList.remove('hidden');

    } catch (error) {
        document.getElementById('status').textContent = 'Error: ' + error.message;
    }
}

function disconnectSensor() {
    if (device && device.gatt.connected) {
        device.gatt.disconnect();
    }
}

function onDisconnected() {
    document.getElementById('status').textContent = 'Disconnected';
    document.getElementById('data-section').classList.add('hidden');
    document.getElementById('connect-section').querySelector('button').textContent = 'Connect to Sensor';
}

function onPressure(event) {
    const value = event.target.value.getUint16(0, true);
    lastPressure = value;
    document.getElementById('pressure').textContent = (value / 100).toFixed(2);
}

function onTemperature(event) {
    const value = event.target.value.getInt16(0, true);
    lastTemp = value;
    document.getElementById('temperature').textContent = (value / 100).toFixed(1);
}

function onBattery(event) {
    document.getElementById('battery').textContent = event.target.value.getUint8(0);
}

async function writeLocation() {
    const val = parseInt(document.getElementById('location').value);
    const data = new Uint8Array([val]);
    await chars.location.writeValue(data);
}

async function writeThreshold() {
    const psi = parseFloat(document.getElementById('threshold').value);
    const hundredths = Math.round(psi * 100);
    const data = new DataView(new ArrayBuffer(2));
    data.setUint16(0, hundredths, true);
    await chars.threshold.writeValue(data);
}

async function writeTxInterval() {
    const val = parseInt(document.getElementById('tx-interval').value);
    if (val < 1 || val > 10) return;
    const data = new Uint8Array([val]);
    await chars.txInterval.writeValue(data);
}

async function setBaseline() {
    const data = new DataView(new ArrayBuffer(4));
    data.setUint16(0, lastPressure, true);
    data.setInt16(2, lastTemp, true);
    await chars.baseline.writeValue(data);
    document.getElementById('baseline-status').textContent =
        `Set: ${(lastPressure/100).toFixed(1)} PSI @ ${(lastTemp/100).toFixed(1)}°C`;
}
```

- [ ] **Step 3: Create style.css**

Create `web-config/style.css`:

```css
* { margin: 0; padding: 0; box-sizing: border-box; }

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
    background: #0d1117;
    color: #e6edf3;
    min-height: 100vh;
}

.container {
    max-width: 480px;
    margin: 0 auto;
    padding: 24px 16px;
}

h1 { font-size: 24px; font-weight: 700; }
h2 { font-size: 16px; font-weight: 600; margin-bottom: 12px; }
.subtitle { color: #8b949e; margin-bottom: 24px; }

.card {
    background: #161b22;
    border: 1px solid #30363d;
    border-radius: 8px;
    padding: 16px;
    margin-bottom: 16px;
}

.reading {
    display: flex;
    align-items: baseline;
    padding: 8px 0;
    border-bottom: 1px solid #21262d;
}
.reading:last-child { border-bottom: none; }
.reading .label { flex: 1; color: #8b949e; }
.reading .value { font-size: 28px; font-weight: 700; font-variant-numeric: tabular-nums; }
.reading .unit { color: #8b949e; margin-left: 4px; }

.field {
    padding: 8px 0;
    display: flex;
    align-items: center;
    gap: 8px;
    flex-wrap: wrap;
}
.field label { flex: 1; min-width: 120px; color: #8b949e; }
.field select, .field input {
    background: #0d1117;
    border: 1px solid #30363d;
    color: #e6edf3;
    padding: 6px 10px;
    border-radius: 4px;
    font-size: 14px;
}
.field input { width: 80px; }

button {
    background: #238636;
    color: #fff;
    border: none;
    padding: 8px 16px;
    border-radius: 6px;
    font-size: 14px;
    cursor: pointer;
}
button:hover { background: #2ea043; }
button:active { background: #196c2e; }

#btn-disconnect { background: #da3633; width: 100%; margin-top: 8px; }
#btn-disconnect:hover { background: #f85149; }
#btn-connect { width: 100%; padding: 12px; font-size: 16px; }

#status { text-align: center; color: #8b949e; margin-top: 8px; }
#baseline-status { color: #8b949e; font-size: 12px; }

.hidden { display: none; }
```

- [ ] **Step 4: Test locally**

```bash
cd /Users/simruelland/bike-tpms-project/web-config
python3 -m http.server 8080
```

Open `http://localhost:8080` in Chrome. Verify the page renders correctly. The "Connect" button will show the Bluetooth device picker (no sensor available yet — that's expected).

- [ ] **Step 5: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add web-config/
git commit -m "feat: add Web Bluetooth configuration tool for sensor setup"
```

---

## Phase 3: Hardware Design

### Task 10: KiCad Schematic

**Files:**
- Create: `hardware/kicad/open-tpms.kicad_pro`
- Create: `hardware/kicad/open-tpms.kicad_sch`

This task is done in KiCad GUI, not in code. Steps documented for the engineer:

- [ ] **Step 1: Create KiCad project**

Open KiCad 8. Create new project at `hardware/kicad/open-tpms`. This generates `.kicad_pro` and `.kicad_sch` files.

- [ ] **Step 2: Add component symbols**

In the schematic editor, place these components:

1. **U1 — Raytac MDBT42Q-512KV2:** Use a generic QFN footprint or download Raytac's KiCad library from their website. Key pins: VCC (3V), GND, P0.26 (SDA), P0.27 (SCL), P0.11 (ACCEL_INT1), SWDIO, SWDCLK.

2. **U2 — Honeywell ABPDANT150PGAA5:** I2C pressure sensor. Pins: VDD, GND, SDA, SCL, pressure port (mechanical).

3. **U3 — LIS2DH12TR:** I2C accelerometer. Pins: VDD, GND, SDA, SCL, INT1, SDO/SA0 (tie high for addr 0x19).

4. **C1 — Supercapacitor:** 47-100mF, 3.5V. Two-terminal component.

5. **BT1 — CR1225 holder:** Two-terminal battery holder.

6. **Passives:** Decoupling caps (100nF on each VDD), pull-up resistors (4.7kΩ on I2C lines), series resistor for supercap charge limiting (100Ω).

7. **J1 — SWD debug header:** 2x5 pin 1.27mm header (or just test pads).

- [ ] **Step 3: Wire schematic**

Connect:
- All VDD pins to CR1225 positive (through supercap)
- Supercap in parallel with battery (through 100Ω charge-limiting resistor)
- I2C bus: SDA (P0.26) to U2.SDA and U3.SDA, with 4.7kΩ pull-up to VDD
- I2C bus: SCL (P0.27) to U2.SCL and U3.SCL, with 4.7kΩ pull-up to VDD
- Accelerometer INT1 (U3.INT1) to P0.11
- 100nF decoupling cap on each IC VDD pin to GND
- SWD: SWDIO, SWDCLK, VDD, GND to test pads

- [ ] **Step 4: Run ERC (Electrical Rules Check)**

In KiCad schematic editor: Inspect → Electrical Rules Checker → Run. Fix all errors.

- [ ] **Step 5: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add hardware/kicad/
git commit -m "feat: add KiCad schematic with all components wired"
```

---

### Task 11: KiCad PCB Layout

**Files:**
- Create: `hardware/kicad/open-tpms.kicad_pcb`
- Create: `hardware/kicad/fabrication/` (Gerbers, BOM, pick-and-place)

This task is done in KiCad GUI:

- [ ] **Step 1: Define board outline**

PCB shape: rectangular strip ~16mm x 40mm with rounded corners. Center hole for valve stem (~7mm diameter — fits standard Presta valve with room for seal).

- [ ] **Step 2: Place components**

- **Block A side (left of valve hole):** MDBT42Q module (antenna toward board edge, away from valve), LIS2DH12, supercapacitor, I2C pull-ups, decoupling caps.
- **Block B side (right of valve hole):** CR1225 battery holder, pressure sensor (port toward board edge for ePTFE membrane access), decoupling caps.
- **Center strip:** SWD test pads (between blocks, accessible for programming).

Keep antenna end of MDBT42Q at the board edge with ground plane keepout per Raytac guidelines.

- [ ] **Step 3: Route traces**

- I2C bus runs from Block A across the strip through Block B.
- Power traces (VDD, GND) run across the strip.
- Use ground pour on bottom layer for EMC.
- Follow Raytac antenna keepout guidelines (no copper/ground under antenna section of module).

- [ ] **Step 4: Run DRC (Design Rules Check)**

In PCB editor: Inspect → Design Rules Checker → Run. Fix all violations.

- [ ] **Step 5: Generate fabrication outputs**

File → Fabrication Outputs:
- Gerber files (all layers)
- Drill files (Excellon)
- BOM (CSV)
- Pick-and-place file (CSV)

Save to `hardware/kicad/fabrication/`.

- [ ] **Step 6: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add hardware/kicad/
git commit -m "feat: add PCB layout with component placement and routing"
```

---

### Task 12: Enclosure CAD Design

**Files:**
- Create: `hardware/enclosure/frame.step`
- Create: `hardware/enclosure/lid.step`
- Create: `hardware/enclosure/lid-pressure.step`
- Create: `hardware/enclosure/stl/` (STL exports)

This task is done in FreeCAD or Fusion 360:

- [ ] **Step 1: Design frame (walls only, no floor)**

Rectangular ring:
- Outer dimensions: 15mm x 16mm
- Wall thickness: 1.2mm
- Height: 3.3mm (Block B, tallest component height)
- Two corner posts with M1.2 holes for brass heat-set inserts (1.5mm OD hole)
- Bottom edge: flat for bonding to PCB with RTV silicone
- Top edge: O-ring groove (0.7mm deep x 1.2mm wide rectangular groove around perimeter)

- [ ] **Step 2: Design standard lid (Block A)**

Flat plate:
- 15mm x 16mm, 1.2mm thick
- Two M1.2 through-holes matching frame posts
- Slight chamfer on edges
- Countersunk screw holes so heads sit flush

- [ ] **Step 3: Design pressure lid (Block B)**

Same as standard lid plus:
- 2mm hole centered over pressure sensor location
- Recessed pocket on inner surface for ePTFE membrane bonding (3mm diameter, 0.3mm deep)

- [ ] **Step 4: Export STL files**

Export each part as STL to `hardware/enclosure/stl/`:
- `frame.stl`
- `lid.stl`
- `lid-pressure.stl`

- [ ] **Step 5: Test print**

Print all parts in ASA:
- Layer height: 0.1mm
- Infill: 100%
- Walls: 3 perimeters minimum
- Verify M1.2 brass inserts press in cleanly
- Verify O-ring groove dimensions with a test O-ring
- Verify frame sits flat on a test PCB (or piece of FR4)

- [ ] **Step 6: Commit**

```bash
cd /Users/simruelland/bike-tpms-project
git add hardware/enclosure/
git commit -m "feat: add enclosure CAD design (frame + lids) for 3D printing"
```

---

## Phase 4: Integration & Testing

### Task 13: Order Components and PCBs

This is a procurement task, not code:

- [ ] **Step 1: Order PCBs**

Upload Gerbers to JLCPCB or PCBWay:
- 2-layer, 0.8mm FR4, ENIG finish
- Order 10 minimum (typical MOQ)
- Add SMT assembly service if available for our components
- Estimated lead time: 5-10 business days

- [ ] **Step 2: Order components**

From DigiKey, Mouser, or LCSC:
- 10x Raytac MDBT42Q-512KV2
- 10x Honeywell ABP (150 PSI I2C variant — confirm exact P/N from Honeywell's configurator)
- 10x LIS2DH12TR
- 10x supercapacitors (47-100mF)
- 10x CR1225 battery holders
- Passive components (100nF caps, 4.7kΩ resistors, 100Ω resistors)
- M1.2 x 3mm stainless Allen screws
- M1.2 brass heat-set inserts
- Silicone O-rings (will need to source custom size or find closest standard)
- ePTFE membrane sheet (cut to size)
- Silicone conformal coating spray
- RTV silicone adhesive
- Loctite 222
- Silicone grease

- [ ] **Step 3: Order test equipment**

- Segger J-Link EDU Mini (~$60) or nRF52-DK board (~$40)
- Nordic Power Profiler Kit II (~$80)
- Calibrated digital pressure gauge

---

### Task 14: Assembly and First Boot

- [ ] **Step 1: Solder PCBs**

If not using SMT assembly service:
- Apply solder paste with stencil
- Place components (tweezers + microscope)
- Reflow in oven or with hot air station
- Inspect joints under magnification

- [ ] **Step 2: Flash SoftDevice**

```bash
# Using nrfjprog (part of nRF Command Line Tools)
nrfjprog --family NRF52 --eraseall
nrfjprog --family NRF52 --program s332_nrf52_7.0.1_softdevice.hex --sectorerase
nrfjprog --family NRF52 --reset
```

- [ ] **Step 3: Flash application firmware**

```bash
cd /Users/simruelland/bike-tpms-project/firmware
make
nrfjprog --family NRF52 --program _build/nrf52832_xxaa.hex --sectorerase
nrfjprog --family NRF52 --reset
```

- [ ] **Step 4: Verify basic operation**

- Open nRF Connect app on phone → scan → should see "OpenTPMS" device
- Connect → verify TPMS service appears with all characteristics
- Read pressure characteristic → should show a value (may be inaccurate without calibration)
- Open Web Bluetooth config page → connect → verify live data appears

- [ ] **Step 5: Assemble enclosure**

1. Apply conformal coating to exposed PCB strip between blocks (2-3 coats, dry between)
2. Press M1.2 brass inserts into frame walls with soldering iron
3. Bond frames to PCB with RTV silicone, align carefully, cure 24 hours
4. Bond ePTFE membrane into Block B lid
5. Insert CR1225 battery
6. Place O-rings in grooves
7. Screw down lids with Loctite 222
8. Dab silicone grease on screw hex sockets

- [ ] **Step 6: Run test procedures**

Execute all 8 test procedures from the design spec (Section 6):
1. Pressure accuracy
2. Seal integrity
3. RF range
4. Battery life measurement
5. Vibration (ride testing)
6. Flat detection
7. Cold weather
8. Insert compatibility

Document results in `docs/test-results/`.

- [ ] **Step 7: Commit test results**

```bash
cd /Users/simruelland/bike-tpms-project
git add docs/test-results/
git commit -m "docs: add initial test results from first prototype"
```
