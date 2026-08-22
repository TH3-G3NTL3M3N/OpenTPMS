// OpenTPMS — MS5837-30BA Pressure Sensor Driver
// Math layer: pure functions, no hardware dependencies — unit-tested on host.
// Bus layer (TWIM/I2C): see TODO section in ms5837.c, implemented against board hardware.

#ifndef OPENTPMS_MS5837_H
#define OPENTPMS_MS5837_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MS5837_I2C_ADDR          0x76u
#define MS5837_PROM_WORDS        7u      // word 0 = CRC(15:12)+factory, words 1..6 = C1..C6

// Factory calibration coefficients read from PROM words 1..6.
typedef struct
{
    uint16_t c1_sens;       // Pressure sensitivity
    uint16_t c2_off;        // Pressure offset
    uint16_t c3_tcs;        // Temperature coefficient of pressure sensitivity
    uint16_t c4_tco;        // Temperature coefficient of pressure offset
    uint16_t c5_tref;       // Reference temperature
    uint16_t c6_tempsens;   // Temperature coefficient of the temperature
} ms5837_cal_t;

// Verify the 4-bit CRC embedded in PROM word 0 (bits 15:12) per TE datasheet.
// prom: the 7 words as read from the device. Returns true if CRC matches.
bool ms5837_prom_crc_ok(const uint16_t prom[MS5837_PROM_WORDS]);

// Convenience: unpack PROM words 1..6 into a calibration struct.
void ms5837_cal_from_prom(const uint16_t prom[MS5837_PROM_WORDS], ms5837_cal_t * p_cal);

// First + second order temperature compensation per TE MS5837-30BA datasheet.
//   d1_raw: 24-bit uncompensated pressure conversion
//   d2_raw: 24-bit uncompensated temperature conversion
//   p_pressure: absolute pressure, units of 0.01 mbar (== pascal). 30 bar = 3,000,000.
//   p_temperature: units of 0.01 degC (2000 = 20.00 degC)
// Divisions by powers of two are arithmetic right shifts (floor), matching the
// datasheet reference code and the Python-generated test vectors.
void ms5837_compensate(const ms5837_cal_t * p_cal,
                       uint32_t             d1_raw,
                       uint32_t             d2_raw,
                       int32_t            * p_pressure,
                       int32_t            * p_temperature);

#ifdef __cplusplus
}
#endif

#endif // OPENTPMS_MS5837_H
