#ifndef _SMC_H
#define _SMC_H

#define SMC_OP_NOOP         0   /* Issue SMC with no switch or effect */
#define SMC_OP_YIELD        1   /* Switch execution state, return from remote */
#define SMC_OP_EXIT         2   /* Shutdown machine */
#define SMC_OP_MAP          3   /* Perform memory map operation */
#define SMC_OP_GET_REG      4   /* Fetch specified register */
#define SMC_OP_SET_REG      5   /* Set specified register */
#define SMC_OP_TEST         6   /* Perform interop test */
#define SMC_OP_DISPATCH     7   /* Execute remote test function */

#ifndef __ASSEMBLY__
#include "interop.h"

extern uint32_t __smc(uint32_t, void *);

typedef union {
    op_dispatch_t disp;
    op_map_mem_t map;
    op_test_t test;
    op_data_t get;
    op_data_t set;
} smc_op_desc_t;

extern smc_op_desc_t *smc_interop_buf;

#define SMC_EXIT()  __smc(SMC_OP_EXIT, NULL)
#define SMC_YIELD()  __smc(SMC_OP_YIELD, smc_interop_buf);

#define SMC_GET_REG(__reg, __el, __val)     \
    do {                                \
        smc_op_desc_t *desc = smc_interop_buf;             \
        desc->get.key = (__reg);         \
        desc->get.el = (__el);           \
        __smc(SMC_OP_GET_REG, desc);   \
        (__val) = desc->get.data;        \
    } while (0)

#define SMC_SET_REG(__reg, __el, __val)     \
    do {                                \
        smc_op_desc_t *desc = smc_interop_buf;             \
        desc->get.key = (__reg);         \
        desc->get.el = (__el);           \
        desc->get.data = (__val);        \
        __smc(SMC_OP_SET_REG, desc);   \
    } while (0)

#endif

#endif
