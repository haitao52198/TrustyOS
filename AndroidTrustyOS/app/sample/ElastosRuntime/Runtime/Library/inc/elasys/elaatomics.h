#ifndef __ELAATOMICS_H__
#define __ELAATOMICS_H__

#include <elatypes.h>

extern "C" {

ECO_PUBLIC int atomic_cmpxchg(int old, int _new, volatile int *ptr);
ECO_PUBLIC int atomic_swap(int _new, volatile int *ptr);

ECO_PUBLIC int atomic_inc(volatile int *ptr);
ECO_PUBLIC int atomic_dec(volatile int *ptr);
ECO_PUBLIC int atomic_add(int value, volatile int* ptr);
ECO_PUBLIC int atomic_and(int value, volatile int* ptr);
ECO_PUBLIC int atomic_or(int value, volatile int *ptr);

}

#endif /* __ELAATOMICS_H__ */
