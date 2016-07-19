#include "libcflat.h"
#include "svc.h"
#include "smc.h"
#include "syscntl.h"
#include "builtins.h"
#include "exception.h"
#include "state.h"
#include "debug.h"
#include "tztest_internal.h"
#include "tztest_el1.h"
#include "cpu.h"

uint32_t el1_check_register_access(uint32_t __attribute__((unused))arg)
{
    uintptr_t val;

    /* Set things to non-secure P1 and attempt accesses */
    TEST_HEAD("restricted register access");

    /* We capture the read and use it in the write of the below to insure we do
     * not destroy the register setting.  This can only go wrong if read fails
     * but write succeeds which is unlikely.
     * Note: If the above scenario occurs, we will likely hang on the way out
     * of the test.
     */
    TEST_MSG("SCR read");
    TEST_EL1_EXCEPTION(val = READ_SCR(), EC_UNKNOWN);

    TEST_MSG("SCR write");
    TEST_EL1_EXCEPTION(WRITE_SCR(val), EC_UNKNOWN);

    TEST_MSG("SDER read");
    TEST_EL1_EXCEPTION(val = READ_SDER(), EC_UNKNOWN);

    TEST_MSG("SDER write");
    TEST_EL1_EXCEPTION(WRITE_SDER(val), EC_UNKNOWN);

#ifdef AARCH32
    TEST_MSG("MVBAR read");
    TEST_EL1_EXCEPTION(val = READ_MVBAR(), EC_UNKNOWN);

    TEST_MSG("MVBAR write");
    TEST_EL1_EXCEPTION(WRITE_MVBAR(val), EC_UNKNOWN);

    /* It should be safe to write a zero to NSACR as we have not disabled any
     * non-secure access at this point.
     */
    TEST_MSG("NSACR write");
    TEST_EL1_EXCEPTION(WRITE_NSACR(0), EC_UNKNOWN);
#endif

#ifdef AARCH64
    TEST_MSG("CPTR_EL3 read");
    TEST_EL1_EXCEPTION(val = READ_CPTR_EL3(), EC_UNKNOWN);

    TEST_MSG("CPTR_EL3 write");
    TEST_EL1_EXCEPTION(WRITE_CPTR_EL3(val), EC_UNKNOWN);
#endif

    return 0;
}

#ifdef AARCH32
uint32_t check_mask_bits()
{
    uint32_t ret = 0;

    uintptr_t cpsr = READ_CPSR();
    uintptr_t val;
    uintptr_t scr;

    SMC_GET_REG(SCR, 3, scr);

    /* Test: SCR.FW/AW protects access to CPSR.F/A when nonsecure
     *       pg. B1-1151  table B1-2
     */
    TEST_HEAD("CPSR accessibility");

    /* It is safe to assume that we are in nonsecure state as we validated
     * this on entry.  Set the SCR FW and AW bits.
     * This SCR setting should allow the CPSR to be written from nonsecure
     * state.
     */
    SMC_SET_REG(SCR, 3, scr | (SCR_AW | SCR_FW));

    TEST_MSG("Writing CPSR.F with SCR.FW enabled");
    val = cpsr | CPSR_F;
    TEST_FUNCTION(WRITE_CPSR(val), READ_CPSR() == val);

    /* Restore CPSR to its original value before next test. */
    WRITE_CPSR(cpsr);

    TEST_MSG("Writing CPSR.A with SCR.AW enabled");
    val = cpsr | CPSR_A;
    TEST_FUNCTION(WRITE_CPSR(val), READ_CPSR() == val);

    /* Restore CPSR to its original value before next test. */
    WRITE_CPSR(cpsr);

    /* Switch SCR back to what we likely started with and retry the CPSR
     * writes.  At this point the setting of these bits should be blocked due
     * to the AW and FW bits being clear.
     */
    SMC_SET_REG(SCR, 3, scr);

    TEST_MSG("Writing CPSR.F with SCR.FW disabled");
    val = cpsr | CPSR_F;
    TEST_FUNCTION(WRITE_CPSR(val), READ_CPSR() != val);

    /* Restore CPSR to its original value before next test. */
    WRITE_CPSR(cpsr);

    TEST_MSG("Writing CPSR.A with SCR.AW disabled");
    val = cpsr | CPSR_A;
    TEST_FUNCTION(WRITE_CPSR(val), READ_CPSR() != val);

    /* Restore CPSR to its original value */
    WRITE_CPSR(cpsr);

    return ret;
}
#endif

tztest_t test_func[] = {
    [TZTEST_SMC] = el1_check_smc,
    [TZTEST_REG_ACCESS] = el1_check_register_access,
#ifdef AARCH64
    [TZTEST_CPACR_TRAP] = el1_check_cpacr_trap,
    [TZTEST_WFX_TRAP] = el1_check_wfx_trap,
    [TZTEST_FP_TRAP] = el1_check_fp_trap,
#endif
#ifdef AARCH32
    [TZTEST_MASK_BITS] = check_mask_bits,
#endif
};

