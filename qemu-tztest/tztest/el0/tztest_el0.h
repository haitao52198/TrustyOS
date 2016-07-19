#ifndef _TZTEST_EL0_H
#define _TZTEST_EL0_H

extern uint32_t el0_check_smc(uint32_t);
extern uint32_t el0_check_register_access(uint32_t);
extern uint32_t el0_check_cpacr_trap(uint32_t);
extern uint32_t el0_check_fp_trap(uint32_t);
extern uint32_t el0_check_wfx_trap(uint32_t);

#endif

