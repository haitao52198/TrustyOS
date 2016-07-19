#include "libcflat.h"
#include "syscntl.h"
#include "smc.h"
#include "builtins.h"
#include "exception.h"
#include "state.h"
#include "cpu.h"
#include "debug.h"
#include "tztest_internal.h"

#ifdef AARCH32
uint32_t el3_check_smc()
{
    TEST_HEAD("monitor mode");

    uint32_t cpsr = READ_CPSR();

    TEST_MSG("Checking monitor mode");
    TEST_CONDITION(CPSR_M_MON == (cpsr & CPSR_M_MON));

    // Test: Check that CPSR.A/I.F are set to 1 on exception to mon mode
    //      pg. B1-1182
    TEST_MSG("Checking monitor mode CPSR.F value");
    TEST_CONDITION(CPSR_F == (CPSR_F & cpsr));

    TEST_MSG("Checking monitor mode CPSR.I value");
    TEST_CONDITION(CPSR_I == (CPSR_I & cpsr));

    TEST_MSG("Checking monitor mode CPSR.A value");
    TEST_CONDITION(CPSR_A == (CPSR_A & cpsr));

    return 0;
}

#if 0
uint32_t el3_check_exceptions()
{
    TEST_HEAD("monitor mode exception state");

    /* B1-1211: SMC exceptions from monitor mode cause transition to secure
     *          state.
     * Test: Check that an exception from mon mode, NS cleared to 0
     *       pg. B1-1170
     */
    TEST_MSG("Checking monitor exception state");
    TEST_FUNCTION(__smc(SMC_OP_NOOP, NULL), (READ_SCR() & SCR_NS) == !SCR_NS);

    return 0;
}

uint32_t el3_check_banked_regs()
{
    uint32_t scr = _read_scr();
    uint32_t val = 0;

    printf("\nValidating monitor banked register access:\n");

    VERIFY_REGISTER_CUSTOM(csselr, 0xF, TZTEST_SVAL, TZTEST_NSVAL);
    /* Modifying SCTLR is highly disruptive, so the test is heavily restricted
     * to avoid complications.  We only flip the V-bit for comparison which is
     * safe unless we take an exception which should be low risk.
     */
    val = _read_sctlr();
    VERIFY_REGISTER_CUSTOM(sctlr, (1 << 13), val, (val | (1 << 13)));

    /* ACTLR is banked but not supported on Vexpress */

    /* For testing purposes we switch the secure world to the nonsecure page
     * table, so any translations can still be made.  Since we are only working
     * out of the non-secure mappings we should be safe.  This assumption could
     * be wrong.
     */
    VERIFY_REGISTER_CUSTOM(ttbr0, 0xFFF00000,
                           (uint32_t)nsec_l1_page_table, TZTEST_NSVAL);
    VERIFY_REGISTER(ttbr1);

    /* Modifying TTBCR is highly disruptive, so the test is heavily restricted
     * to avoid complications.  We only use the PD1-bit for comparison as we
     * are not using ttbr1 at this time.
     */
    val = _read_ttbcr();

    VERIFY_REGISTER_CUSTOM(ttbcr, (1 << 5), val, (val | (1 << 5)));
    /* Leave the bottom 4 bits alone as they will disrupt address translation
     * otherwise.
     */
    VERIFY_REGISTER_CUSTOM(dacr, 0xFFFFFFF0, 0x55555555, 0xAAAAAAA5);

    VERIFY_REGISTER(dfsr);
    VERIFY_REGISTER(ifsr);
    VERIFY_REGISTER(dfar);
    VERIFY_REGISTER(ifar);
    VERIFY_REGISTER_CUSTOM(par, 0xFFFFF000, TZTEST_SVAL, TZTEST_NSVAL);
    VERIFY_REGISTER(prrr);
    VERIFY_REGISTER(nmrr);

    /* The bottome 5 bits are SBZ */
    VERIFY_REGISTER_CUSTOM(vbar, 0xFFFFFFE0, TZTEST_SVAL, TZTEST_NSVAL);

    VERIFY_REGISTER(fcseidr);
    VERIFY_REGISTER(contextidr);
    VERIFY_REGISTER(tpidrurw);
    VERIFY_REGISTER(tpidruro);
    VERIFY_REGISTER(tpidrprw);

    /* Restore the SCR to it's original value */
    _write_scr(scr);

    return 0;
}
#endif
#endif

tztest_t test_func[] = {
#ifdef AARCH32
    [TZTEST_SMC] = el3_check_smc,
#endif
};
