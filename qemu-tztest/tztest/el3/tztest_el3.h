#ifndef _TZTEST_EL3_H
#define _TZTEST_EL3_H

extern uintptr_t el3_check_state(uint32_t);
extern uintptr_t el3_check_exceptions(uint32_t);
extern uintptr_t el3_check_banked_regs(uint32_t);

#endif
