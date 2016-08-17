#include <elrefbase.h>
#include <cutils/log.h>

_ELASTOS_NAMESPACE_BEGIN

// compile with refcounting debugging enabled
#define DEBUG_REFS                      0
#define DEBUG_REFS_FATAL_SANITY_CHECKS  0
#define DEBUG_REFS_ENABLED_BY_DEFAULT   1
#define DEBUG_REFS_CALLSTACK_ENABLED    1

// log all reference counting operations
#define PRINT_REFS                      0

// ---------------------------------------------------------------------------
#define INITIAL_STRONG_VALUE (1<<28)

// ---------------------------------------------------------------------------
class ElRefBase::WeakRefImpl : public ElRefBase::WeakRefType
{
public:
    volatile Int32    mStrong;
    volatile Int32    mWeak;
    ElRefBase* const   mBase;
    volatile Int32    mFlags;

#if !DEBUG_REFS

    WeakRefImpl(ElRefBase* base)
        : mStrong(INITIAL_STRONG_VALUE)
        , mWeak(0)
        , mBase(base)
        , mFlags(0)
    {}

    void AddStrongRef(const void* /*id*/) { }
    void RemoveStrongRef(const void* /*id*/) { }
    void RenameStrongRefId(const void* /*old_id*/, const void* /*new_id*/) { }
    void AddWeakRef(const void* /*id*/) { }
    void RemoveWeakRef(const void* /*id*/) { }
    void RenameWeakRefId(const void* /*old_id*/, const void* /*new_id*/) { }
    void PrintRefs() const { }
    void TrackMe(Boolean, Boolean) { }

#else

    WeakRefImpl(ElRefBase* base)
        : mStrong(INITIAL_STRONG_VALUE)
        , mWeak(0)
        , mBase(base)
        , mFlags(0)
        , mStrongRefs(NULL)
        , mWeakRefs(NULL)
        , mTrackEnabled(!!DEBUG_REFS_ENABLED_BY_DEFAULT)
        , mRetain(FALSE)
    {}

    ~WeakRefImpl()
    {
//        bool dumpStack = false;
//        if (!mRetain && mStrongRefs != NULL) {
//            dumpStack = true;
//#if DEBUG_REFS_FATAL_SANITY_CHECKS
//            LOG_ALWAYS_FATAL("Strong references remain!");
//#else
//            ALOGE("Strong references remain:");
//#endif
//            ref_entry* refs = mStrongRefs;
//            while (refs) {
//                char inc = refs->ref >= 0 ? '+' : '-';
//                ALOGD("\t%c ID %p (ref %d):", inc, refs->id, refs->ref);
//#if DEBUG_REFS_CALLSTACK_ENABLED
//                refs->stack.dump();
//#endif
//                refs = refs->next;
//            }
//        }
//
//        if (!mRetain && mWeakRefs != NULL) {
//            dumpStack = true;
//#if DEBUG_REFS_FATAL_SANITY_CHECKS
//            LOG_ALWAYS_FATAL("Weak references remain:");
//#else
//            ALOGE("Weak references remain!");
//#endif
//            ref_entry* refs = mWeakRefs;
//            while (refs) {
//                char inc = refs->ref >= 0 ? '+' : '-';
//                ALOGD("\t%c ID %p (ref %d):", inc, refs->id, refs->ref);
//#if DEBUG_REFS_CALLSTACK_ENABLED
//                refs->stack.dump();
//#endif
//                refs = refs->next;
//            }
//        }
//        if (dumpStack) {
//            ALOGE("above errors at:");
//            CallStack stack;
//            stack.update();
//            stack.dump();
//        }
    }

    void AddStrongRef(const void* id)
    {
        //ALOGD_IF(mTrackEnabled,
        //        "addStrongRef: RefBase=%p, id=%p", mBase, id);
        AddRef(&mStrongRefs, id, mStrong);
    }

    void RemoveStrongRef(const void* id)
    {
        //ALOGD_IF(mTrackEnabled,
        //        "removeStrongRef: RefBase=%p, id=%p", mBase, id);
        if (!mRetain) {
            RemoveRef(&mStrongRefs, id);
        }
        else {
            AddRef(&mStrongRefs, id, -mStrong);
        }
    }

