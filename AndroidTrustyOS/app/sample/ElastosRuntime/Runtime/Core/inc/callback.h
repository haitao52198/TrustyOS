
#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#ifndef INVALID_ADDRVALUE
#define INVALID_ADDRVALUE           ((virtaddr_t)(0xcccccccc))
#endif

ELAPI_(void) _Impl_CallbackSink_FreeCallbackEvent(
            _ELASTOS PVoid callbackEvent);

#include <semaphore.h>
#include <time.h>
#include <elaatomics.h>

typedef _ELASTOS Int32 CallbackEventId;
typedef _ELASTOS Int32 CallbackEventFlags;

#define CallbackEventFlag_PriorityMask  0x000000FF
#define CallbackEventFlag_Normal        0x00000000
#define CallbackEventFlag_Duplicated    0x00000100
#define CallbackEventFlag_SyncCall      0x00000200
#define CallbackEventFlag_DirectCall    0x00000400

#define CallingStatus_Created       0x01
#define CallingStatus_Running       0x02
#define CallingStatus_Completed     0x03
#define CallingStatus_Cancelled     0x04

typedef _ELASTOS ECode (ELFUNCCALLTYPE *PCallbackEventHandleRoutine)(_ELASTOS PVoid thisObj);

typedef _ELASTOS ECode (ELFUNCCALLTYPE *PCoalesceEventFunc)(_ELASTOS PVoid* oldEvent, _ELASTOS PVoid* newEvent);

class CarCallbackEvent
{
public:
    CarCallbackEvent()
        : mID(0)
        , mFlags(0)
        , mSender(NULL)
        , mReceiver(NULL)
        , mCoalesceFunc(NULL)
        , mHandlerThis(NULL)
        , mHandlerFunc(NULL)
        , mTypeOfFunc(0)
        , mRet(NOERROR)
        , mStatus(CallingStatus_Created)
        , mCompleted(0)
        , mParameters(NULL)
        , mPrev(NULL)
        , mNext(NULL)
        , mWhen(0)
        , mRef(0)
    {
        sem_init(&mSyncEvent, 0, 0);
    }

    CarCallbackEvent(
        /* [in] */ CallbackEventId id,
        /* [in] */ CallbackEventFlags flags,
        /* [in] */ PInterface sender,
        /* [in] */ PInterface receiver,
        /* [in] */ _ELASTOS PVoid coalesceFunc,
        /* [in] */ _ELASTOS PVoid handlerThis,
        /* [in] */ _ELASTOS PVoid handlerFunc,
        /* [in] */ _ELASTOS Int32 typeOfFunc,
        /* [in] */ IParcel* parameters)
        : mID(id)
        , mFlags(flags)
        , mSender(sender)
        , mReceiver(receiver)
        , mCoalesceFunc(coalesceFunc)
        , mHandlerThis(handlerThis)
        , mHandlerFunc(handlerFunc)
        , mTypeOfFunc(typeOfFunc)
        , mRet(NOERROR)
        , mStatus(CallingStatus_Created)
        , mCompleted(0)
        , mParameters(parameters)
        , mPrev(NULL)
        , mNext(NULL)
        , mWhen(0)
        , mRef(0)
    {
        sem_init(&mSyncEvent, 0, 0);
    }

    virtual ~CarCallbackEvent()
    {
        sem_post(&mSyncEvent);
        sem_destroy(&mSyncEvent);
    }

    _ELASTOS UInt32 AddRef()
    {
        _ELASTOS Int32 ref = atomic_inc(&mRef);
        return (_ELASTOS UInt32)ref;
    }

    _ELASTOS UInt32 Release()
    {
        _ELASTOS Int32 nRef = atomic_dec(&mRef);
        if (nRef == 0) _Impl_CallbackSink_FreeCallbackEvent(this);
        return nRef;
    }

    CarCallbackEvent* Prev() const { return mPrev; }

    // Return the next node.
    CarCallbackEvent* Next() const { return mNext; }

