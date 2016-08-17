
#ifndef __ELREFBASE_H__
#define __ELREFBASE_H__

#include <car.h>
#include <stdio.h>

_ELASTOS_NAMESPACE_BEGIN

class ElRefBase
{
public:
    class WeakRefType
    {
    public:
        CARAPI_(ElRefBase*) GetRefBase() const;

        CARAPI_(void) IncWeak(const void* id);
        CARAPI_(void) DecWeak(const void* id);

        CARAPI_(Boolean) AttemptIncStrong(const void* id);

        //! This is only safe if you have set OBJECT_LIFETIME_FOREVER.
        CARAPI_(Boolean) AttemptIncWeak(const void* id);

        //! DEBUGGING ONLY: Get current weak ref count.
        CARAPI_(Int32) GetWeakCount() const;

        //! DEBUGGING ONLY: Print references held on object.
        CARAPI_(void) PrintRefs() const;

        //! DEBUGGING ONLY: Enable tracking for this object.
        // enable -- enable/disable tracking
        // retain -- when tracking is enable, if true, then we save a stack trace
        //           for each reference and dereference; when retain == false, we
        //           match up references and dereferences and keep only the
        //           outstanding ones.
        CARAPI_(void) TrackMe(Boolean enable, Boolean retain);
    };

public:
    CARAPI_(void) IncStrong(const void* id) const;
    CARAPI_(void) DecStrong(const void* id) const;

    CARAPI_(void) ForceIncStrong(const void* id) const;

    //! DEBUGGING ONLY: Get current strong ref count.
    CARAPI_(Int32) GetStrongCount() const;

    CARAPI_(WeakRefType*) CreateWeak(const void* id) const;

    CARAPI_(WeakRefType*) GetWeakRefs() const;

    //! DEBUGGING ONLY: Print references held on object.
    inline CARAPI_(void) PrintRefs() const { GetWeakRefs()->PrintRefs(); }

    //! DEBUGGING ONLY: Enable tracking of object.
    inline CARAPI_(void) TrackMe(Boolean enable, Boolean retain)
    {
        GetWeakRefs()->TrackMe(enable, retain);
    }

    CARAPI_(UInt32) AddRef();
    CARAPI_(UInt32) Release();

protected:
    ElRefBase();
    virtual ~ElRefBase();

    //! Flags for extendObjectLifetime()
    enum {
        OBJECT_LIFETIME_STRONG  = 0x0000,
        OBJECT_LIFETIME_WEAK    = 0x0001,
        OBJECT_LIFETIME_MASK    = 0x0001
    };

    void ExtendObjectLifetime(Int32 mode);

    //! Flags for onIncStrongAttempted()
    enum {
        FIRST_INC_STRONG = 0x0001
    };

    virtual CARAPI_(void) OnFirstRef();
    virtual CARAPI_(void) OnLastStrongRef(const void* id);
    virtual CARAPI_(Boolean) OnIncStrongAttempted(UInt32 flags, const void* id);
    virtual CARAPI_(void) OnLastWeakRef(const void* id);

// private:
//     friend class ReferenceMover;
//     static void moveReferences(void* d, void const* s, size_t n,
//             const ReferenceConverterBase& caster);

private:
    friend class WeakRefType;
    class WeakRefImpl;

    ElRefBase(const ElRefBase& other);
    ElRefBase& operator = (const ElRefBase& other);

    WeakRefImpl* const mRefs;
};

class ElLightRefBase
{
public:
    ElLightRefBase() : mRef(0) {}

    virtual ~ElLightRefBase() {}

    CARAPI_(UInt32) AddRef()
    {
        Int32 ref = atomic_inc(&mRef);
        return ref;
    }

    CARAPI_(UInt32) Release()
    {
        Int32 ref = atomic_dec(&mRef);
        if (ref == 0) {
            delete this;
        }

        ELA_ASSERT_WITH_BLOCK(ref >= 0) {
            printf(" >> Error: Ref count of %p is %d, it should be >= 0.\n", this, ref);
        }

        return ref;
    }

    CARAPI_(UInt32) GetRefCount() const
    {
        return mRef;
    }

protected:
    Int32 mRef;
};

class WeakReferenceImpl
    : public ElLightRefBase
    , public IWeakReference
{
public:
    WeakReferenceImpl(
        /* [in] */ IInterface* object,
        /* [in] */ ElRefBase::WeakRefType* ref);

    ~WeakReferenceImpl();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [in] */ InterfaceID* iid);

    CARAPI Resolve(
        /* [in] */ const InterfaceID& riid,
        /* [out] */ IInterface** objectReference);

private:
    IInterface* mObject;
    ElRefBase::WeakRefType* mRef;
};

_ELASTOS_NAMESPACE_END

#ifndef REFCOUNT_ADD
#define REFCOUNT_ADD(i) if (i) { (i)->AddRef(); }
#endif

#ifndef REFCOUNT_RELEASE
#define REFCOUNT_RELEASE(i) if (i) { (i)->Release(); }
#endif

#endif //__ELREFBASE_H__
