//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CConstantInfo.h"

CConstantInfo::CConstantInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ ConstDirEntry* constDirEntry)
    : mClsModule(clsModule)
    , mConstDirEntry(constDirEntry)
{}

UInt32 CConstantInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CConstantInfo::Release()
{
    return ElLightRefBase::Release();
}

PInterface CConstantInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IConstantInfo) {
        return (IConstantInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CConstantInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CConstantInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = adjustNameAddr(mClsModule->mBase, mConstDirEntry->mName);
    return NOERROR;
}

ECode CConstantInfo::GetValue(
    /* [out] */ Int32* value)
{
    if (!value) {
        return E_INVALID_ARGUMENT;
    }

    if (mConstDirEntry->mType == TYPE_INTEGER32) {
        *value = mConstDirEntry->mV.mInt32Value.mValue;
        return NOERROR;
    }
    else return E_INVALID_OPERATION;
}

ECode CConstantInfo::GetModuleInfo(
    /* [out] */ IModuleInfo** moduleInfo)
{
    return mClsModule->GetModuleInfo(moduleInfo);
}
