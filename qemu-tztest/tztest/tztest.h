#ifndef _TZTEST_H
#define _TZTEST_H

#include <stdint.h>

typedef uint32_t (*tztest_t)(uint32_t);
typedef struct {
    uint32_t fid;
    uint32_t el;
    uint32_t state;
    uint32_t arg;
} tztest_case_t;
extern tztest_case_t tztest[];
extern tztest_t test_func[];
extern void tztest_start();
extern void run_test(uint32_t, uint32_t, uint32_t, uint32_t);

#endif
