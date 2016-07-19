#ifndef _SVC_H
#define _SVC_H

#define SVC_OP_YIELD        1
#define SVC_OP_EXIT         2
#define SVC_OP_MAP          3
#define SVC_OP_GET_REG      4
#define SVC_OP_SET_REG      5
#define SVC_OP_TEST         6
#define SVC_OP_DISPATCH     7
#define SVC_OP_GET_SYSCNTL  8
#define SVC_OP_ALLOC        9

#ifndef __ASSEMBLY__
#include "interop.h"

extern const char *svc_op_name[];

typedef union {
    op_alloc_mem_t alloc;
    op_map_mem_t map;
    op_data_t get;
    op_data_t set;
    op_test_t test;
    op_dispatch_t disp;
} svc_op_desc_t;

extern uint32_t __svc(uint32_t, const svc_op_desc_t *);

#define SVC_GET_REG(__reg, __el, __val)     \
    do {                                \
        svc_op_desc_t desc;             \
        desc.get.key = (__reg);         \
        desc.get.el = (__el);           \
        __svc(SVC_OP_GET_REG, &desc);   \
        (__val) = desc.get.data;        \
    } while (0)

#define SVC_SET_REG(__reg, __el, __val)     \
    do {                                \
        svc_op_desc_t desc;             \
        desc.get.key = (__reg);         \
        desc.get.el = (__el);           \
        desc.get.data = (__val);        \
        __svc(SVC_OP_SET_REG, &desc);   \
    } while (0)

#endif

#endif
