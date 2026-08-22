// OpenTPMS — Calibration Correction Tests (host-compiled, see Makefile)

#include <stdio.h>
#include "calibration.h"

static int g_failures = 0;

#define CHECK_EQ(label, got, want)                                             \
    do {                                                                       \
        long long _g = (long long)(got), _w = (long long)(want);               \
        if (_g != _w) { printf("FAIL %-38s got %lld, want %lld\n", label, _g, _w); g_failures++; } \
        else         { printf("ok   %-38s %lld\n", label, _g); }               \
    } while (0)

#define CHECK(label, cond)                                                     \
    do { if (!(cond)) { printf("FAIL %s\n", label); g_failures++; }             \
         else         { printf("ok   %s\n", label); } } while (0)

int main(void)
{
    // Identity: no correction at all.
    calibration_t id = CALIBRATION_IDENTITY;
    CHECK_EQ("identity passes value through", calibration_apply(&id, 250000), 250000);
    CHECK_EQ("identity at zero",              calibration_apply(&id, 0), 0);
    CHECK_EQ("NULL cal passes through",       calibration_apply(NULL, 123456), 123456);

    // Pure offset: +150 Pa (1.5 mbar systematic offset).
    calibration_t off = { .a0 = 150, .a1_q16 = 65536, .a2_q30 = 0 };
    CHECK_EQ("offset-only", calibration_apply(&off, 250000), 250150);

    // Gain 1.01 (Q16: 1.01 * 65536 = 66191.36 -> 66191).
    // 250000 * 66191 / 65536 = 252498.9... -> floor 252498
    calibration_t gain = { .a0 = 0, .a1_q16 = 66191, .a2_q30 = 0 };
    CHECK_EQ("gain 1.01", calibration_apply(&gain, 250000), 252498);

    // Combined realistic correction: -80 Pa offset, gain 0.998, tiny quadratic.
    // a1 = 0.998*65536 = 65404.9 -> 65404;  a2 = 20 (Q30: 20/2^30 = 1.86e-8 /Pa)
    // linear: 250000*65404 >> 16 = 16351000000 / 65536 = 249496.4 -> 249496
    // quad:   20 * 250000^2 >> 30 = 1.25e12*... = 20*6.25e10 = 1.25e12 / 2^30 = 1164.15 -> 1164
    // total:  -80 + 249496 + 1164 = 250580
    calibration_t real = { .a0 = -80, .a1_q16 = 65404, .a2_q30 = 20 };
    CHECK_EQ("realistic combined", calibration_apply(&real, 250000), 250580);

    // Monotonicity: sane corrections must never invert ordering of pressures.
    int32_t prev = calibration_apply(&real, 80000);
    int mono_ok = 1;
    for (int32_t p = 90000; p <= 3000000; p += 10000)
    {
        int32_t c = calibration_apply(&real, p);
        if (c <= prev) { mono_ok = 0; break; }
        prev = c;
    }
    CHECK("monotonic over full 30-bar range", mono_ok);

    // Sanity gate: identity is sane; garbage (erased flash = 0xFFFFFFFF) is not.
    CHECK("identity is sane", calibration_is_sane(&id));
    calibration_t erased = { .a0 = -1, .a1_q16 = -1, .a2_q30 = -1 };
    CHECK("erased-flash pattern rejected", !calibration_is_sane(&erased));
    calibration_t wild = { .a0 = 200000, .a1_q16 = 65536, .a2_q30 = 0 };
    CHECK("wild offset rejected", !calibration_is_sane(&wild));
    calibration_t half_gain = { .a0 = 0, .a1_q16 = 32768, .a2_q30 = 0 };
    CHECK("gain 0.5 rejected", !calibration_is_sane(&half_gain));
    CHECK("NULL rejected", !calibration_is_sane(NULL));

    if (g_failures) { printf("\n%d FAILURE(S)\n", g_failures); return 1; }
    printf("\nALL TESTS PASSED\n");
    return 0;
}
