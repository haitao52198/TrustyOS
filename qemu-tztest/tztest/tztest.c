#include "syscntl.h"
#include "interop.h"
#include "svc.h"
#include "libcflat.h"
#include "tztest_internal.h"

tztest_case_t tztest[256];
uint32_t tztest_num = 0;

#define TEST(_f, _e, _s, _a) \
    {.fid = (_f), .el = (_e), .state = (_s), .arg = (_a)}
#define TEST_END TEST(0, 0, 0, 0)

tztest_case_t tztest[] = {
    TEST(TZTEST_SMC, EL0, NONSECURE, 0),
    TEST(TZTEST_SMC, EL0, SECURE, 0),
    TEST(TZTEST_REG_ACCESS, EL0, NONSECURE, 0),
    TEST(TZTEST_REG_ACCESS, EL0, SECURE, 0),

#ifdef AARCH64
    TEST(TZTEST_CPACR_TRAP, EL0, NONSECURE, 0),
    TEST(TZTEST_CPACR_TRAP, EL0, SECURE, 0),
    TEST(TZTEST_WFX_TRAP, EL0, NONSECURE, 0),
    TEST(TZTEST_WFX_TRAP, EL0, SECURE, 0),
    TEST(TZTEST_FP_TRAP, EL0, NONSECURE, 0),
    TEST(TZTEST_FP_TRAP, EL0, SECURE, 0),
#endif

    TEST(TZTEST_SMC, EL1, NONSECURE, 0),
    TEST(TZTEST_SMC, EL1, SECURE, 0),
    TEST(TZTEST_REG_ACCESS, EL1, NONSECURE, 0),
    TEST(TZTEST_REG_ACCESS, EL1, SECURE, 0),

#ifdef AARCH32
    TEST(TZTEST_MASK_BITS, EL1, NONSECURE, 0),
    TEST(TZTEST_SMC, EL3, SECURE, 0),
#endif
#ifdef AARCH64
    TEST(TZTEST_CPACR_TRAP, EL1, NONSECURE, 0),
    TEST(TZTEST_CPACR_TRAP, EL1, SECURE, 0),
    TEST(TZTEST_WFX_TRAP, EL1, NONSECURE, 0),
    TEST(TZTEST_WFX_TRAP, EL1, SECURE, 0),
    TEST(TZTEST_FP_TRAP, EL1, NONSECURE, 0),
    TEST(TZTEST_FP_TRAP, EL1, SECURE, 0),
#endif
    TEST_END
};

void interop_test()
{
    op_test_t test;

    test.orig = test.val = 1024;
    test.fail = test.count = 0;
    printf("\nValidating interop communication between ELs... ");
    __svc(SVC_OP_TEST, (svc_op_desc_t *)&test);
    TEST_CONDITION(!test.fail && test.val == (test.orig >> test.count));
}

void run_test(uint32_t fid, uint32_t el, uint32_t state, uint32_t arg)
{
    op_dispatch_t disp;

    if (el == exception_level && state == secure_state) {
        if (test_func[fid]) {
            test_func[fid](arg);
        }
    } else {
        disp.fid = fid;
        disp.el = el;
        disp.state = state;
        disp.arg = arg;
        __svc(SVC_OP_DISPATCH, (svc_op_desc_t *)&disp);
    }
}

void tztest_start()
{
    uint32_t i = 0;

    printf("Starting TZ test...\n");

    /* Test EL to EL communication */
    interop_test();

    while (tztest[i].fid != 0) {
        run_test(tztest[i].fid, tztest[i].el, tztest[i].state, tztest[i].arg);
        i++;
    }

    printf("\nValidation complete.  Passed %d of %d tests.\n",
              syscntl->test_cntl->test_count - syscntl->test_cntl->fail_count,
              syscntl->test_cntl->test_count);
}

