// OpenTPMS — Factory Calibration Correction
// See calibration.h. Pure math; unit tests in tests/test_calibration.c.

#include <stddef.h>
#include "calibration.h"

bool calibration_is_sane(const calibration_t * p_cal)
{
    if (p_cal == NULL)
    {
        return false;
    }
    if (p_cal->a0 < -50000 || p_cal->a0 > 50000)          // |offset| <= 500 mbar
    {
        return false;
    }
    if (p_cal->a1_q16 < 52429 || p_cal->a1_q16 > 78643)   // gain 0.8 .. 1.2
    {
        return false;
    }
    if (p_cal->a2_q30 < -1024 || p_cal->a2_q30 > 1024)    // tiny curvature only
    {
        return false;
    }
    return true;
}

int32_t calibration_apply(const calibration_t * p_cal, int32_t pressure)
{
    if (p_cal == NULL)
    {
        return pressure;
    }

    int64_t p = pressure;

    // a1 * p, Q16 -> integer (arithmetic shift = floor, consistent with ms5837.c)
    int64_t linear = ((int64_t)p_cal->a1_q16 * p) >> 16;

    // a2 * p^2, Q30 -> integer. p^2 <= ~9e12 for 30 bar; * |a2| <= 1024 stays
    // well inside int64.
    int64_t quad = (((int64_t)p_cal->a2_q30 * p * p) >> 30);

    int64_t out = (int64_t)p_cal->a0 + linear + quad;

    // Clamp to int32 range defensively.
    if (out > INT32_MAX) { out = INT32_MAX; }
    if (out < INT32_MIN) { out = INT32_MIN; }
    return (int32_t)out;
}
