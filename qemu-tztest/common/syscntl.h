#ifndef _SYSCNTL_H
#define _SYSCNTL_H

#include <stdint.h>
#include "libcflat.h"

typedef struct {
     void *buf_pa;
     void *buf_va;
} smc_interop_t;

typedef struct {
    uint32_t ec;
    uint32_t iss;
    uint32_t far;
    bool log;
    uint32_t action;
    bool taken;
    uint32_t el;
    uint32_t state;
} sys_exception_t;

#define EXCP_ACTION_SKIP 1

#define SEC     0
#define NSEC    1

typedef struct {
    volatile int fail_count;
    volatile int test_count;
} test_control_t;

typedef struct {
    smc_interop_t smc_interop;
    sys_exception_t excp;
    test_control_t *test_cntl;
} sys_control_t;

extern sys_control_t *syscntl;

#endif
