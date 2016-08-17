//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __ELAOBJ_H__
#define __ELAOBJ_H__

#include <elstring.h>

extern "C" {

typedef interface IInterface IInterface;
typedef interface IInterface SynchronizedIObject;
typedef interface IAspect IAspect;

typedef interface IInterface* PInterface;
typedef interface IAspect* PASPECT;
typedef interface IRegime* PRegime;

typedef enum AggregateType
{
    AggrType_AspectAttach = 1,
    AggrType_AspectDetach,
    AggrType_Aggregate,
    AggrType_Unaggregate,
    AggrType_ObjectEnter,
    AggrType_ObjectLeave,
    AggrType_EnteredRegime,
    AggrType_LeftRegime,
    AggrType_Connect,
    AggrType_Disconnect,
    AggrType_AddConnection,
    AggrType_FriendEnter,
    AggrType_FriendLeave,
    AggrType_ChildConstruct,
    AggrType_ChildDestruct,
    AggrType_ParentAttach,
    AggrType_AspectDetached,
} AggregateType, AggrType;

EXTERN_C const _ELASTOS InterfaceID EIID_IInterface;
EXTERN_C const _ELASTOS InterfaceID EIID_IObject;
EXTERN_C const _ELASTOS InterfaceID EIID_IAspect;
EXTERN_C const _ELASTOS InterfaceID EIID_IProxy;
EXTERN_C const _ELASTOS InterfaceID EIID_IProxyDeathRecipient;
EXTERN_C const _ELASTOS InterfaceID EIID_IWeakReference;
EXTERN_C const _ELASTOS InterfaceID EIID_IWeakReferenceSource;

#define BASE_INTERFACE_METHOD_COUNT 4

CAR_INTERFACE("00000000-0000-0000-C000-000000000066")
IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IInterface*) Probe(
        /* [in] */ PInterface object)
    {
        if (object == NULL) return NULL;
        return (IInterface*)object->Probe(EIID_IInterface);
    }

    virtual CARAPI_(_ELASTOS UInt32) AddRef() = 0;

    virtual CARAPI_(_ELASTOS UInt32) Release() = 0;

    virtual CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ _ELASTOS InterfaceID* iid) = 0;
};

CAR_INTERFACE("00000000-0000-0000-C000-000000000068")
IObject : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IObject*) Probe(
        /* [in] */ PInterface object)
    {
        if (object == NULL) return NULL;
        return (IObject*)object->Probe(EIID_IObject);
    }

    virtual CARAPI Aggregate(
        /* [in] */ AggregateType type,
        /* [in] */ IInterface* object) = 0;

    virtual CARAPI GetDomain(
        /* [out] */ IInterface** object) = 0;

    virtual CARAPI GetClassID(
        /* [out] */ _ELASTOS ClassID* clsid) = 0;

    virtual CARAPI Equals(
        /* [in] */ IInterface* object,
        /* [out] */ _ELASTOS Boolean* equals) = 0;

    virtual CARAPI GetHashCode(
        /* [out] */ _ELASTOS Int32* hashCode) = 0;

    virtual CARAPI ToString(
        /* [out] */ _ELASTOS String* info) = 0;
};

struct CCarObject{};

CAR_INTERFACE("00010002-0000-0000-C000-000000000066")
IAspect : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IAspect*) Probe(
        /* [in] */ PInterface object)
    {
        if (object == NULL) return NULL;
        return (IAspect*)object->Probe(EIID_IAspect);
    }

    virtual CARAPI AspectAggregate(
        /* [in] */ AggregateType type,
        /* [in] */ PInterface object) = 0;

    virtual CARAPI AspectGetDomain(
        /* [out] */ PInterface* object) = 0;

    virtual CARAPI GetAspectID(
        /* [out] */ _ELASTOS ClassID* clsid) = 0;
};

interface ICallbackSink;

CAR_INTERFACE("00010004-0000-0000-C000-000000000066")
ICallbackConnector : public IInterface
{
    virtual CARAPI AcquireCallbackSink(
        /* [in] */ ICallbackSink** callbackSink) = 0;

    virtual CARAPI CheckCallbackSinkConnection(
        /* [in] */ _ELASTOS Int32 event) = 0;

    virtual CARAPI DisconnectCallbackSink() = 0;
};

interface IProxyDeathRecipient;
struct _CIClassInfo;