    CarCallbackEvent* Detach()
    {
        mPrev->mNext = mNext;
        mNext->mPrev = mPrev;
    #ifdef _DEBUG
        mPrev = mNext = (CarCallbackEvent *)INVALID_ADDRVALUE;
    #endif

        return this;
    }

    void Initialize() { mPrev = mNext = this; }

    _ELASTOS Boolean IsEmpty() const { return this == mNext; }

    CarCallbackEvent* First() const { return Next(); }

    CarCallbackEvent* InsertNext(
        /* [in] */ CarCallbackEvent* nextNode)
    {
        assert(nextNode);

        nextNode->mPrev = this;
        nextNode->mNext = mNext;
        mNext->mPrev = nextNode;
        mNext = nextNode;

        return nextNode;
    }

    CarCallbackEvent* InsertFirst(
        /* [in] */ CarCallbackEvent* firstNode)
    {
        return InsertNext(firstNode);
    }

public:
    CallbackEventId                 mID;
    CallbackEventFlags              mFlags;
    _ELASTOS AutoPtr<IInterface>    mSender;
    PInterface                      mReceiver;
    _ELASTOS PVoid                  mCoalesceFunc;
    _ELASTOS PVoid                  mHandlerThis;
    _ELASTOS PVoid                  mHandlerFunc;
    _ELASTOS Int32                  mTypeOfFunc;
    _ELASTOS ECode                  mRet;
    sem_t                           mSyncEvent;
    _ELASTOS Int16                  mStatus;
    _ELASTOS Int16                  mCompleted;
    _ELASTOS AutoPtr<IParcel>       mParameters;
    CarCallbackEvent*               mPrev;
    CarCallbackEvent*               mNext;
    _ELASTOS Int64          mWhen;

private:
    _ELASTOS Int32                  mRef;
};

typedef class CarCallbackEvent* PCallbackEvent;

ELAPI _Impl_CallbackSink_InitCallbackContext(
    /* [out] */ PInterface* callbackContext);

ELAPI _Impl_CallbackSink_GetCallbackContext(
    /* [out] */ PInterface* callbackContext);

ELAPI _Impl_CallbackSink_AcquireCallbackContext(
    /* [out] */ PInterface* callbackContext);

ELAPI_(PCallbackEvent) _Impl_CallbackSink_AllocCallbackEvent(
    /* [in] */ _ELASTOS Int32 size);

ELAPI _Impl_CallbackSink_GetThreadEvent(
    /* [in] */ PInterface callbackContext,
    /* [out] */ sem_t* event);

ELAPI _Impl_CallbackSink_PostCallbackEvent(
    /* [in] */ PInterface callbackContext,
    /* [in] */ PCallbackEvent callbackEvent);

ELAPI _Impl_CallbackSink_PostCallbackEventAtTime(
    /* [in] */ PInterface callbackContext,
    /* [in] */ PCallbackEvent callbackEvent,
    /* [in] */ _ELASTOS Int64 uptimeMillis);

ELAPI _Impl_CallbackSink_SendCallbackEvent(
    /* [in] */ PInterface callbackContext,
    /* [in] */ PCallbackEvent callbackEvent,
    /* [in] */ _ELASTOS Int32 timeOut);

ELAPI _Impl_CallbackSink_WaitForCallbackEvent(
    /* [in] */ PInterface callbackContext,
    /* [in] */ _ELASTOS Int32 msTimeOut,
    /* [in] */ WaitResult* result,
    /* [out] */ _ELASTOS Boolean* eventOccured,
    /* [in] */ _ELASTOS UInt32 priority);

ELAPI _Impl_CallbackSink_CleanupAllCallbacks(
    /* [in] */ PInterface callbackContext);

ELAPI _Impl_CallbackSink_CancelAllPendingCallbacks(
    /* [in] */ PInterface callbackContext,
    /* [in] */ PInterface sender);

ELAPI _Impl_CallbackSink_CancelPendingCallback(
    /* [in] */ PInterface callbackContext,
    /* [in] */ PInterface sender,
    /* [in] */ CallbackEventId id);

