#include "libcflat.h"
#include "svc.h"
#include "smc.h"
#include "syscntl.h"
#include "builtins.h"
#include "exception.h"
#include "state.h"
#include "cpu.h"
#include "debug.h"
#include "tztest_internal.h"

uint32_t el1_check_smc(uint32_t __attribute__((unused))arg)
{
    TEST_HEAD("smc behavior");

#ifdef AARCH64
    /* On AArch64, SMC calls below EL3 should result in an undefined exception
     * if the SCR.SMD bit is set.  This is regardless of whether EL2 is present
     * or not.
     * However if we try to test this by setting the SMD bit then we get into
     * a state we can't get out of, because SMC_SET_REG() relies on
     * SMC working to work at all... So don't try it.
     */
#else
    uintptr_t scr;

    SMC_GET_REG(SCR, 3, scr);
    SMC_SET_REG(SCR, 3, scr | SCR_SMD);
    /* On AArch32, SMC calls are undefined when SCR.SCD is set only when the
     * virtualization extensions are present.
     */
    //TEST_EL1_EXCEPTION(__smc(SMC_OP_NOOP, NULL), EC_UNKNOWN);

    /* When the virtualization extensions are not present, the SCR.SCD bit
     * setting should have no impact on SMC.
     */
    TEST_MSG("SMC call without virt (SCR.SCD = 1)");
    TEST_NO_EXCEPTION(__smc(SMC_OP_NOOP, NULL));

    /* Restore SCR */
    SMC_SET_REG(SCR, 3, scr);
#endif

    return 0;
}

#ifdef AARCH64
uint32_t el1_check_cpacr_trap(uint32_t __attribute__((unused))arg)
{
    uint64_t cptr_el3, cpacr;

    TEST_HEAD("CPACR trapping");

    /* Get the current CPTR so we can restore it later */
    SMC_GET_REG(CPTR_EL3, 3, cptr_el3);

    /* Disable CPACR access */
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TCPAC);

    /* Try to read CPACR */
    TEST_MSG("Read of disabled CPACR");
    TEST_EL3_EXCEPTION(cpacr = READ_CPACR(), EC_SYSINSN);

    /* Try to write CPACR */
    TEST_MSG("Write of disabled CPACR");
    TEST_EL3_EXCEPTION(WRITE_CPACR(cpacr), EC_SYSINSN);

#ifdef FP_TEST
    /* Disable FP access */
    TEST_MSG("Read of disabled FP reg");
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TFP);
    TEST_EL3_EXCEPTION(asm volatile("fcmp s0, #0.0\n"), EC_SIMD);
#endif

    /* Restore the original CPTR */
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3);

    return 0;
}

