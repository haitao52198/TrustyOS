#ifndef _EL3_MONITOR_H
#define _EL3_MONITOR_H

typedef struct state_buf {
    uintptr_t sp_el0;
    uintptr_t elr_el0;
    uintptr_t spsr_irq;
    uintptr_t sp_irq;
    uintptr_t elr_irq;
    uintptr_t spsr_el1;
    uintptr_t sp_el1;
    uintptr_t elr_el1;
    uintptr_t spsr_abt;
    uintptr_t sp_abt;
    uintptr_t elr_abt;
    uintptr_t spsr_und;
    uintptr_t sp_und;
    uintptr_t elr_und;
    uintptr_t spsr_el3;
    uintptr_t elr_el3;
    uintptr_t reg[9];    /* r4-r12 */
} state_buf;

extern void monitor_switch(int op, smc_op_desc_t *desc);
extern void monitor_restore_state(state_buf *);
extern void monitor_save_state(state_buf *);

#endif
