#include "el1_common.h"
#include "cpu.h"

uintptr_t EL1_S_INIT_BASE = (uintptr_t)&_EL1_S_INIT_BASE;
uintptr_t EL1_S_INIT_SIZE = (uintptr_t)&_EL1_S_INIT_SIZE;
uintptr_t EL1_S_FLASH_TEXT = (uintptr_t)&_EL1_S_FLASH_TEXT;
uintptr_t EL1_S_TEXT_BASE = (uintptr_t)&_EL1_S_TEXT_BASE;
uintptr_t EL1_S_DATA_BASE = (uintptr_t)&_EL1_S_DATA_BASE;
uintptr_t EL1_S_TEXT_SIZE = (uintptr_t)&_EL1_S_TEXT_SIZE;
uintptr_t EL1_S_DATA_SIZE = (uintptr_t)&_EL1_S_DATA_SIZE;

char *sec_state_str[] = {"secure", "nonsecure"};
uint32_t secure_state = SECURE;
const uint32_t exception_level = EL1;

#if REMOVE_OR_INTEGRATE
void el1_sec_check_init()
{
    printf("\nValidating startup state:\n");

    printf("\tChecking for security extension ...");
    int idpfr1 = 0;
    /* Read the ID_PFR1 CP register and check that it is marked for support of
     * the security extension.
     */
    __mrc(15, 0, idpfr1, 0, 1, 1);
    if (0x10 != (idpfr1 & 0xf0)) {
        printf("FAILED\n");
        DEBUG_MSG("current IDPFR1 (%d) != expected IDPFR1 (%d)\n",
                  (idpfr1 & 0xf0), 0x10);
        exit(1);
    } else {
        printf("PASSED\n");
    }

    printf("\tChecking initial processor mode... ");
    if (CPSR_M_SVC != (_read_cpsr() & 0x1f)) {
        printf("FAILED\n");
        DEBUG_MSG("current CPSR (%d) != expected CPSR (%d)\n",
                  (_read_cpsr() & 0x1f), CPSR_M_SVC);
        assert(CPSR_M_SVC == (_read_cpsr() & 0x1f));
    } else {
        printf("PASSED\n");
    }

    // Test: Check that on reset if sec et present, starts in sec state
    //      pg. B1-1204
    printf("\tChecking initial security state... ");
    if (0 != (_read_scr() & SCR_NS)) {
        printf("Failed\n");
        DEBUG_MSG("current SCR.NS (%d) != expected SCR.NS (%d)\n",
                  (_read_cpsr() & SCR_NS), 0);
        assert(0 == (_read_scr() & SCR_NS));
    } else {
        printf("PASSED\n");
    }
}
#endif

void el1_init_el0()
{
    uintptr_t main;

    el1_load_el0(EL0_S_FLASH_BASE, &main);

        __exception_return(main, SPSR_EL0);
}
