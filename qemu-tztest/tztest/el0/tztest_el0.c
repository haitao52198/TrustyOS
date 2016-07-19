#include "libcflat.h"
#include "svc.h"
#include "syscntl.h"
#include "builtins.h"
#include "exception.h"
#include "state.h"
#include "cpu.h"
#include "debug.h"
#include "tztest_internal.h"

uint32_t el0_check_smc(uint32_t __attribute__((unused))arg)
{
    TEST_HEAD("smc behavior");

    TEST_MSG("SMC call");
    TEST_EL1_EXCEPTION(asm volatile("smc #0\n"), EC_UNKNOWN);

    return 0;
}

uint32_t el0_check_register_access(uint32_t __attribute__((unused))arg)
{
    /* Set things to non-secure P1 and attempt accesses */
    TEST_HEAD("restricted register access");

    TEST_MSG("SCR read");
    TEST_EL1_EXCEPTION(READ_SCR(), EC_UNKNOWN);

    TEST_MSG("SCR write");
    TEST_EL1_EXCEPTION(WRITE_SCR(0), EC_UNKNOWN);

    TEST_MSG("SDER read");
    TEST_EL1_EXCEPTION(READ_SDER(), EC_UNKNOWN);

    TEST_MSG("SDER write");
    TEST_EL1_EXCEPTION(WRITE_SDER(0), EC_UNKNOWN);

#ifdef AARCH32
    TEST_MSG("MVBAR read");
    TEST_EL1_EXCEPTION(READ_MVBAR(), EC_UNKNOWN);

    TEST_MSG("MVBAR write");
    TEST_EL1_EXCEPTION(WRITE_MVBAR(0), EC_UNKNOWN);

    TEST_MSG("NSACR write");
    TEST_EL1_EXCEPTION(WRITE_NSACR(0), EC_UNKNOWN);
#endif

#ifdef AARCH64
    TEST_MSG("CPTR_EL3 read");
    TEST_EL1_EXCEPTION(READ_CPTR_EL3(), EC_UNKNOWN);

    TEST_MSG("CPTR_EL3 write");
    TEST_EL1_EXCEPTION(WRITE_CPTR_EL3(0), EC_UNKNOWN);
#endif

    return 0;
}

#ifdef AARCH64
uint32_t el0_check_cpacr_trap(uint32_t __attribute__((unused))arg)
{
    uint64_t cptr_el3, cpacr;

    TEST_HEAD("CPACR trapping");

    /* Get the current CPTR so we can restore it later */
    SVC_GET_REG(CPTR_EL3, 3, cptr_el3);

    /* Disable CPACR access */
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TCPAC);

    /* Try to read CPACR */
    TEST_MSG("Read of disabled CPACR");
    TEST_EL3_EXCEPTION(SVC_GET_REG(CPACR, 1, cpacr), EC_SYSINSN);

    /* Try to write CPACR */
    TEST_MSG("Write of disabled CPACR");
    TEST_EL3_EXCEPTION(SVC_SET_REG(CPACR, 1, cpacr), EC_SYSINSN);

    /* Restore the original CPTR */
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3);

    return 0;
}

