// OpenTPMS — Factory Calibration Correction
// Applies a per-sensor correction polynomial to compensated pressure readings.
// The polynomial is fitted at factory calibration time (tools/calibrate.py)
// from 3-5 reference pressures and stored in flash (storage.c). This module
// only APPLIES it — pure math, host-testable.

#ifndef OPENTPMS_CALIBRATION_H
#define OPENTPMS_CALIBRATION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// corrected = a0 + a1*p + a2*p^2, evaluated in fixed point.
//   p, a0:  pascals (== 0.01 mbar, the unit ms5837.c outputs)
//   a1:     Q16.16 — 65536 represents gain 1.0
//   a2:     Q2.30  — per-pascal curvature, ~0 for a healthy sensor
// Identity (no correction): { .a0 = 0, .a1_q16 = 65536, .a2_q30 = 0 }
typedef struct
{
    int32_t a0;
    int32_t a1_q16;
    int32_t a2_q30;
} calibration_t;

#define CALIBRATION_IDENTITY  { 0, 65536, 0 }

// Plausibility check for coefficients loaded from flash (rejects erased/garbage
// flash): |a0| <= 50000 Pa (5 bar of offset is nonsense), gain within 20% of
// unity, |a2| small. Returns true if the set is sane to apply.
bool calibration_is_sane(const calibration_t * p_cal);

// Apply the correction. Falls back to identity if p_cal is NULL.
// Input/output in pascals; intermediate math in int64.
int32_t calibration_apply(const calibration_t * p_cal, int32_t pressure);

#ifdef __cplusplus
}
#endif

#endif // OPENTPMS_CALIBRATION_H
