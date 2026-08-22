// OpenTPMS — Temperature-Compensated Flat Detection
// See alert.h. Pure logic; unit tests in tests/test_alert.c.

#include <stddef.h>
#include "alert.h"

// 0.01 degC -> 0.01 K (both scaled x100, so one integer add)
#define CENTI_KELVIN(centi_c)   ((int64_t)(centi_c) + 27315)

void alert_baseline_set(alert_ctx_t * p_ctx,
                        int32_t       pressure,
                        int32_t       temp,
                        int32_t       threshold)
{
    p_ctx->baseline_pressure = pressure;
    p_ctx->baseline_temp     = temp;
    p_ctx->threshold         = threshold;
    p_ctx->active            = false;
}

int32_t alert_expected_pressure(const alert_ctx_t * p_ctx, int32_t temp_now)
{
    int64_t t_now_ck  = CENTI_KELVIN(temp_now);
    int64_t t_base_ck = CENTI_KELVIN(p_ctx->baseline_temp);

    if (t_base_ck <= 0 || t_now_ck <= 0)
    {
        // Physically impossible input; fail safe by expecting the baseline.
        return p_ctx->baseline_pressure;
    }

    // P * T ratio: worst case ~3e6 Pa * ~40000 cK ~ 1.2e11 — comfortably int64.
    return (int32_t)(((int64_t)p_ctx->baseline_pressure * t_now_ck) / t_base_ck);
}

alert_status_t alert_evaluate(alert_ctx_t * p_ctx,
                              int32_t       pressure_now,
                              int32_t       temp_now)
{
    int32_t expected = alert_expected_pressure(p_ctx, temp_now);
    int32_t deficit  = expected - pressure_now;   // positive = missing air

    if (p_ctx->active)
    {
        // Hysteresis: stay alerted until the deficit halves below threshold.
        if (deficit < p_ctx->threshold / 2)
        {
            p_ctx->active = false;
        }
    }
    else if (deficit > p_ctx->threshold)
    {
        p_ctx->active = true;
    }

    return p_ctx->active ? ALERT_LOW_PRESSURE : ALERT_NONE;
}
