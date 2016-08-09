//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CINTERFACEINFO_H__
#define __CINTERFACEINFO_H__

#include "CEntryList.h"

class CInterfaceInfo
    : public ElLightRefBase
    , public IInterfaceInfo
{
public:
    CInterfaceInfo(
        /* [in] */ CClsModule* clsModule,
        /* [in] */ UInt32 index);

    virtual ~CInterfaceInfo();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI Init();

    CARAPI GetSize(
        /* [out] */ MemorySize* size);

    CARAPI GetDataType(
        /* [out] */ CarDataType* dataType);

    CARAPI GetName(
        /* [out] */ String* name);

    CARAPI GetNamespace(
        /* [out] */ String* ns);

    CARAPI GetId(
        /* [out] */ InterfaceID* id);

    CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo** moduleInfo);

    CARAPI IsLocal(
        /* [out] */ Boolean* local);

    CARAPI HasBase(
        /* [out] */ Boolean* hasBase);

    CARAPI GetBaseInfo(
        /* [out] */ IInterfaceInfo** baseInfo);

    CARAPI GetMethodCount(
        /* [out] */ Int32* count);

    CARAPI GetAllMethodInfos(
        /* [out] */ ArrayOf<IMethodInfo *>* methodInfos);

    CARAPI GetMethodInfo(
        /* [in] */ const String& name,
        /* [in] */ const String& signature,
        /* [out] */ IMethodInfo** methodInfo);

    CARAPI CreateIFList();

    CARAPI AcquireMethodList();

private:
    AutoPtr<CClsModule>     mClsModule;
    CLSModule*              mClsMod;
    AutoPtr<CEntryList>     mMethodList;
    InterfaceDirEntry*      mInterfaceDirEntry;
    InterfaceDescriptor*    mDesc;
    IFIndexEntry*           mIFList;

    UInt32  mIndex;
    UInt32  mIFCount;
    UInt32  mMethodCount;
    Int32   mBase;
};

#endif // __CINTERFACEINFO_H__