uint32_t el1_check_wfx_trap(uint32_t __attribute__((unused))arg)
{
    uint64_t sctlr, scr;

    TEST_HEAD("WFx traps");

    /* Get the current SCR so we can restore it later */
    SMC_GET_REG(SCR, 3, scr);

    /* Get the current SCTLR so we can restore it later */
    sctlr = READ_SCTLR();

    /* Clearing SCTLR.nTWE normally traps WFE to EL1 but we are already there */
    WRITE_SCTLR(sctlr & ~SCTLR_nTWE);
    SMC_SET_REG(SCR, 3, scr & ~SCR_TWE);
    TEST_MSG("WFE (SCTLR.nTWE clear, SCR.WFE clear)");
    TEST_NO_EXCEPTION(asm volatile("wfe\n"));

#if 0
    /* Architecturally there is no guarantee that executing a WFE
     * will cause it to trap -- the trap only happens if the WFE would
     * put the CPU into a "power saving state", so a NOP implementation
     * can validly not trap at all. Since QEMU's implementation of WFE
     * does exactly this (though our WFI does not), disable the WFE tests.
     */

    /* Trap WFE instructions to EL3.  This should work regardless of the
     * SCTLR.nTWE setting.
     */
    SMC_SET_REG(SCR, 3, scr | SCR_TWE);
    TEST_MSG("WFE (SCTLR.nTWE clear, SCR.WFE set)");
    TEST_EL3_EXCEPTION(asm volatile("wfe\n"), EC_WFI_WFE);

    /* This should trap to EL3 with SCTLR.nTWE set */
    WRITE_SCTLR(sctlr | SCTLR_nTWE);
    TEST_MSG("WFE (SCTLR.nTWE set, SCR.WFE set)");
    TEST_EL3_EXCEPTION(asm volatile("wfe\n"), EC_WFI_WFE);
#endif

    /* We cannot test the effect of WFI in EL1 mode like we did with WFE as it
     * causes a hang.  It is assumed that since the exception is not trapped we
     * actually execute the instruction.
     * For this reason we don't bother to test the SCTLR bit effect or
     * precedence.
     */

    /* Trap WFI instructions to EL3.  This should work regardless of the
     * SCTLR.nTWE setting.
     */
    SMC_SET_REG(SCR, 3, scr | SCR_TWI);
    TEST_MSG("WFI (SCTLR.nTWI clear, SCR.WFI set)");
    TEST_EL3_EXCEPTION(asm volatile("wfi\n"), EC_WFI_WFE);

    /* This should trap to EL3 with SCTLR.nTWE set */
    WRITE_SCTLR(sctlr | SCTLR_nTWE);
    TEST_MSG("WFI (SCTLR.nTWE set, SCR.WFI set)");
    TEST_EL3_EXCEPTION(asm volatile("wfi\n"), EC_WFI_WFE);

    /* Restore SCTLR */
    WRITE_SCTLR(sctlr);

    /* Restore SCR */
    SMC_SET_REG(SCR, 3, scr);

    return 0;
}

uint32_t el1_check_fp_trap(uint32_t __attribute__((unused))arg)
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
    SMC_GET_REG(CPTR_EL3, 3, cptr_el3);
    cpacr = READ_CPACR();

    /* First check that enabled FP does not give us an exception */
    TEST_MSG("FP enabled (CPACR.FPEN = 3, CPTR_EL3 = 0)");
    WRITE_CPACR(cpacr | CPACR_FPEN(3));
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3 & ~CPTR_TFP);
    TEST_NO_EXCEPTION(asm volatile("fcmp s0, #0.0\n"));

    /* Disable CPACR FP access */
    TEST_MSG("FP disabled (CPACR.FPEN = 0, CPTR_EL3 = 0)");
    WRITE_CPACR(cpacr & ~(CPACR_FPEN(3)));
    TEST_ENABLE_EXCP_LOG();
    asm volatile("fcmp s0, #0.0\n");
    WRITE_CPACR(cpacr);           /* Reenable printing */
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
    WRITE_CPACR(cpacr & ~(CPACR_FPEN(2)));
    /* No exception in EL1 if FPEN = 1 */
    TEST_NO_EXCEPTION(asm volatile("fcmp s0, #0.0\n"));

    TEST_MSG("FP disabled (CPACR.FPEN = 2, CPTR_EL3 = 0)");
    WRITE_CPACR(cpacr & ~(CPACR_FPEN(1)));
    TEST_ENABLE_EXCP_LOG();
    asm volatile("fcmp s0, #0.0\n");
    WRITE_CPACR(cpacr);           /* Reenable printing */
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
    WRITE_CPACR(cpacr & ~(CPACR_FPEN(3)));
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TFP);
    TEST_ENABLE_EXCP_LOG();
    asm volatile("fcmp s0, #0.0\n");
    WRITE_CPACR(cpacr);           /* Reenable printing */
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3);     /* Reenable printing */
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
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3 | CPTR_TFP);
    TEST_ENABLE_EXCP_LOG();
    asm volatile("fcmp s0, #0.0\n");
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3);     /* Reenable printing */
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
    WRITE_CPACR(cpacr);

    /* Restore the original CPTR */
    SMC_SET_REG(CPTR_EL3, 3, cptr_el3);
#endif

    return 0;
}
#endif