    void RenameStrongRefId(const void* oldId, const void* newId)
    {
        //ALOGD_IF(mTrackEnabled,
        //        "renameStrongRefId: RefBase=%p, oid=%p, nid=%p",
        //        mBase, old_id, new_id);
        RenameRefsId(mStrongRefs, oldId, newId);
    }

    void AddWeakRef(const void* id)
    {
        AddRef(&mWeakRefs, id, mWeak);
    }

    void RemoveWeakRef(const void* id)
    {
        if (!mRetain) {
            RemoveRef(&mWeakRefs, id);
        }
        else {
            AddRef(&mWeakRefs, id, -mWeak);
        }
    }

    void RenameWeakRefId(const void* oldId, const void* newId)
    {
        RenameRefsId(mWeakRefs, oldId, newId);
    }

    void TrackMe(Boolean track, Boolean retain)
    {
        mTrackEnabled = track;
        mRetain = retain;
    }

    void PrintRefs() const
    {
        String text;

        {
            Mutex::Autolock _l(mMutex);

            char buf[128];
            sprintf(buf, "Strong references on ElRefBase %p (WeakRefType %p):\n", mBase, this);
            text.append(buf);
            PrintRefsLocked(&text, mStrongRefs);
            sprintf(buf, "Weak references on ElRefBase %p (WeakRefType %p):\n", mBase, this);
            text.append(buf);
            PrintRefsLocked(&text, mWeakRefs);
        }

        {
            char name[100];
            snprintf(name, 100, "/data/%p.stack", this);
            int rc = open(name, O_RDWR | O_CREAT | O_APPEND);
            if (rc >= 0) {
                write(rc, text.string(), text.length());
                close(rc);
                ALOGD("STACK TRACE for %p saved in %s", this, name);
            }
            else ALOGE("FAILED TO PRINT STACK TRACE for %p in %s: %s", this,
                      name, strerror(errno));
        }
    }

private:
    struct RefEntry
    {
        RefEntry* mNext;
        const void* mId;
#if DEBUG_REFS_CALLSTACK_ENABLED
        CallStack mStack;
#endif
        Int32 mRef;
    };

    void AddRef(RefEntry** refs, const void* id, Int32 ref)
    {
        if (mTrackEnabled) {
            Mutex::Autolock _l(mMutex);

            RefEntry* ref = new RefEntry;
            // Reference count at the time of the snapshot, but before the
            // update.  Positive value means we increment, negative--we
            // decrement the reference count.
            ref->mRef = ref;
            ref->mId = id;
#if DEBUG_REFS_CALLSTACK_ENABLED
            ref->mStack.update(2);
#endif
            ref->mNext = *refs;
            *refs = ref;
        }
    }

    void RemoveRef(RefEntry** refs, const void* id)
    {
        if (mTrackEnabled) {
            Mutex::Autolock _l(mMutex);

            RefEntry* const head = *refs;
            RefEntry* ref = head;
            while (ref != NULL) {
                if (ref->mId == id) {
                    *refs = ref->mNext;
                    delete ref;
                    return;
                }

                refs = &ref->mNext;
                ref = *refs;
            }

//#if DEBUG_REFS_FATAL_SANITY_CHECKS
//            LOG_ALWAYS_FATAL("RefBase: removing id %p on RefBase %p"
//                    "(weakref_type %p) that doesn't exist!",
//                    id, mBase, this);
//#endif
//
//            ALOGE("RefBase: removing id %p on RefBase %p"
//                    "(weakref_type %p) that doesn't exist!",
//                    id, mBase, this);
//
//            ref = head;
//            while (ref) {
//                char inc = ref->ref >= 0 ? '+' : '-';
//                ALOGD("\t%c ID %p (ref %d):", inc, ref->id, ref->ref);
//                ref = ref->next;
//            }
//
//            CallStack stack;
//            stack.update();
//            stack.dump();
        }
    }

    void RenameRefsId(RefEntry* r, const void* oldId, const void* newId)
    {
        if (mTrackEnabled) {
            Mutex::Autolock _l(mMutex);

            RefEntry* ref = r;
            while (ref != NULL) {
                if (ref->mId == oldId) {
                    ref->mId = newId;
                }
                ref = ref->mNext;
            }
        }
    }

