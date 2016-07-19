#ifndef _BUILTINS_H
#define _BUILTINS_H

#include "stdint.h"

extern uint64_t read_currentel();
extern void write_currentel(uint64_t);
extern uint64_t read_scr_el3();
extern void write_scr_el3(uint64_t);
extern uint64_t read_sder32_el3();
extern void write_sder32_el3(uint64_t);
extern uint64_t read_cptr_el3();
extern void write_cptr_el3(uint64_t);
extern uint64_t read_cpacr_el1();
extern void write_cpacr_el1(uint64_t);
extern uint64_t read_cpacr_el1();
extern void write_cpacr_el1(uint64_t);
extern uint64_t read_sctlr_el1();
extern void write_sctlr_el1(uint64_t);
extern uint64_t read_sctlr_el3();
extern void write_sctlr_el3(uint64_t);
extern void __set_exception_return(uint64_t);
extern void __exception_return(uintptr_t, uint32_t);

#define READ_SCR() read_scr_el3()
#define WRITE_SCR(_val) write_scr_el3(_val)
#define READ_SDER() read_sder32_el3()
#define WRITE_SDER(_val) write_sder32_el3(_val)
#define READ_CPTR_EL3() read_cptr_el3()
#define WRITE_CPTR_EL3(_val) write_cptr_el3(_val)
#define READ_CPACR() read_cpacr_el1()
#define WRITE_CPACR(_val) write_cpacr_el1(_val)
#define READ_CURRENTEL() read_currentel()
#define WRITE_CURRENTEL(_val) read_currentel(_val)
#define READ_SCTLR() read_sctlr_el1()
#define WRITE_SCTLR(_val) write_sctlr_el1(_val)
#define READ_SCTLR_EL3() read_sctlr_el3()
#define WRITE_SCTLR_EL3(_val) write_sctlr_el3(_val)

#endif
