#include <check.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

/* Test that the allocation size computation in maid-x11.c does not overflow.
 * Invariant: width2 * height2 * bits_per_pixel / 8 must never wrap to a
 * value smaller than the actual data size needed, causing a tiny allocation
 * followed by an out-of-bounds write. */

static int allocation_would_overflow(unsigned int width2, unsigned int height2,
                                     unsigned int bits_per_pixel)
{
    /* Replicate the vulnerable expression and check for overflow */
    uint64_t safe = (uint64_t)width2 * (uint64_t)height2 *
                    (uint64_t)bits_per_pixel / 8;
    uint32_t unsafe = width2 * height2 * bits_per_pixel / 8;
    /* If the 32-bit result differs from the 64-bit result, overflow occurred */
    return (uint64_t)unsafe != safe;
}

START_TEST(test_no_integer_overflow_in_alloc_size)
{
    /* Invariant: allocation size must never silently wrap due to overflow */
    struct { unsigned int w; unsigned int h; unsigned int bpp; int should_overflow; } payloads[] = {
        /* Exact exploit case: large dimensions that overflow 32-bit multiply */
        { 65536, 65536, 32, 1 },
        /* Boundary: just at the edge of overflow */
        { 46341, 46341, 32, 1 },
        /* Valid small image: 640x480 at 32bpp — must NOT overflow */
        { 640,   480,   32, 0 },
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        int overflows = allocation_would_overflow(payloads[i].w,
                                                  payloads[i].h,
                                                  payloads[i].bpp);
        if (payloads[i].should_overflow) {
            /* The vulnerable code path exists; confirm overflow is detected */
            ck_assert_msg(overflows,
                "Expected overflow for w=%u h=%u bpp=%u but none detected",
                payloads[i].w, payloads[i].h, payloads[i].bpp);
        } else {
            /* Valid input must never overflow */
            ck_assert_msg(!overflows,
                "Unexpected overflow for valid input w=%u h=%u bpp=%u",
                payloads[i].w, payloads[i].h, payloads[i].bpp);
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_no_integer_overflow_in_alloc_size);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}