//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CLocalPtrInfo.h"
#include "CObjInfoList.h"

CLocalPtrInfo::CLocalPtrInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ TypeDescriptor* typeDescriptor,
    /* [in] */ Int32 pointer)
{
    mClsModule = clsModule;
    mTypeDescriptor = typeDescriptor;
    mPointer = pointer;
}

UInt32 CLocalPtrInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CLocalPtrInfo::Release()
{
    g_objInfoList.LockHashTable(EntryType_Local);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        g_objInfoList.RemoveLocalPtrInfo(mTypeDescriptor, mPointer);
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_Local);
    assert(ref >= 0);
    return ref;
}

PInterface CLocalPtrInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_ILocalPtrInfo) {
        return (ILocalPtrInfo *)this;
    }
    else if (riid == EIID_IDataTypeInfo) {
        return (IDataTypeInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CLocalPtrInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CLocalPtrInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = g_cDataTypeList[CarDataType_LocalPtr].mName;
    return NOERROR;
}

ECode CLocalPtrInfo::GetSize(
    /* [out] */ MemorySize* size)
{
    if (!size) {
        return E_INVALID_ARGUMENT;
    }

    *size = sizeof(PVoid);

    return NOERROR;
}

ECode CLocalPtrInfo::GetDataType(
    /* [out] */ CarDataType* dataType)
{
    if (!dataType) {
        return E_INVALID_ARGUMENT;
    }

    *dataType = CarDataType_LocalPtr;

    return NOERROR;
}

ECode CLocalPtrInfo::GetTargetTypeInfo(
    /* [out] */ IDataTypeInfo** dateTypeInfo)
{
    return g_objInfoList.AcquireDataTypeInfo(mClsModule,
            mTypeDescriptor, dateTypeInfo);
}

ECode CLocalPtrInfo::GetPtrLevel(
    /* [out] */ Int32* level)
{
    if (!level) {
        return E_INVALID_ARGUMENT;
    }

    *level = mPointer;

    return NOERROR;
}
