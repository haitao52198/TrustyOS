//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __ELASTOS_SPINLOCK_H__
#define __ELASTOS_SPINLOCK_H__

#include <elatypes.h>
#include <elaatomics.h>
#include <unistd.h>

_ELASTOS_NAMESPACE_BEGIN

class SpinLock
{
public:
    SpinLock() : mLocked(FALSE) {}

    void Lock();

    void Unlock();

    Boolean TryLock();

private:
    Int32 mLocked;
};

CAR_INLINE void SpinLock::Lock()
{
    while (atomic_swap(TRUE, &mLocked)) {
        while (mLocked) usleep(1);
    }
}

CAR_INLINE void SpinLock::Unlock()
{
    atomic_swap(FALSE, &mLocked);
}

CAR_INLINE _ELASTOS Boolean SpinLock::TryLock()
{
    return !atomic_swap(TRUE, &mLocked);
}

_ELASTOS_NAMESPACE_END

#endif //__ELASTOS_SPINLOCK_H__
