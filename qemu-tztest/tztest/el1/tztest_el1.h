#ifndef _TZTEST_EL1_H
#define _TZTEST_EL1_H

uint32_t el1_check_smc(uint32_t);
uint32_t el1_check_cpacr_trap(uint32_t);
uint32_t el1_check_wfx_trap(uint32_t);
uint32_t el1_check_fp_trap(uint32_t);
uint32_t el1_check_register_access(uint32_t);

#endif
