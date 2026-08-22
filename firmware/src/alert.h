// OpenTPMS — Temperature-Compensated Flat Detection
// Ideal gas law on a fixed-volume tire: P_abs / T_abs is constant, so the
// expected pressure at the current temperature is
//     P_expected = P_baseline * T_now_K / T_baseline_K
// An alert fires only when measured pressure falls below expected by more than
// the user threshold — pressure changes explained by temperature never alarm.
// Pure logic, host-testable; hardware and radio layers live elsewhere.

#ifndef OPENTPMS_ALERT_H
#define OPENTPMS_ALERT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ALERT_NONE = 0,        // pressure consistent with baseline + temperature
    ALERT_LOW_PRESSURE,    // deficit exceeded threshold — probable leak
} alert_status_t;

typedef struct
{
    int32_t baseline_pressure;   // Pa (absolute), captured when tire known-good
    int32_t baseline_temp;       // 0.01 degC at capture time
    int32_t threshold;           // Pa of unexplained deficit that triggers (user config)
    bool    active;              // current alert state (hysteresis lives here)
} alert_ctx_t;

// (Re)arm with a known-good reading. Threshold in Pa; design default 10000
// (100 mbar ~ 1.45 PSI). Clears any active alert.
void alert_baseline_set(alert_ctx_t * p_ctx,
                        int32_t       pressure,
                        int32_t       temp,
                        int32_t       threshold);

// Expected absolute pressure at temp_now given the baseline (ideal gas law).
// Temperatures in 0.01 degC; Kelvin conversion handled internally.
int32_t alert_expected_pressure(const alert_ctx_t * p_ctx, int32_t temp_now);

// Evaluate one reading. Sets and clears ctx->active with hysteresis:
// fires at deficit > threshold, clears only when deficit < threshold/2.
alert_status_t alert_evaluate(alert_ctx_t * p_ctx,
                              int32_t       pressure_now,
                              int32_t       temp_now);

#ifdef __cplusplus
}
#endif

#endif // OPENTPMS_ALERT_H