uint32_t el0_check_fp_trap(uint32_t __attribute__((unused))arg)
{
#ifdef DEBUG
    /* This test cannot be run easily when debug is enabled as messing with the
     * floating-point enable disables printing.  It is easier to disable
     * debugging than change the debugging to support floating-point enable, so
     * the test is disabled.
     */
    TEST_HEAD("FP trapping test disabled during debug");
#else

    /* Disabling floating-point also disables printing and causes hangs if you
     * try to print.  For this reason not all standard test macros can be used
     * so instead checking and printing status is pulled out so printing can be
     * reenabled ahead of time.
     */
    uint64_t cptr_el3, cpacr;

    TEST_HEAD("FP trapping");

    /* Get the current CPTR so we can restore it later */
    SVC_GET_REG(CPTR_EL3, 3, cptr_el3);
    SVC_GET_REG(CPACR, 1, cpacr);

    /* First check that enabled FP does not give us an exception */
    TEST_MSG("FP enabled (CPACR.FPEN = 3, CPTR_EL3 = 0)");
    SVC_SET_REG(CPACR, 1, cpacr | CPACR_FPEN(3));
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3 & ~CPTR_TFP);
    TEST_NO_EXCEPTION(asm volatile("fcmp s0, #0.0\n"));

    /* Disable CPACR FP access */
    TEST_MSG("FP disabled (CPACR.FPEN = 0, CPTR_EL3 = 0)");
    SVC_SET_REG(CPACR, 1, cpacr & ~(CPACR_FPEN(3)));
    TEST_ENABLE_EXCP_LOG();
    asm volatile("fcmp s0, #0.0\n");
    SVC_SET_REG(CPACR, 1, cpacr);           /* Reenable printing */
    if (syscntl->excp.taken && syscntl->excp.el == 1 &&
        syscntl->excp.ec == EC_SIMD) {
        TEST_MSG_SUCCESS();
    } else {
        TEST_MSG_FAILURE();
        INC_FAIL_COUNT();
    }
    INC_TEST_COUNT();
    TEST_EXCP_RESET();

    TEST_MSG("FP disabled (CPACR.FPEN = 1, CPTR_EL3 = 0)");
    SVC_SET_REG(CPACR, 1, cpacr & ~(CPACR_FPEN(2)));
    TEST_ENABLE_EXCP_LOG();
    asm volatile("fcmp s0, #0.0\n");
    SVC_SET_REG(CPACR, 1, cpacr);           /* Reenable printing */
    if (syscntl->excp.taken && syscntl->excp.el == 1 &&
        syscntl->excp.ec == EC_SIMD) {
        TEST_MSG_SUCCESS();
    } else {
        TEST_MSG_FAILURE();
        INC_FAIL_COUNT();
    }
    INC_TEST_COUNT();
    TEST_EXCP_RESET();

    TEST_MSG("FP disabled (CPACR.FPEN = 2, CPTR_EL3 = 0)");
    SVC_SET_REG(CPACR, 1, cpacr & ~(CPACR_FPEN(1)));
    TEST_ENABLE_EXCP_LOG();
    asm volatile("fcmp s0, #0.0\n");
    SVC_SET_REG(CPACR, 1, cpacr);           /* Reenable printing */
    if (syscntl->excp.taken && syscntl->excp.el == 1 &&
        syscntl->excp.ec == EC_SIMD) {
        TEST_MSG_SUCCESS();
    } else {
        TEST_MSG_FAILURE();
        INC_FAIL_COUNT();
    }
    INC_TEST_COUNT();
    TEST_EXCP_RESET();

    TEST_MSG("FP disabled (CPACR.FPEN = 0, CPTR_EL3 = 1)");
    SVC_SET_REG(CPACR, 1, cpacr & ~(CPACR_FPEN(3)));
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TFP);
    TEST_ENABLE_EXCP_LOG();
    asm volatile("fcmp s0, #0.0\n");
    SVC_SET_REG(CPACR, 1, cpacr);           /* Reenable printing */
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3);     /* Reenable printing */
    if (syscntl->excp.taken && syscntl->excp.el == 1 &&
        syscntl->excp.ec == EC_SIMD) {
        TEST_MSG_SUCCESS();
    } else {
        TEST_MSG_FAILURE();
        INC_FAIL_COUNT();
    }
    INC_TEST_COUNT();
    TEST_EXCP_RESET();

    TEST_MSG("FP disabled (CPACR.FPEN = 3, CPTR_EL3 = 1)");
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TFP);
    TEST_ENABLE_EXCP_LOG();
    asm volatile("fcmp s0, #0.0\n");
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3);     /* Reenable printing */
    if (syscntl->excp.taken && syscntl->excp.el == 3 &&
        syscntl->excp.ec == EC_SIMD) {
        TEST_MSG_SUCCESS();
    } else {
        TEST_MSG_FAILURE();
        INC_FAIL_COUNT();
    }
    INC_TEST_COUNT();
    TEST_EXCP_RESET();

    /* Restore the original CPACR*/
    SVC_SET_REG(CPACR, 1, cpacr);

    /* Restore the original CPTR */
    SVC_SET_REG(CPTR_EL3, 3, cptr_el3);