    void PrintRefsLocked(String* out, const RefEntry* refs) const
    {
        char buf[128];
        while (refs) {
            char inc = refs->ref >= 0 ? '+' : '-';
            sprintf(buf, "\t%c ID %p (ref %d):\n",
                    inc, refs->id, refs->ref);
            out->append(buf);
#if DEBUG_REFS_CALLSTACK_ENABLED
            out->append(refs->stack.toString("\t\t"));
#else
            out->append("\t\t(call stacks disabled)");
#endif
            refs = refs->mNext;
        }
    }

    mutable Mutex mMutex;
    RefEntry* mStrongRefs;
    RefEntry* mWeakRefs;

    Boolean mTrackEnabled;
    // Collect stack traces on addref and removeref, instead of deleting the stack references
    // on removeref that match the address ones.
    Boolean mRetain;

#endif
};

// ---------------------------------------------------------------------------

void ElRefBase::IncStrong(const void* id) const
{
    WeakRefImpl* const refs = mRefs;
    refs->IncWeak(id);

    refs->AddStrongRef(id);
    const Int32 c = atomic_inc(&refs->mStrong) - 1;
    ELA_ASSERT_WITH_BLOCK(c > 0) {
        ALOGE(" > ElRefBase: incStrong() called on %p after last strong ref", refs);
    }
#if PRINT_REFS
    ALOGD("incStrong of %p from %p: cnt=%d\n", this, id, c);
#endif
    if (c != INITIAL_STRONG_VALUE)  {
        return;
    }

    atomic_add(-INITIAL_STRONG_VALUE, &refs->mStrong);
    refs->mBase->OnFirstRef();
}

