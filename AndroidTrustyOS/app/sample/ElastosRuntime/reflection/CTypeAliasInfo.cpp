//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CTypeAliasInfo.h"
#include "CObjInfoList.h"

CTypeAliasInfo::CTypeAliasInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ AliasDirEntry* aliasDirEntry)
{
    mClsModule = clsModule;
    mAliasDirEntry = aliasDirEntry;
}

UInt32 CTypeAliasInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CTypeAliasInfo::Release()
{
    g_objInfoList.LockHashTable(EntryType_TypeAliase);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        g_objInfoList.RemoveTypeAliasInfo(mAliasDirEntry);
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_TypeAliase);
    assert(ref >= 0);
    return ref;
}

PInterface CTypeAliasInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_ITypeAliasInfo) {
        return (ITypeAliasInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CTypeAliasInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CTypeAliasInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = adjustNameAddr(mClsModule->mBase, mAliasDirEntry->mName);
    return NOERROR;
}

ECode CTypeAliasInfo::GetTypeInfo(
    /* [out] */ IDataTypeInfo** typeInfo)
{
    return g_objInfoList.AcquireDataTypeInfo(mClsModule,
            &mAliasDirEntry->mType, typeInfo);
}

ECode CTypeAliasInfo::GetModuleInfo(
    /* [out] */ IModuleInfo** moduleInfo)
{
    return mClsModule->GetModuleInfo(moduleInfo);
}

ECode CTypeAliasInfo::IsDummy(
    /* [out] */ Boolean* isDummy)
{
    if (!isDummy) {
        return E_INVALID_ARGUMENT;
    }

    *isDummy = mAliasDirEntry->mIsDummyType;
    return NOERROR;
}

ECode CTypeAliasInfo::GetPtrLevel(
    /* [out] */ Int32* level)
{
    if (!level) {
        return E_INVALID_ARGUMENT;
    }

    *level = mAliasDirEntry->mType.mPointer;
    return NOERROR;
}