#endif

    return 0;
}

uint32_t el0_check_wfx_trap(uint32_t __attribute__((unused))arg)
{
    uint64_t sctlr, scr;

    TEST_HEAD("WFx traps");

    /* Get the current SCR so we can restore it later */
    SVC_GET_REG(SCR, 3, scr);

    /* Get the current SCTLR so we can restore it later */
    SVC_GET_REG(SCTLR, 1, sctlr);

#if 0
    /* Architecturally there is no guarantee that executing a WFE
     * will cause it to trap -- the trap only happens if the WFE would
     * put the CPU into a "power saving state", so a NOP implementation
     * can validly not trap at all. Since QEMU's implementation of WFE
     * does exactly this (though our WFI does not), disable the WFE tests.
     */

    /* Clear SCTLR.nTWE to cause WFE instructions to trap to EL1 */
    SVC_SET_REG(SCTLR, 1, sctlr & ~SCTLR_nTWE);
    TEST_MSG("WFE (SCTLR.nTWE clear, SCR.WFE clear)");
    TEST_EL1_EXCEPTION(asm volatile("wfe\n"), EC_WFI_WFE);

    /* Even though we set the SCR to trap WFE instructions to EL3, precedence
     * should be still given to EL1 as log as SCTLR.nTWE is clear.
     */
    SVC_SET_REG(SCR, 3, scr | SCR_TWE);
    TEST_MSG("WFE (SCTLR.nTWE clear, SCR.WFE set)");
    TEST_EL1_EXCEPTION(asm volatile("wfe\n"), EC_WFI_WFE);

    /* Now we should trap to EL3 with SCTLR.nTWE set */
    SVC_SET_REG(SCTLR, 1, sctlr | SCTLR_nTWE);
    TEST_MSG("WFE (SCTLR.nTWE set, SCR.WFE set)");
    TEST_EL3_EXCEPTION(asm volatile("wfe\n"), EC_WFI_WFE);
#endif

    /* Restore SCR */
    SVC_SET_REG(SCR, 3, scr);

    /* Clear SCTLR.nTWI to cause WFI instructions to trap to EL1 */
    SVC_SET_REG(SCTLR, 1, sctlr & ~SCTLR_nTWI);
    TEST_MSG("WFI (SCTLR.nTWI clear, SCR.WFI clear)");
    TEST_EL1_EXCEPTION(asm volatile("wfi\n"), EC_WFI_WFE);

    /* Even though we set the SCR to trap WFI instructions to EL3, precedence
     * should be still given to EL1 as log as SCTLR.nTWI is clear.
     */
    SVC_SET_REG(SCR, 3, scr | SCR_TWI);
    TEST_MSG("WFI (SCTLR.nTWI clear, SCR.WFI set)");
    TEST_EL1_EXCEPTION(asm volatile("wfi\n"), EC_WFI_WFE);

    /* Now we should trap to EL3 with SCTLR.nTWI set */
    SVC_SET_REG(SCTLR, 1, sctlr | SCTLR_nTWI);
    TEST_MSG("WFI (SCTLR.nTWI set, SCR.WFI set)");
    TEST_EL3_EXCEPTION(asm volatile("wfi\n"), EC_WFI_WFE);

    /* Restore SCTLR */
    SVC_SET_REG(SCTLR, 1, sctlr);

    /* Restore SCR */
    SVC_SET_REG(SCR, 3, scr);

    return 0;
}
#endif
