//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CClsModule.h"
#include "CEntryList.h"
#include <dlfcn.h>

CClsModule::CClsModule(
    /* [in] */ CLSModule* clsMod,
    /* [in] */ Boolean allocedClsMod,
    /* [in] */ const String& path,
    /* [in] */ Void* module)
{
    mClsMod = clsMod;
    mAllocedClsMode = allocedClsMod;
    mPath = path;
    mTypeAliasList = NULL;
    mModule = NULL;
    if (!mAllocedClsMode && module) {
        mModule = module;
    }

    if (allocedClsMod) {
        mBase = 0;
    }
    else {
        mBase = Int32(clsMod);
    }
}

CClsModule::~CClsModule()
{
    if (mTypeAliasList) {
        for (Int32 i = 0; i < mClsMod->mAliasCount; i++) {
            if (mTypeAliasList[i].mOrgTypeDesc) {
                delete mTypeAliasList[i].mOrgTypeDesc;
            }
        }
        delete[] mTypeAliasList;
    }

    if (mAllocedClsMode) {
        if (mClsMod) DisposeFlattedCLS(mClsMod);
    }
    else {
        if (mModule) dlclose(mModule);
    }
}

UInt32 CClsModule::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CClsModule::Release()
{
    g_objInfoList.LockHashTable(EntryType_ClsModule);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        g_objInfoList.RemoveClsModule(mPath);
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_ClsModule);
    assert(ref >= 0);
    return ref;
}

ECode CClsModule::GetModuleInfo(
    /* [out] */ IModuleInfo** moduleInfo)
{
    if (!moduleInfo) {
        return E_INVALID_ARGUMENT;
    }

    return _CReflector_AcquireModuleInfo(
            String(adjustNameAddr(mBase, mClsMod->mUunm)), moduleInfo);
}

ECode CClsModule::InitOrgType()
{
    if (!mClsMod->mAliasCount) {
        return NOERROR;
    }

    Int32 i = 0;
    mTypeAliasList = new TypeAliasDesc[mClsMod->mAliasCount];
    if (!mTypeAliasList) {
        return E_OUT_OF_MEMORY;
    }

    memset(mTypeAliasList, 0, sizeof(TypeAliasDesc) * mClsMod->mAliasCount);

    for (i = 0; i < mClsMod->mAliasCount; i++) {
        mTypeAliasList[i].mOrgTypeDesc = new TypeDescriptor;
        if (!mTypeAliasList[i].mOrgTypeDesc) goto EExit;
        mTypeAliasList[i].mTypeDesc = &(getAliasDirAddr(mBase,
                mClsMod->mAliasDirs, i)->mType);
        _GetOriginalType(this, mTypeAliasList[i].mTypeDesc,
                mTypeAliasList[i].mOrgTypeDesc);
    }

    return NOERROR;

EExit:
    for (i = 0; i < mClsMod->mAliasCount; i++) {
        if (mTypeAliasList[i].mOrgTypeDesc) {
            delete mTypeAliasList[i].mOrgTypeDesc;
        }
    }

    delete[] mTypeAliasList;
    mTypeAliasList = NULL;

    return E_OUT_OF_MEMORY;
}

ECode CClsModule::AliasToOriginal(
    /* [in] */ TypeDescriptor* typeDype,
    /* [out] */ TypeDescriptor** orgTypeDesc)
{
    if (!mClsMod->mAliasCount) {
        return E_INVALID_OPERATION;
    }

    if (typeDype->mType != Type_alias) {
        return E_INVALID_ARGUMENT;
    }

    if (!mTypeAliasList) {
        ECode ec = InitOrgType();
        if (FAILED(ec)) {
            return ec;
        }
    }

//    while (typeDype->type == Type_alias) {
//        typeDype = &mClsMod->mAliasDirs[typeDype->mIndex]->type;
//    }

    if (typeDype->mIndex >= mClsMod->mAliasCount) {
        return E_INVALID_ARGUMENT;
    }

    *orgTypeDesc = mTypeAliasList[typeDype->mIndex].mOrgTypeDesc;

    return NOERROR;
}
