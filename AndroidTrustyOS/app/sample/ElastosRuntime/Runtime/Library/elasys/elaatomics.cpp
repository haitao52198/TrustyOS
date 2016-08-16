#include <elaatomics.h>
#ifdef _android
#include <cutils/atomic.h>
#endif

int atomic_cmpxchg(int old, int _new, volatile int *ptr)
{
#ifdef _android
    return android_atomic_cmpxchg(old, _new, ptr);
#else
    return __sync_val_compare_and_swap(ptr, old, _new) != old;
#endif
}

int atomic_swap(int _new, volatile int *ptr)
{
#ifdef _android
    int old;
    do {
        old = *ptr;
    } while (android_atomic_cmpxchg(old, _new, ptr));
    return old;
#else
    int old;
    do {
        old = *ptr;
    } while (__sync_val_compare_and_swap(ptr, old, _new) != old);
    return old;
#endif
}

int atomic_inc(volatile int *ptr)
{
#ifdef _android
    // __atomic_inc of android bionic C library will return
    // the old value of *ptr before it is increased
    int prev = android_atomic_inc(ptr);
    // so we should increase nRef before return
    return ++prev;
#else
    return __sync_add_and_fetch(ptr, 1);
#endif
}

int atomic_dec(volatile int *ptr)
{
#ifdef _android
    // __atomic_dec of android bionic C library will return
    // the old value of *ptr before it is decreased
    int prev = android_atomic_dec(ptr);
    // so we should decrease nRef before return
    return --prev;
#else
    return __sync_sub_and_fetch(ptr, 1);
#endif
}

int atomic_add(int value, volatile int* ptr)
{
#ifdef _android
    return android_atomic_add(value, ptr);
#else
    return -1;
#endif
}

int atomic_and(int value, volatile int* ptr)
{
#ifdef _android
    return android_atomic_and(value, ptr);
#else
    return -1;
#endif
}

int atomic_or(int value, volatile int *ptr)
{
#ifdef _android
    return android_atomic_or(value, ptr);
#else
    return -1;
#endif
}