CAR_INTERFACE("00010005-0000-0000-C000-000000000066")
IProxy : public IInterface
{
    virtual CARAPI GetInterface(
        /* [in] */ _ELASTOS UInt32 index,
        /* [out] */ IInterface** object) = 0;

    virtual CARAPI GetInterfaceIndex(
        /* [in] */ IInterface* object,
        /* [out] */ _ELASTOS UInt32* index) = 0;

    virtual CARAPI  GetClassID(
        /* [out] */ _ELASTOS EMuid* clsid) = 0;

    virtual CARAPI GetClassInfo(
        /* [out] */ _CIClassInfo** classInfo) = 0;

    virtual CARAPI IsStubAlive(
        /* [out] */ _ELASTOS Boolean* result) = 0;

    virtual CARAPI LinkToDeath(
        /* [in] */ IProxyDeathRecipient* recipient,
        /* [in] */ _ELASTOS Int32 flags) = 0;

    virtual CARAPI UnlinkToDeath(
        /* [in] */ IProxyDeathRecipient* recipient,
        /* [in] */ _ELASTOS Int32 flags,
        /* [out] */ _ELASTOS Boolean* result) = 0;
};

CAR_INTERFACE("0001000D-0000-0000-C000-000000000066")
IProxyDeathRecipient : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IProxyDeathRecipient*) Probe(
        /* [in] */ PInterface object)
    {
        if (object == NULL) return NULL;
        return (IProxyDeathRecipient*)object->Probe(EIID_IProxyDeathRecipient);
    }

    virtual CARAPI ProxyDied() = 0;
};

CAR_INTERFACE("00010008-0000-0000-C000-000000000066")
IWeakReference : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IWeakReference*) Probe(
        /* [in] */ PInterface object)
    {
        if (object == NULL) return NULL;
        return (IWeakReference*)object->Probe(EIID_IWeakReference);
    }

    virtual CARAPI Resolve(
        /* [in] */ _ELASTOS REIID riid,
        /* [out] */ IInterface** objectReference) = 0;
};

CAR_INTERFACE("0001000A-0000-0000-C000-000000000066")
IWeakReferenceSource : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IWeakReferenceSource*) Probe(
        /* [in] */ PInterface object)
    {
        if (object == NULL) return NULL;
        return (IWeakReferenceSource*)object->Probe(EIID_IWeakReferenceSource);
    }

    virtual CARAPI GetWeakReference(
        /* [out] */ IWeakReference** weakReference) = 0;
};

}   // extern "C"

// Helper types Small and Big - guarantee that sizeof(Small) < sizeof(Big)
//
template <class T, class U>
struct ConversionHelper
{
    typedef char Small;
    struct Big { char dummy[2]; };
    static Big   Test(...);
    static Small Test(U);
    static T & MakeT();
};

// class template Conversion
// Figures out the conversion relationships between two types
// Invocations (T and U are types):
// exists: returns (at compile time) true if there is an implicit conversion
// from T to U (example: Derived to Base)
// Caveat: might not work if T and U are in a private inheritance hierarchy.
//
template <class T, class U>
struct Conversion
{
    typedef ConversionHelper<T, U> H;
    enum {
        exists = sizeof(typename H::Small) == sizeof((H::Test(H::MakeT())))
    };
    enum { exists2Way = exists && Conversion<U, T>::exists };
    enum { sameType = FALSE };
};

template <class T>
class Conversion<T, T>
{
public:
    enum { exists = TRUE, exists2Way = TRUE, sameType = TRUE };
};

template <>
struct Conversion<void, CCarObject>
{
    enum { exists = FALSE, exists2Way = FALSE, sameType = FALSE };
};

#ifndef SUPERSUBCLASS
#define SUPERSUBCLASS(SuperT, SubT) \
    (Conversion<SubT*, SuperT*>::exists && !Conversion<SuperT*, void*>::sameType)
#endif

#ifndef SUPERSUBCLASS_EX
#define SUPERSUBCLASS_EX(Super, Sub) \
    (Conversion<Sub, Super>::exists && !Conversion<Super, void*>::sameType)
#endif

#ifndef DEFINE_CONVERSION_FOR
#define DEFINE_CONVERSION_FOR(DerivedT, BaseT)                              \
template <>                                                                 \
struct Conversion<DerivedT*, BaseT*>                                        \
{                                                                           \
    enum { exists = TRUE, exists2Way = FALSE, sameType = FALSE };           \
};
#endif

#endif // __ELAOBJ_H__