ELAPI _Impl_CallbackSink_CancelCallbackEvents(
    /* [in] */ PInterface callbackContext,
    /* [in] */ PInterface sender,
    /* [in] */ CallbackEventId id,
    /* [in] */ _ELASTOS PVoid handlerThis,
    /* [in] */ _ELASTOS PVoid handlerFunc);

ELAPI _Impl_CallbackSink_RequestToFinish(
    /* [in] */ PInterface callbackContext,
    /* [in] */ _ELASTOS Int32 flag);

ELAPI_(_ELASTOS Int32) _Impl_CallbackSink_GetStatus(
    /* [in] */ PInterface callbackContext);

ELAPI _Impl_CallbackSink_TryToHandleEvents(
    /* [in] */ PInterface callbackContext);

ELAPI _Impl_CCallbackRendezvous_New(
    /* [in] */ PInterface callbackContext,
    /* [in] */ ICallbackSink* callbackSink,
    /* [in] */ CallbackEventId eventId,
    /* [in] */ _ELASTOS Boolean* eventFlag,
    /* [in] */ _ELASTOS Boolean newCallback,
    /* [out] */ ICallbackRendezvous** callbackRendezvous);

//
//  struct DelegateNode
//
struct DelegateNode
{
    DelegateNode() : mCallbackContext(NULL) {}

    DelegateNode(
        /* [in] */ PInterface callbackContext,
        /* [in] */ _ELASTOS EventHandler delegate)
        : mCallbackContext(callbackContext)
        , mDelegate(delegate)
    {
        assert(callbackContext);
    }

    _ELASTOS ECode CancelCallbackEvents(
        /* [in] */ PInterface sender,
        /* [in] */ CallbackEventId id)
    {
        return _Impl_CallbackSink_CancelCallbackEvents(
                mCallbackContext, sender, id,
                mDelegate.GetThisPtr(),
                mDelegate.GetFuncPtr());
    }

    DelegateNode* Prev()
    {
        return mPrev;
    }

    DelegateNode* Next()
    {
        return mNext;
    }

    DelegateNode* InsertPrev(
        /* [in] */ DelegateNode* prevNode)
    {
        assert(prevNode);

        prevNode->mPrev = mPrev;
        prevNode->mNext = this;
        mPrev->mNext = prevNode;
        mPrev = prevNode;

        return prevNode;
    }

    DelegateNode* Detach()
    {
        mPrev->mNext = mNext;
        mNext->mPrev = mPrev;
    #ifdef _DEBUG
        mPrev = mNext = (DelegateNode *)INVALID_ADDRVALUE;
    #endif

        return this;
    }

    void Initialize()
    {
        mPrev = mNext = this;
    }

    _ELASTOS Boolean IsEmpty()
    {
        return this == mNext;
    }

    DelegateNode* First()
    {
        return Next();
    }

    PInterface              mCallbackContext;
    _ELASTOS EventHandler   mDelegate;
    DelegateNode*           mPrev;
    DelegateNode*           mNext;
};

class DelegateContainer
{
public:
    DelegateContainer();

    _ELASTOS ECode Current(
        /* [in] */ DelegateNode** node);

    _ELASTOS ECode MoveNext();

    _ELASTOS ECode Reset();

    _ELASTOS ECode Add(
        /* [in] */ PInterface callbackContext,
        /* [in] */ const _ELASTOS EventHandler& delegate);

    _ELASTOS ECode Remove(
        /* [in] */ _ELASTOS EventHandler& delegate,
        /* [in] */ PInterface sender,
        /* [in] */ CallbackEventId id);

    _ELASTOS ECode Dispose(
        /* [in] */ PInterface sender,
        /* [in] */ CallbackEventId id);

    ~DelegateContainer();

public:
    DelegateNode    mHead;
    DelegateNode*   mCurrent;
};

CAR_INLINE DelegateContainer::DelegateContainer()
{
    mHead.Initialize();
    mCurrent = &mHead;
}

