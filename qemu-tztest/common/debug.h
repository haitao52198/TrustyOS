#ifndef _DEBUG_H
#define _DEBUG_H

#include "state.h"

#ifdef DEBUG
#define DEBUG_MSG(_str, ...) \
    printf("\n[DEBUG] %s (%s): "\
           _str, __FUNCTION__, sec_state_str[secure_state], ##__VA_ARGS__)
#define DEBUG_ARG
#else
#define DEBUG_MSG(_str, ...)
#define DEBUG_ARG __attribute__ ((unused))
#endif

#endif
