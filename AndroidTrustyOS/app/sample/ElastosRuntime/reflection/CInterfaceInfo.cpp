//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CInterfaceInfo.h"

CInterfaceInfo::CInterfaceInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ UInt32 index)
{
    mClsModule = clsModule;
    mClsMod = mClsModule->mClsMod;
    mBase = mClsModule->mBase;
    mInterfaceDirEntry = getInterfaceDirAddr(mBase, mClsMod->mInterfaceDirs, index);
    mDesc = adjustInterfaceDescAddr(mBase, mInterfaceDirEntry->mDesc);
    mIndex = index;
    mIFList = NULL;
}

CInterfaceInfo::~CInterfaceInfo()
{
    if (mIFList) delete[] mIFList;
}

UInt32 CInterfaceInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CInterfaceInfo::Release()
{
    g_objInfoList.LockHashTable(EntryType_Interface);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        g_objInfoList.RemoveInterfaceInfo(mDesc->mIID);
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_Interface);
    assert(ref >= 0);
    return ref;
}

PInterface CInterfaceInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IInterfaceInfo) {
        return (IInterfaceInfo *)this;
    }
    else if (riid == EIID_IDataTypeInfo) {
        return (IDataTypeInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CInterfaceInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CInterfaceInfo::Init()
{
    return CreateIFList();
}

ECode CInterfaceInfo::GetSize(
    /* [out] */ MemorySize* size)
{
    if (!size) {
        return E_INVALID_ARGUMENT;
    }

    *size = sizeof(IInterface *);
    return NOERROR;
}

ECode CInterfaceInfo::GetDataType(
    /* [out] */ CarDataType* dataType)
{
    if (!dataType) {
        return E_INVALID_ARGUMENT;
    }

    *dataType = CarDataType_Interface;
    return NOERROR;
}

ECode CInterfaceInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = adjustNameAddr(mBase, mInterfaceDirEntry->mName);
    return NOERROR;
}

ECode CInterfaceInfo::GetNamespace(
    /* [out] */ String* ns)
{
    if (ns == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *ns = adjustNameAddr(mBase, mInterfaceDirEntry->mNameSpace);
    return NOERROR;
}

ECode CInterfaceInfo::GetId(
    /* [out] */ InterfaceID* id)
{
    if (!id) {
        return E_INVALID_ARGUMENT;
    }

    *id = mDesc->mIID;
    return NOERROR;
}

ECode CInterfaceInfo::GetModuleInfo(
    /* [out] */ IModuleInfo** moduleInfo)
{
    return mClsModule->GetModuleInfo(moduleInfo);
}

ECode CInterfaceInfo::IsLocal(
    /* [out] */ Boolean* local)
{
    if (!local) {
        return E_INVALID_ARGUMENT;
    }

    if (mDesc->mAttribs & InterfaceAttrib_local) {
        *local = TRUE;
    }
    else {
        *local = FALSE;
    }

    return NOERROR;
}

ECode CInterfaceInfo::HasBase(
    /* [out] */ Boolean* hasBase)
{
    if (!hasBase) {
        return E_INVALID_ARGUMENT;
    }

    if (mIndex != mDesc->mParentIndex) {
        *hasBase = TRUE;
    }
    else {
        *hasBase = FALSE;
    }

    return NOERROR;
}

ECode CInterfaceInfo::GetBaseInfo(
    /* [out] */ IInterfaceInfo** baseInfo)
{
    if (!baseInfo) {
        return E_INVALID_ARGUMENT;
    }

    *baseInfo = NULL;
    UInt32 index = mDesc->mParentIndex;
    return g_objInfoList.AcquireInterfaceInfo(mClsModule, index,
            (IInterface**)baseInfo);
}

ECode CInterfaceInfo::GetMethodCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    if (!mIFList) {
        ECode ec = CreateIFList();
        if (FAILED(ec)) return ec;
    }

    *count = 0;
    for (UInt32 i = 0; i < mIFCount; i++) {
        *count += mIFList[i].mDesc->mMethodCount;
    }

    return NOERROR;
}

ECode CInterfaceInfo::AcquireMethodList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_Method);
    if (!mMethodList) {
        mMethodList = new CEntryList(EntryType_Method,
            mDesc, mMethodCount, mClsModule, mIFList, mIFCount);
        if (!mMethodList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_Method);

    return ec;

    return NOERROR;
}

ECode CInterfaceInfo::GetAllMethodInfos(
    /* [out] */ ArrayOf<IMethodInfo *>* methodInfos)
{
    ECode ec = AcquireMethodList();
    if (FAILED(ec)) return ec;

    return mMethodList->GetAllObjInfos((PTypeInfos)methodInfos);
}

ECode CInterfaceInfo::GetMethodInfo(
    /* [in] */ const String& name,
    /* [in] */ const String& signature,
    /* [out] */ IMethodInfo** methodInfo)
{
    if (name.IsNull() || signature.IsNull() || !methodInfo) {
        return E_INVALID_ARGUMENT;
    }

    assert(mMethodCount);

    ECode ec = AcquireMethodList();
    if (FAILED(ec)) return ec;

    String strName = name + signature;
    return mMethodList->AcquireObjByName(strName, (IInterface **)methodInfo);
}

ECode CInterfaceInfo::CreateIFList()
{
    if (mIFList) {
        return NOERROR;
    }

    UInt32* indexList = (UInt32 *)alloca(
            mClsMod->mInterfaceCount * sizeof(UInt32));
    if (indexList == NULL) {
        return E_OUT_OF_MEMORY;
    }

    Int32 i, j = 0;
    mIFCount = 0;
    UInt32 index = mIndex;
    InterfaceDirEntry* ifDir = NULL;
    while (index != 0) {
        indexList[mIFCount++] = index;
        ifDir = getInterfaceDirAddr(mBase, mClsMod->mInterfaceDirs, index);
        index = adjustInterfaceDescAddr(mBase, ifDir->mDesc)->mParentIndex;
    }

    indexList[mIFCount] = 0;
    mIFCount++;

    UInt32 beginNo = METHOD_START_NO;
    mIFList = new IFIndexEntry[mIFCount];
    if (mIFList == NULL) {
        return E_OUT_OF_MEMORY;
    }

    for (i = mIFCount - 1, j = 0; i >= 0; i--, j++) {
        index = indexList[i];
        mIFList[j].mIndex = index;
        mIFList[j].mBeginNo = beginNo;
        ifDir = getInterfaceDirAddr(mBase, mClsMod->mInterfaceDirs, index);
        mIFList[j].mName = adjustNameAddr(mBase, ifDir->mName);
        mIFList[j].mNameSpace = adjustNameAddr(mBase, ifDir->mNameSpace);
        mIFList[j].mDesc = adjustInterfaceDescAddr(mBase, ifDir->mDesc);
        beginNo += mIFList[j].mDesc->mMethodCount;
    }

    mMethodCount = 0;
    for (i = 0; i < (int)mIFCount; i++) {
        mMethodCount += mIFList[i].mDesc->mMethodCount;
    }

    return NOERROR;
}