CAR_INLINE DelegateContainer::~DelegateContainer()
{
    DelegateNode* node;

    while (!mHead.IsEmpty()) {
        node = (DelegateNode *)mHead.First();
        node->Detach();
        delete node;
    }
    mCurrent = NULL;
}

CAR_INLINE _ELASTOS ECode DelegateContainer::Dispose(
    /* [in] */ PInterface sender,
    /* [in] */ CallbackEventId id)
{
    DelegateNode* node;

    while (!mHead.IsEmpty()) {
        node = (DelegateNode *)mHead.First();
        node->Detach();
        node->CancelCallbackEvents(sender, id);
        delete node;
    }
    mCurrent = NULL;
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode DelegateContainer::Current(
    /* [in] */ DelegateNode** node)
{
    assert(NULL != node);

    if (NULL == mCurrent || &mHead == mCurrent) {
        return E_INVALID_OPERATION;
    }
    *node = mCurrent;
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode DelegateContainer::MoveNext()
{
    if (NULL == mCurrent) {
        return E_INVALID_OPERATION;
    }
    if (mCurrent == mHead.Prev()) {
        mCurrent = NULL;
        return S_FALSE;
    }
    mCurrent = mCurrent->Next();
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode DelegateContainer::Reset()
{
    mCurrent = &mHead;
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode DelegateContainer::Add(
    /* [in] */ PInterface callbackContext,
    /* [in] */ const _ELASTOS EventHandler& delegate)
{
    DelegateNode* node;

    assert(callbackContext);

    node = new DelegateNode(callbackContext, delegate);
    if (NULL == node) {
        return E_OUT_OF_MEMORY;
    }

    mHead.InsertPrev(node);
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode DelegateContainer::Remove(
    /* [in] */ _ELASTOS EventHandler& delegate,
    /* [in] */ PInterface sender,
    /* [in] */ CallbackEventId id)
{
    DelegateNode* node;
    _ELASTOS ECode ec = NOERROR;

    for (node = mHead.mNext; node != &mHead; node = node->mNext) {
        if (node->mDelegate == delegate) {
            if (mCurrent == node) {
                mCurrent = mCurrent->Prev();
            }
            node->Detach();
            ec = node->CancelCallbackEvents(sender, id);
            delete node;
            break;
        }
    }

    return ec;
}

//
//  struct CallbackContextNode
//
struct CallbackContextNode
{
    CallbackContextNode() : mCallbackContext(NULL) {}

    CallbackContextNode(
        /* [in] */ PInterface callbackContext,
        /* [in] */ _ELASTOS Boolean occured)
        : mCallbackContext(callbackContext)
        , mEventOccured(occured)
        , mEmpty(TRUE)
    {
        assert(mCallbackContext);
    }

    void Initialize();

    CallbackContextNode* Prev();

    CallbackContextNode* Next();

    CallbackContextNode* InsertPrev(
        /* [in] */ CallbackContextNode* prevNode);

    CallbackContextNode* Detach();

    _ELASTOS Boolean IsEmpty();

    _ELASTOS ECode MoveNext();

    _ELASTOS ECode Reset();

    _ELASTOS ECode Current(
        /* [out] */ _ELASTOS EventHandler** delegate);

    _ELASTOS ECode Add(
        /* [in] */ const _ELASTOS EventHandler& delegate);

    _ELASTOS ECode Remove(
        /* [in] */ _ELASTOS EventHandler& delegate,
        /* [in] */ PInterface sender,
        /* [in] */ CallbackEventId id);

    _ELASTOS ECode Dispose(
        /* [in] */ PInterface sender,
        /* [in] */ CallbackEventId id);

    _ELASTOS AutoPtr<IInterface>    mCallbackContext;
    DelegateContainer               mDelegateContainer;
    _ELASTOS Boolean                mEventOccured;
    _ELASTOS Boolean                mEmpty;
    CallbackContextNode*            mPrev;
    CallbackContextNode*            mNext;
};

CAR_INLINE void CallbackContextNode::Initialize()
{
    mPrev = mNext = this;
}

CAR_INLINE CallbackContextNode* CallbackContextNode::Prev()
{
    return mPrev;
}

CAR_INLINE CallbackContextNode* CallbackContextNode::Next()
{
    return mNext;
}

CAR_INLINE CallbackContextNode* CallbackContextNode::InsertPrev(
    /* [in] */ CallbackContextNode* prevNode)
{
    assert(prevNode);

    prevNode->mPrev = mPrev;
    prevNode->mNext = this;
    mPrev->mNext = prevNode;
    mPrev = prevNode;

    return prevNode;
}

CAR_INLINE CallbackContextNode* CallbackContextNode::Detach()
{
    mPrev->mNext = mNext;
    mNext->mPrev = mPrev;
#ifdef _DEBUG
    mPrev = mNext = (CallbackContextNode *)INVALID_ADDRVALUE;
#endif

    return this;
}

CAR_INLINE _ELASTOS Boolean CallbackContextNode::IsEmpty()
{
    return this == mNext;
}

CAR_INLINE _ELASTOS ECode CallbackContextNode::MoveNext()
{
    return mDelegateContainer.MoveNext();
}

CAR_INLINE _ELASTOS ECode CallbackContextNode::Reset()
{
    return mDelegateContainer.Reset();
}

CAR_INLINE _ELASTOS ECode CallbackContextNode::Dispose(
    /* [in] */ PInterface sender,
    /* [in] */ CallbackEventId id)
{
    _ELASTOS ECode ec = mDelegateContainer.Dispose(sender, id);
    if (FAILED(ec)) return ec;
    Detach();
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode CallbackContextNode::Current(
    /* [in] */ _ELASTOS EventHandler** delegate)
{
    _ELASTOS ECode ec;
    DelegateNode* node;

    ec = mDelegateContainer.Current(&node);
    if (NOERROR == ec) {
        *delegate = &node->mDelegate;
    }
    return ec;
}

CAR_INLINE _ELASTOS ECode CallbackContextNode::Add(
    /* [in] */ const _ELASTOS EventHandler& delegate)
{
    mEmpty = FALSE;
    return mDelegateContainer.Add(mCallbackContext, delegate);
}

CAR_INLINE _ELASTOS ECode CallbackContextNode::Remove(
    /* [in] */ _ELASTOS EventHandler& delegate,
    /* [in] */ PInterface sender,
    /* [in] */ CallbackEventId id)
{
    _ELASTOS ECode ec = mDelegateContainer.Remove(delegate, sender, id);
    if (FAILED(ec)) return ec;

    if (mDelegateContainer.mHead.IsEmpty()) {
        mEmpty = TRUE;
        Detach();
    }
    return NOERROR;
}

//
//  class CallbackContextContainer
//
class CallbackContextContainer
{
public:
    CallbackContextContainer()
        : mEventOccured(FALSE)
    {
        mHead.Initialize();
        mCurrent = &mHead;
    }

    ~CallbackContextContainer();

    _ELASTOS ECode Find(
        /* [in] */ PInterface callbackContext,
        /* [in] */ CallbackContextNode** node);

    _ELASTOS ECode Current(
        /* [in] */ CallbackContextNode** node);

    _ELASTOS ECode MoveNext();

    _ELASTOS ECode Add(
        /* [in] */ PInterface callbackContext,
        /* [in] */ const _ELASTOS EventHandler& delegate);

    _ELASTOS ECode Remove(
        /* [in] */ CallbackContextNode* node);

    _ELASTOS ECode Remove(
        /* [in] */ PInterface callbackContext,
        /* [in] */ _ELASTOS EventHandler& delegate,
        /* [in] */ PInterface sender,
        /* [in] */ CallbackEventId id);

    _ELASTOS ECode Dispose(
        /* [in] */ PInterface callbackContext,
        /* [in] */ PInterface sender,
        /* [in] */ CallbackEventId id);

    _ELASTOS ECode Reset();

public:
    CallbackContextNode     mHead;
    CallbackContextNode*    mCurrent;
    _ELASTOS Boolean        mEventOccured;
};

CAR_INLINE CallbackContextContainer::~CallbackContextContainer()
{
    CallbackContextNode* node;

    Reset();
    assert(mHead.IsEmpty() && "You should call CFoo::RemoveAllCallbacks first.");

    while (MoveNext() == NOERROR) {
        if (Current(&node) != NOERROR) {
            break;
        }
        Remove(node);
        delete node;
    }
    assert(mHead.IsEmpty());
    mCurrent = NULL;
}

CAR_INLINE _ELASTOS ECode CallbackContextContainer::Find(
    /* [in] */ PInterface callbackContext,
    /* [in] */ CallbackContextNode** node)
{
    CallbackContextNode* currNode;

    for (currNode = mHead.mNext; currNode != &mHead; currNode = currNode->mNext) {
        if (currNode->mCallbackContext.Get() == callbackContext) {
            *node = currNode;
            return NOERROR;
        }
    }
    *node = NULL;
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode CallbackContextContainer::Dispose(
    /* [in] */ PInterface callbackContext,
    /* [in] */ PInterface sender,
    /* [in] */ CallbackEventId id)
{
    CallbackContextNode* node;

    node = mHead.Next();
    while (&mHead != node) {
        if (node->mCallbackContext.Get() == callbackContext) {
            node->Dispose(sender, id);
            delete node;
            break;
        }
        node = node->Next();
    }
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode CallbackContextContainer::Current(
    /* [in] */ CallbackContextNode** node)
{
    assert(NULL != node);

    if (NULL == mCurrent || &mHead == mCurrent) {
        return E_INVALID_OPERATION;
    }
    *node = mCurrent;
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode CallbackContextContainer::MoveNext()
{
    if (NULL == mCurrent) {
        return E_INVALID_OPERATION;
    }
    if (mCurrent == mHead.Prev()) {
        mCurrent = NULL;
        return S_FALSE;
    }
    mCurrent = mCurrent->Next();
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode CallbackContextContainer::Add(
    /* [in] */ PInterface callbackContext,
    /* [in] */ const _ELASTOS EventHandler& delegate)
{
    _ELASTOS Boolean found = FALSE;
    CallbackContextNode* node;
    _ELASTOS ECode ec;

    if (NULL == callbackContext) {
        ec = _Impl_CallbackSink_GetCallbackContext(&callbackContext);
        if (FAILED(ec)) return ec;
    }
    else {
        callbackContext->AddRef();
    }

    for (node = mHead.mNext; node != &mHead; node = node->mNext) {
        if (node->mCallbackContext.Get() == callbackContext) {
            found = TRUE;
            break;
        }
    }

    if (!found) {
        node = new CallbackContextNode(callbackContext, FALSE);
        if (NULL == node) {
            callbackContext->Release();
            return E_OUT_OF_MEMORY;
        }
        mHead.InsertPrev(node);
    }

    ec = node->Add(delegate);
    callbackContext->Release();

    return ec;
}

CAR_INLINE _ELASTOS ECode CallbackContextContainer::Remove(
    /* [in] */ CallbackContextNode* node)
{
    if (mCurrent == node) {
        mCurrent = mCurrent->Prev();
    }
    node->Detach();
    return NOERROR;
}

CAR_INLINE _ELASTOS ECode CallbackContextContainer::Remove(
    /* [in] */ PInterface callbackContext,
    /* [in] */ _ELASTOS EventHandler& delegate,
    /* [in] */ PInterface sender,
    /* [in] */ CallbackEventId id)
{
    assert(callbackContext);

    CallbackContextNode* node;

    for (node = mHead.mNext; node != &mHead; node = node->mNext) {
        if (node->mCallbackContext.Get() == callbackContext) {
            _ELASTOS ECode ec = node->Remove(delegate, sender, id);
            if (FAILED(ec)) return ec;
            if (node->mEmpty) delete node;
            return NOERROR;
        }
    }

    return NOERROR;
}

CAR_INLINE _ELASTOS ECode CallbackContextContainer::Reset()
{
    mCurrent = &mHead;
    return NOERROR;
}

#endif //__CALLBACK_H__
