// OpenTPMS — Flat Detection Logic Tests (host-compiled, see Makefile)
// The scenarios mirror real riding: the module must stay silent through
// temperature swings and fire on genuine leaks — including leaks partially
// masked by warming air.

#include <stdio.h>
#include "alert.h"

static int g_failures = 0;

#define CHECK_EQ(label, got, want)                                             \
    do {                                                                       \
        long long _g = (long long)(got), _w = (long long)(want);               \
        if (_g != _w) { printf("FAIL %-44s got %lld, want %lld\n", label, _g, _w); g_failures++; } \
        else         { printf("ok   %-44s %lld\n", label, _g); }               \
    } while (0)

// Baseline everywhere: 2.5 bar absolute (250000 Pa) at 20.00 degC, 100 mbar threshold.
static alert_ctx_t fresh_ctx(void)
{
    alert_ctx_t ctx;
    alert_baseline_set(&ctx, 250000, 2000, 10000);
    return ctx;
}

int main(void)
{
    alert_ctx_t ctx;

    // --- expected-pressure math (hand-verified) ---
    ctx = fresh_ctx();
    // 30.00C: 250000 * 30315 / 29315 = 258528 (floor)
    CHECK_EQ("expected @ +30C", alert_expected_pressure(&ctx, 3000), 258528);
    // 5.00C:  250000 * 27815 / 29315 = 237207 (floor of 237207.91)
    CHECK_EQ("expected @ +5C", alert_expected_pressure(&ctx, 500), 237207);
    // same temp = same pressure
    CHECK_EQ("expected @ baseline temp", alert_expected_pressure(&ctx, 2000), 250000);

    // --- scenario: nothing happening ---
    ctx = fresh_ctx();
    CHECK_EQ("steady state stays quiet", alert_evaluate(&ctx, 250000, 2000), ALERT_NONE);

    // --- scenario: cold morning (the false-positive killer) ---
    // Temperature fell 15C; pressure fell exactly as gas law predicts.
    // Every dumb-threshold competitor alarms here. We must not.
    ctx = fresh_ctx();
    CHECK_EQ("gas-law pressure drop stays quiet", alert_evaluate(&ctx, 237208, 500), ALERT_NONE);
    // Even 5 mbar below prediction is inside threshold.
    CHECK_EQ("small extra deficit stays quiet", alert_evaluate(&ctx, 236708, 500), ALERT_NONE);

    // --- scenario: genuine slow leak at constant temperature ---
    ctx = fresh_ctx();
    CHECK_EQ("20 kPa loss fires", alert_evaluate(&ctx, 230000, 2000), ALERT_LOW_PRESSURE);

    // --- scenario: leak partially masked by warming ---
    // Tire warmed to 30C (expected 258528) but reads only 245000: 13.5 kPa
    // unexplained deficit even though the reading is only 5 kPa under baseline.
    ctx = fresh_ctx();
    CHECK_EQ("warming-masked leak fires", alert_evaluate(&ctx, 245000, 3000), ALERT_LOW_PRESSURE);

    // --- hysteresis: no flapping around the threshold ---
    ctx = fresh_ctx();
    alert_evaluate(&ctx, 235000, 2000);                                     // fire (15 kPa)
    CHECK_EQ("still active above thr/2", alert_evaluate(&ctx, 242000, 2000), ALERT_LOW_PRESSURE); // 8 kPa deficit
    CHECK_EQ("clears below thr/2", alert_evaluate(&ctx, 246000, 2000), ALERT_NONE);               // 4 kPa deficit
    CHECK_EQ("stays clear after clearing", alert_evaluate(&ctx, 246000, 2000), ALERT_NONE);

    // --- re-arming clears an active alert ---
    ctx = fresh_ctx();
    alert_evaluate(&ctx, 230000, 2000);
    alert_baseline_set(&ctx, 230000, 2000, 10000);   // rider re-armed at new pressure
    CHECK_EQ("re-baseline clears alert", alert_evaluate(&ctx, 230000, 2000), ALERT_NONE);

    // --- overpressure never alarms (pump-up, hot day) ---
    ctx = fresh_ctx();
    CHECK_EQ("overpressure stays quiet", alert_evaluate(&ctx, 300000, 2000), ALERT_NONE);

    if (g_failures) { printf("\n%d FAILURE(S)\n", g_failures); return 1; }
    printf("\nALL TESTS PASSED\n");
    return 0;
}
