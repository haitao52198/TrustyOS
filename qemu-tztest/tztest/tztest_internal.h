#ifndef _TZTEST_INTERNAL_H
#define _TZTEST_INTERNAL_H

#include "tztest.h"
#include "state.h"

typedef enum {
    TZTEST_SMC = 1,
    TZTEST_REG_ACCESS,
    TZTEST_CPACR_TRAP,
    TZTEST_WFX_TRAP,
    TZTEST_FP_TRAP,
    TZTEST_MASK_BITS,
    TZTEST_COUNT
} tztest_func_id_t;

#define TEST_HEAD(_str, ...) \
    printf("\nValidating %s EL%d " _str ":\n", \
           sec_state_str[secure_state], exception_level, ##__VA_ARGS__)

#define TEST_MSG(_str, ...) \
    printf("\tEL%d (%s): " _str "... ", \
           exception_level, sec_state_str[secure_state], ##__VA_ARGS__)

#define TEST_MSG_SUCCESS() printf("PASSED\n")
#define TEST_MSG_FAILURE() printf("FAILED\n")

#define INC_TEST_COUNT()    (syscntl->test_cntl->test_count += 1)
#define INC_FAIL_COUNT()    (syscntl->test_cntl->fail_count += 1)

#define TEST_CONDITION(_cond)                           \
    do {                                                \
        if (!(_cond)) {                                 \
            TEST_MSG_FAILURE();                         \
            INC_FAIL_COUNT();                           \
        } else {                                        \
            TEST_MSG_SUCCESS();                         \
        }                                               \
        INC_TEST_COUNT();                               \
    } while(0)

#define TEST_FUNCTION(_fn, _cond)                       \
    do {                                                \
        _fn;                                            \
        TEST_CONDITION(_cond);                          \
    } while(0)

#define TEST_ENABLE_EXCP_LOG()                          \
    do {                                                \
        syscntl->excp.action = EXCP_ACTION_SKIP;        \
        syscntl->excp.log = true;                       \
    } while(0)

#define TEST_EXCP_RESET()                               \
    do {                                                \
        syscntl->excp.taken = 0;                        \
        syscntl->excp.ec = 0;                           \
        syscntl->excp.iss = 0;                          \
        syscntl->excp.far = 0;                          \
        syscntl->excp.el = 0;                           \
        syscntl->excp.state = 0;                        \
        syscntl->excp.action = 0;                       \
        syscntl->excp.log = false;                      \
    } while(0)

#define TEST_EXCEPTION(_fn, _excp, _el)                 \
    do {                                                \
        TEST_ENABLE_EXCP_LOG();                         \
        _fn;                                            \
        TEST_CONDITION(syscntl->excp.taken &&           \
                       syscntl->excp.el == (_el) &&     \
                       syscntl->excp.ec == (_excp));    \
        TEST_EXCP_RESET();                              \
    } while (0)

#define TEST_NO_EXCEPTION(_fn)                          \
    do {                                                \
        TEST_ENABLE_EXCP_LOG();                         \
        _fn;                                            \
        TEST_CONDITION(!syscntl->excp.taken);           \
        TEST_EXCP_RESET();                              \
    } while (0)

#define TEST_EL1_EXCEPTION(_fn, _excp) \
        TEST_EXCEPTION(_fn, _excp, EL1)
#define TEST_EL3_EXCEPTION(_fn, _excp) \
        TEST_EXCEPTION(_fn, _excp, EL3)

#endif