void ElRefBase::DecStrong(const void* id) const
{
    WeakRefImpl* const refs = mRefs;
    refs->RemoveStrongRef(id);
    const Int32 c = atomic_dec(&refs->mStrong) + 1;
#if PRINT_REFS
    ALOGD("decStrong of %p from %p: cnt=%d\n", this, id, c);
#endif
    ELA_ASSERT_WITH_BLOCK(c >= 1) {
        ALOGE(" > ElRefBase: decStrong() called on %p too many times", refs);
    }
    if (c == 1) {
        refs->mBase->OnLastStrongRef(id);
        if ((refs->mFlags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
            delete this;
        }
    }
    refs->DecWeak(id);
}

void ElRefBase::ForceIncStrong(const void* id) const
{
    WeakRefImpl* const refs = mRefs;
    refs->IncWeak(id);

    refs->AddStrongRef(id);
    const Int32 c = atomic_inc(&refs->mStrong) - 1;
    ELA_ASSERT_WITH_BLOCK(c >= 0) {
        ALOGE(" > ElRefBase: forceIncStrong called on %p after ref count underflow", refs);
    }
#if PRINT_REFS
    ALOGD("forceIncStrong of %p from %p: cnt=%d\n", this, id, c);
#endif

    switch (c) {
    case INITIAL_STRONG_VALUE:
        atomic_add(-INITIAL_STRONG_VALUE, &refs->mStrong);
        // fall through...
    case 0:
        refs->mBase->OnFirstRef();
    }
}

Int32 ElRefBase::GetStrongCount() const
{
    return mRefs->mStrong;
}

ElRefBase* ElRefBase::WeakRefType::GetRefBase() const
{
    return static_cast<const WeakRefImpl*>(this)->mBase;
}

void ElRefBase::WeakRefType::IncWeak(const void* id)
{
    WeakRefImpl* const impl = static_cast<WeakRefImpl*>(this);
    impl->AddWeakRef(id);
    const Int32 c __attribute__((__unused__)) = atomic_inc(&impl->mWeak) - 1;
    ELA_ASSERT_WITH_BLOCK(c >= 0) {
        ALOGE(" > ElRefBase: incWeak called on %p after last weak ref", this);
    }
}

void ElRefBase::WeakRefType::DecWeak(const void* id)
{
    WeakRefImpl* const impl = static_cast<WeakRefImpl*>(this);
    impl->RemoveWeakRef(id);
    const Int32 c = atomic_dec(&impl->mWeak) + 1;
    ELA_ASSERT_WITH_BLOCK(c >= 1) {
        ALOGE(" > ElRefBase: decWeak called on %p too many times", this);
    }
    if (c != 1) return;

    if ((impl->mFlags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
        // This is the regular lifetime case. The object is destroyed
        // when the last strong reference goes away. Since weakref_impl
        // outlive the object, it is not destroyed in the dtor, and
        // we'll have to do it here.
        if (impl->mStrong == INITIAL_STRONG_VALUE) {
            // Special case: we never had a strong reference, so we need to
            // destroy the object now.
            delete impl->mBase;
        }
        else {
            // ALOGV("Freeing refs %p of old RefWrapper %p\n", this, impl->mBase);
            delete impl;
        }
    }
    else {
        // less common case: lifetime is OBJECT_LIFETIME_{WEAK|FOREVER}
        impl->mBase->OnLastWeakRef(id);
        if ((impl->mFlags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_WEAK) {
            // this is the OBJECT_LIFETIME_WEAK case. The last weak-reference
            // is gone, we can destroy the object.
            delete impl->mBase;
        }
    }
}

Boolean ElRefBase::WeakRefType::AttemptIncStrong(const void* id)
{
    IncWeak(id);

    WeakRefImpl* const impl = static_cast<WeakRefImpl*>(this);

    Int32 curCount = impl->mStrong;
    ELA_ASSERT_WITH_BLOCK(curCount >= 0) {
        ALOGE(" > ElRefBase: attemptIncStrong called on %p after underflow", this);
    }
    while (curCount > 0 && curCount != INITIAL_STRONG_VALUE) {
        if (atomic_cmpxchg(curCount, curCount + 1, &impl->mStrong) == 0) {
            break;
        }
        curCount = impl->mStrong;
    }

    if (curCount <= 0 || curCount == INITIAL_STRONG_VALUE) {
        Boolean allow;
        if (curCount == INITIAL_STRONG_VALUE) {
            // Attempting to acquire first strong reference...  this is allowed
            // if the object does NOT have a longer lifetime (meaning the
            // implementation doesn't need to see this), or if the implementation
            // allows it to happen.
            allow = (impl->mFlags & OBJECT_LIFETIME_WEAK) != OBJECT_LIFETIME_WEAK
                  || impl->mBase->OnIncStrongAttempted(FIRST_INC_STRONG, id);
        }
        else {
            // Attempting to revive the object...  this is allowed
            // if the object DOES have a longer lifetime (so we can safely
            // call the object with only a weak ref) and the implementation
            // allows it to happen.
            allow = (impl->mFlags & OBJECT_LIFETIME_WEAK) == OBJECT_LIFETIME_WEAK
                  && impl->mBase->OnIncStrongAttempted(FIRST_INC_STRONG, id);
        }
        if (!allow) {
            DecWeak(id);
            return FALSE;
        }
        curCount = atomic_inc(&impl->mStrong) - 1;

        // If the strong reference count has already been incremented by
        // someone else, the implementor of onIncStrongAttempted() is holding
        // an unneeded reference.  So call onLastStrongRef() here to remove it.
        // (No, this is not pretty.)  Note that we MUST NOT do this if we
        // are in fact acquiring the first reference.
        if (curCount > 0 && curCount < INITIAL_STRONG_VALUE) {
            impl->mBase->OnLastStrongRef(id);
        }
    }

    impl->AddStrongRef(id);

#if PRINT_REFS
    ALOGD("attemptIncStrong of %p from %p: cnt=%d\n", this, id, curCount);
#endif

    if (curCount == INITIAL_STRONG_VALUE) {
        atomic_add(-INITIAL_STRONG_VALUE, &impl->mStrong);
        impl->mBase->OnFirstRef();
    }

    return TRUE;
}

Boolean ElRefBase::WeakRefType::AttemptIncWeak(const void* id)
{
    WeakRefImpl* const impl = static_cast<WeakRefImpl*>(this);

    Int32 curCount = impl->mWeak;
    ELA_ASSERT_WITH_BLOCK(curCount >= 0) {
        ALOGE(" > ElRefBase: attemptIncWeak called on %p after underflow", this);
    }
    while (curCount > 0) {
        if (atomic_cmpxchg(curCount, curCount + 1, &impl->mWeak) == 0) {
            break;
        }
        curCount = impl->mWeak;
    }

    if (curCount > 0) {
        impl->AddWeakRef(id);
    }

    return curCount > 0;
}

Int32 ElRefBase::WeakRefType::GetWeakCount() const
{
    return static_cast<const WeakRefImpl*>(this)->mWeak;
}

void ElRefBase::WeakRefType::PrintRefs() const
{
    static_cast<const WeakRefImpl*>(this)->PrintRefs();
}

void ElRefBase::WeakRefType::TrackMe(Boolean enable, Boolean retain)
{
    static_cast<WeakRefImpl*>(this)->TrackMe(enable, retain);
}

ElRefBase::WeakRefType* ElRefBase::CreateWeak(const void* id) const
{
    mRefs->IncWeak(id);
    return mRefs;
}

ElRefBase::WeakRefType* ElRefBase::GetWeakRefs() const
{
    return mRefs;
}

UInt32 ElRefBase::AddRef()
{
    IncStrong(this);
    return GetStrongCount();
}

UInt32 ElRefBase::Release()
{
    UInt32 ref = GetStrongCount();
    DecStrong(this);
    return --ref;
}

ElRefBase::ElRefBase()
    : mRefs(new WeakRefImpl(this))
{}

ElRefBase::~ElRefBase()
{
    if (mRefs->mStrong == INITIAL_STRONG_VALUE) {
        // we never acquired a strong (and/or weak) reference on this object.
        delete mRefs;
    }
    else {
        // life-time of this object is extended to WEAK or FOREVER, in
        // which case weakref_impl doesn't out-live the object and we
        // can free it now.
        if ((mRefs->mFlags & OBJECT_LIFETIME_MASK) != OBJECT_LIFETIME_STRONG) {
            // It's possible that the weak count is not 0 if the object
            // re-acquired a weak reference in its destructor
            if (mRefs->mWeak == 0) {
                delete mRefs;
            }
        }
    }
    // for debugging purposes, clear this.
    const_cast<WeakRefImpl*&>(mRefs) = NULL;
}

void ElRefBase::ExtendObjectLifetime(Int32 mode)
{
    atomic_or(mode, &mRefs->mFlags);
}

void ElRefBase::OnFirstRef()
{}

void ElRefBase::OnLastStrongRef(const void* /*id*/)
{}

Boolean ElRefBase::OnIncStrongAttempted(UInt32 flags, const void* id)
{
    return (flags & FIRST_INC_STRONG) ? TRUE : FALSE;
}

void ElRefBase::OnLastWeakRef(const void* /*id*/)
{}

// ---------------------------------------------------------------------------

//void RefBase::moveReferences(void* dst, void const* src, size_t n,
//        const ReferenceConverterBase& caster)
//{
//#if DEBUG_REFS
//    const size_t itemSize = caster.getReferenceTypeSize();
//    for (size_t i=0 ; i<n ; i++) {
//        void*       d = reinterpret_cast<void      *>(intptr_t(dst) + i*itemSize);
//        void const* s = reinterpret_cast<void const*>(intptr_t(src) + i*itemSize);
//        RefBase* ref(reinterpret_cast<RefBase*>(caster.getReferenceBase(d)));
//        ref->mRefs->renameStrongRefId(s, d);
//        ref->mRefs->renameWeakRefId(s, d);
//    }
//#endif
//}


WeakReferenceImpl::WeakReferenceImpl(
    /* [in] */ IInterface* object,
    /* [in] */ ElRefBase::WeakRefType* ref)
    : mObject(object)
    , mRef(ref)
{}

WeakReferenceImpl::~WeakReferenceImpl()
{
    if (mRef != NULL) mRef->DecWeak(this);
}

PInterface WeakReferenceImpl::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)(IWeakReference*)this;
    }
    else if (riid == EIID_IWeakReference) {
        return (IWeakReference*)this;
    }

    return NULL;
}

UInt32 WeakReferenceImpl::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 WeakReferenceImpl::Release()
{
    return ElLightRefBase::Release();
}

ECode WeakReferenceImpl::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [in] */ InterfaceID* iid)
{
    if (object == (IInterface*)(IWeakReference*)this) {
        *iid = EIID_IWeakReference;
    }
    else {
        return E_INVALID_ARGUMENT;
    }
    return NOERROR;
}

ECode WeakReferenceImpl::Resolve(
    /* [in] */ const InterfaceID& riid,
    /* [out] */ IInterface** objectReference)
{
    if (objectReference == NULL) {
        return E_INVALID_ARGUMENT;
    }

    if (mObject && mRef->AttemptIncStrong(objectReference)) {
        *objectReference = mObject->Probe(riid);
        if (*objectReference == NULL) {
            mObject->Release();
        }
    }
    else {
        *objectReference = NULL;
    }
    return NOERROR;
}

_ELASTOS_NAMESPACE_END
