#include "tztest_internal.h"
#include "tztest_el0.h"

tztest_t test_func[] = {
    [TZTEST_SMC] = el0_check_smc,
    [TZTEST_REG_ACCESS] = el0_check_register_access,
#ifdef AARCH64
    [TZTEST_CPACR_TRAP] = el0_check_cpacr_trap,
    [TZTEST_WFX_TRAP] = el0_check_wfx_trap,
    [TZTEST_FP_TRAP] = el0_check_fp_trap,
#endif
};

