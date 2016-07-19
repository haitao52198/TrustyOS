#include "el1_common.h"
#include "cpu.h"

uintptr_t EL1_NS_INIT_BASE = (uintptr_t)&_EL1_NS_INIT_BASE;
uintptr_t EL1_NS_INIT_SIZE = (uintptr_t)&_EL1_NS_INIT_SIZE;
uintptr_t EL1_NS_FLASH_TEXT = (uintptr_t)&_EL1_NS_FLASH_TEXT;
uintptr_t EL1_NS_TEXT_BASE = (uintptr_t)&_EL1_NS_TEXT_BASE;
uintptr_t EL1_NS_DATA_BASE = (uintptr_t)&_EL1_NS_DATA_BASE;
uintptr_t EL1_NS_TEXT_SIZE = (uintptr_t)&_EL1_NS_TEXT_SIZE;
uintptr_t EL1_NS_DATA_SIZE = (uintptr_t)&_EL1_NS_DATA_SIZE;

char *sec_state_str[] = {"secure", "nonsecure"};
uint32_t secure_state = NONSECURE;
const uint32_t exception_level = EL1;

void el1_init_el0()
{
    uintptr_t main;

    el1_load_el0(EL0_NS_FLASH_BASE, &main);

    __exception_return(main, SPSR_EL0);

}
