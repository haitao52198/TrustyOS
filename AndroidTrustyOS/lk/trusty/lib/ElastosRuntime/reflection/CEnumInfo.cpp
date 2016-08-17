//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CEnumInfo.h"
#include "CEnumItemInfo.h"

CEnumInfo::CEnumInfo()
    : mEnumDirEntry(NULL)
    , mItemInfos(NULL)
{}

CEnumInfo::~CEnumInfo()
{
    if (mItemInfos) {
        for (Int32 i = 0; i < mItemInfos->GetLength(); i++) {
            if ((*mItemInfos)[i]) {
                delete (CEnumItemInfo*)(*mItemInfos)[i];
                (*mItemInfos)[i] = NULL;
            }
        }
        ArrayOf<IEnumItemInfo *>::Free(mItemInfos);
    }
}

UInt32 CEnumInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CEnumInfo::Release()
{
    g_objInfoList.LockHashTable(EntryType_Enum);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        if (!mClsModule) {
            g_objInfoList.DetachEnumInfo(this);
        }
        else {
            g_objInfoList.RemoveEnumInfo(mEnumDirEntry);
        }
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_Enum);
    assert(ref >= 0);
    return ref;
}

PInterface CEnumInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IEnumInfo) {
        return (IEnumInfo *)this;
    }
    else if (riid == EIID_IDataTypeInfo) {
        return (IDataTypeInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CEnumInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CEnumInfo::InitStatic(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ EnumDirEntry* enumDirEntry)
{
    mClsModule = clsModule;

    Int32 base = mClsModule->mBase;
    mEnumDirEntry = enumDirEntry;
    mName = adjustNameAddr(base, mEnumDirEntry->mName);
    mNamespace = adjustNameAddr(base, mEnumDirEntry->mNameSpace);
    EnumDescriptor* desc = adjustEnumDescAddr(base, enumDirEntry->mDesc);

    mItemValues = ArrayOf<Int32>::Alloc(desc->mElementCount);
    if (!mItemValues) {
        return E_OUT_OF_MEMORY;
    }

    mItemNames = ArrayOf<String>::Alloc(desc->mElementCount);
    if (!mItemNames) {
        mItemValues = NULL;
        return E_OUT_OF_MEMORY;
    }

    EnumElement* elem = NULL;
    for (Int32 i = 0; i < desc->mElementCount; i++) {
        elem = getEnumElementAddr(base, desc->mElements, i);
        (*mItemValues)[i] = elem->mValue;
        (*mItemNames)[i] = adjustNameAddr(base, elem->mName);
    }

    ECode ec = InitItemInfos();
    if (FAILED(ec)) {
        mItemNames = NULL;
        mItemValues = NULL;
    }

    return NOERROR;
}

ECode CEnumInfo::InitDynamic(
    /* [in] */ const String& fullName,
    /* [in] */ ArrayOf<String>* itemNames,
    /* [in] */ ArrayOf<Int32>* itemValues)
{
    if (itemNames == NULL || itemValues == NULL) {
        return E_INVALID_ARGUMENT;
    }

    Int32 index = fullName.LastIndexOf('.');
    mName = index > 0 ? fullName.Substring(index + 1) : fullName;
    mNamespace = index > 0 ? fullName.Substring(0, index - 1) : String("");
    mItemNames = itemNames;
    mItemValues = itemValues;

    ECode ec = InitItemInfos();
    if (FAILED(ec)) {
        mItemNames = NULL;
        mItemValues = NULL;
        return ec;
    }

    return NOERROR;
}

ECode CEnumInfo::GetSize(
    /* [out] */ MemorySize* size)
{
    if (!size) {
        return E_INVALID_ARGUMENT;
    }

    *size = sizeof(Int32);
    return NOERROR;
}

ECode CEnumInfo::GetDataType(
    /* [out] */ CarDataType* dataType)
{
    if (!dataType) {
        return E_INVALID_ARGUMENT;
    }

    *dataType = CarDataType_Enum;
    return NOERROR;
}

ECode CEnumInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = mName;
    return NOERROR;
}

ECode CEnumInfo::GetNamespace(
    /* [out] */ String* ns)
{
    if (ns == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *ns = mNamespace;
    return NOERROR;
}

ECode CEnumInfo::GetModuleInfo(
    /* [out] */ IModuleInfo** moduleInfo)
{
    if (mClsModule) {
        return mClsModule->GetModuleInfo(moduleInfo);
    }
    else {
        return E_INVALID_OPERATION;
    }
}

ECode CEnumInfo::GetItemCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mItemValues->GetLength();
    return NOERROR;
}

ECode CEnumInfo::InitItemInfos()
{
    Int32 count = mItemValues->GetLength();
    mItemInfos = ArrayOf<IEnumItemInfo *>::Alloc(count);
    if (mItemInfos == NULL) {
        return E_OUT_OF_MEMORY;
    }

    for (Int32 i = 0; i < count; i++) {
        // do not use mItemInfos->Set(i, xxx), because it will
        // call xxx->AddRef();
        (*mItemInfos)[i] = new CEnumItemInfo(this,
                (*mItemNames)[i], (*mItemValues)[i]);
        if (!(*mItemInfos)[i]) goto EExit;
    }

    return NOERROR;

EExit:
    for (Int32 i = 0; i < count; i++) {
        if ((*mItemInfos)[i]) {
            delete (CEnumItemInfo*)(*mItemInfos)[i];
            (*mItemInfos)[i] = NULL;
        }
    }
    ArrayOf<IEnumItemInfo *>::Free(mItemInfos);
    mItemInfos = NULL;

    return NOERROR;
}

ECode CEnumInfo::GetAllItemInfos(
    /* [out] */ ArrayOf<IEnumItemInfo *>* itemInfos)
{
    if (!itemInfos) {
        return E_INVALID_ARGUMENT;
    }

    itemInfos->Copy(mItemInfos);

    return NOERROR;
}

ECode CEnumInfo::GetItemInfo(
    /* [in] */ const String& name,
    /* [out] */ IEnumItemInfo** enumItemInfo)
{
    if (name.IsNull() || !enumItemInfo) {
        return E_INVALID_ARGUMENT;
    }

    for (Int32 i = 0; i < mItemInfos->GetLength(); i++) {
        if (name.Equals((*mItemNames)[i].string())) {
            *enumItemInfo = (*mItemInfos)[i];
            (*enumItemInfo)->AddRef();
            return NOERROR;
        }
    }

    return E_DOES_NOT_EXIST;
}
