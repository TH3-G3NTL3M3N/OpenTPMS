// OpenTPMS — MS5837-30BA Pressure Sensor Driver
// Math layer (this section): pure, host-testable. See tests/test_ms5837.c.

#include "ms5837.h"

// CRC4 over the PROM contents, algorithm from the TE MS5837 datasheet.
// The 4 CRC bits live in prom[0] bits 15:12 and are masked out of the calculation.
static uint8_t crc4(const uint16_t prom[MS5837_PROM_WORDS])
{
    uint16_t n_prom[8];
    for (int i = 0; i < (int)MS5837_PROM_WORDS; i++)
    {
        n_prom[i] = prom[i];
    }
    n_prom[0] &= 0x0FFF;   // CRC nibble replaced by 0
    n_prom[7]  = 0;        // subsidiary word per datasheet

    uint16_t n_rem = 0;
    for (int cnt = 0; cnt < 16; cnt++)
    {
        if (cnt % 2 == 1)
        {
            n_rem ^= (uint16_t)(n_prom[cnt >> 1] & 0x00FF);
        }
        else
        {
            n_rem ^= (uint16_t)(n_prom[cnt >> 1] >> 8);
        }
        for (int n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & 0x8000)
            {
                n_rem = (uint16_t)((n_rem << 1) ^ 0x3000);
            }
            else
            {
                n_rem = (uint16_t)(n_rem << 1);
            }
        }
    }
    return (uint8_t)((n_rem >> 12) & 0x000F);
}

bool ms5837_prom_crc_ok(const uint16_t prom[MS5837_PROM_WORDS])
{
    uint8_t expected = (uint8_t)((prom[0] >> 12) & 0x000F);
    return crc4(prom) == expected;
}

void ms5837_cal_from_prom(const uint16_t prom[MS5837_PROM_WORDS], ms5837_cal_t * p_cal)
{
    p_cal->c1_sens     = prom[1];
    p_cal->c2_off      = prom[2];
    p_cal->c3_tcs      = prom[3];
    p_cal->c4_tco      = prom[4];
    p_cal->c5_tref     = prom[5];
    p_cal->c6_tempsens = prom[6];
}

void ms5837_compensate(const ms5837_cal_t * p_cal,
                       uint32_t             d1_raw,
                       uint32_t             d2_raw,
                       int32_t            * p_pressure,
                       int32_t            * p_temperature)
{
    // First order (datasheet, MS5837-30BA variant).
    // Arithmetic right shift == floor division; required to match reference vectors.
    int64_t dt   = (int64_t)d2_raw - ((int64_t)p_cal->c5_tref << 8);
    int64_t temp = 2000 + ((dt * p_cal->c6_tempsens) >> 23);              // 0.01 degC
    int64_t off  = ((int64_t)p_cal->c2_off << 16) + ((p_cal->c4_tco * dt) >> 7);
    int64_t sens = ((int64_t)p_cal->c1_sens << 15) + ((p_cal->c3_tcs * dt) >> 8);

    // Second order.
    int64_t ti, offi, sensi;
    if (temp < 2000)                           // low temperature (< 20.00 degC)
    {
        ti    = (3 * dt * dt) >> 33;
        offi  = (3 * (temp - 2000) * (temp - 2000)) / 2;
        sensi = (5 * (temp - 2000) * (temp - 2000)) / 8;
        if (temp < -1500)                      // very low temperature (< -15.00 degC)
        {
            offi  += 7 * (temp + 1500) * (temp + 1500);
            sensi += 4 * (temp + 1500) * (temp + 1500);
        }
    }
    else                                       // high temperature
    {
        ti    = (2 * dt * dt) >> 37;
        offi  = ((temp - 2000) * (temp - 2000)) / 16;
        sensi = 0;
    }

    int64_t off2  = off - offi;
    int64_t sens2 = sens - sensi;
    int64_t p     = ((((int64_t)d1_raw * sens2) >> 21) - off2) >> 13;      // 0.1 mbar

    *p_temperature = (int32_t)(temp - ti);      // 0.01 degC
    *p_pressure    = (int32_t)(p * 10);         // 0.01 mbar (== Pa)
}

// ---------------------------------------------------------------------------
// Bus layer — TODO (implementation-plan Task 2, step 3)
// TWIM (I2C with EasyDMA) transactions against the real sensor:
//   - reset command (0x1E), PROM reads (0xA0..0xAC)
//   - D1/D2 conversions (OSR selection) + ADC read, with conversion delays
//   - top-level: ms5837_init() -> PROM + CRC + cal; ms5837_read() -> compensate
// Requires nRF5 SDK headers; excluded from host unit tests by design.
// ---------------------------------------------------------------------------
