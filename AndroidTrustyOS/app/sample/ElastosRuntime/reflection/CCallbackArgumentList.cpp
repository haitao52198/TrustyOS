//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CCallbackArgumentList.h"

CCallbackArgumentList::CCallbackArgumentList()
    : mParamBuf(NULL)
    , mParamBufSize(0)
    , mParamElem(NULL)
    , mParamCount(0)
{}

CCallbackArgumentList::~CCallbackArgumentList()
{
    if (mParamBuf) free(mParamBuf);
}

UInt32 CCallbackArgumentList::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CCallbackArgumentList::Release()
{
    return ElLightRefBase::Release();
}

PInterface  CCallbackArgumentList::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_ICallbackArgumentList) {
        return (ICallbackArgumentList *)this;
    }
    else {
        return NULL;
    }
}

ECode CCallbackArgumentList::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CCallbackArgumentList::Init(
    /* [in] */ ICallbackMethodInfo* callbackMethodInfo,
    /* [in] */ ParmElement* paramElem,
    /* [in] */ UInt32 paramCount,
    /* [in] */ UInt32 paramBufSize)
{
    mParamBuf = (PByte)malloc(paramBufSize);
    if (mParamBuf == NULL) {
        return E_OUT_OF_MEMORY;
    }

    mParamElem = paramElem;
    mParamCount = paramCount;
    memset(mParamBuf, 0, paramBufSize);
    mParamBufSize = paramBufSize;
    mCallbackMethodInfo = callbackMethodInfo;

    return NOERROR;
}

ECode CCallbackArgumentList::GetCallbackMethodInfo(
    /* [out] */ ICallbackMethodInfo** callbackMethodInfo)
{
    if (!callbackMethodInfo) {
        return E_INVALID_ARGUMENT;
    }
    *callbackMethodInfo = mCallbackMethodInfo;
    (*callbackMethodInfo)->AddRef();

    return NOERROR;
}

ECode CCallbackArgumentList::GetParamValue(
    /* [in] */ Int32 index,
    /* [in] */ PVoid param,
    /* [in] */ CarDataType type)
{
    if (type == CarDataType_CarArray
        && mParamElem[index].mType == CarDataType_ArrayOf) {
        type = mParamElem[index].mType;
    }

    if (index < 0 || index >= (Int32)mParamCount || !param
        || type != mParamElem[index].mType) {
        return E_INVALID_ARGUMENT;
    }

    if (!mParamBuf) {
        return E_INVALID_OPERATION;
    }

    UInt32* paramValue = (UInt32 *)(mParamBuf + mParamElem[index].mPos);

    if (mParamElem[index].mSize == 1) {
        *(Byte *)param = (Byte)*paramValue;
    }
    else if (mParamElem[index].mSize == 2) {
        *(UInt16 *)param = (UInt16)*paramValue;
    }
    else if (mParamElem[index].mSize == 4) {
        *(UInt32 *)param = (UInt32)*paramValue;
    }
    else if (mParamElem[index].mSize == 8) {
        *(UInt64 *)param = *(UInt64 *)paramValue;
    }
    else {
        return E_INVALID_OPERATION;
    }

    return NOERROR;
}

ECode CCallbackArgumentList::GetServerPtrArgument(
    /* [out] */ PInterface* server)
{
    if (!server) {
        return E_INVALID_ARGUMENT;
    }

    *server = *(PInterface *)mParamBuf;
    return NOERROR;
}

ECode CCallbackArgumentList::GetInt16Argument(
    /* [in] */ Int32 index,
    /* [out] */ Int16* value)
{
    return GetParamValue(index, value, CarDataType_Int16);
}

ECode CCallbackArgumentList::GetInt32Argument(
    /* [in] */ Int32 index,
    /* [out] */ Int32* value)
{
    return GetParamValue(index, value, CarDataType_Int32);
}

ECode CCallbackArgumentList::GetInt64Argument(
    /* [in] */ Int32 index,
    /* [out] */ Int64* value)
{
    return GetParamValue(index, value, CarDataType_Int64);
}

ECode CCallbackArgumentList::GetByteArgument(
    /* [in] */ Int32 index,
    /* [out] */ Byte* value)
{
    return GetParamValue(index, value, CarDataType_Byte);
}

ECode CCallbackArgumentList::GetFloatArgument(
    /* [in] */ Int32 index,
    /* [out] */ Float* value)
{
    return GetParamValue(index, value, CarDataType_Float);
}

ECode CCallbackArgumentList::GetDoubleArgument(
    /* [in] */ Int32 index,
    /* [out] */ Double* value)
{
    return GetParamValue(index, value, CarDataType_Double);
}

ECode CCallbackArgumentList::GetCharArgument(
    /* [in] */ Int32 index,
    /* [out] */ Char32* value)
{
    return GetParamValue(index, value, CarDataType_Char32);
}

ECode CCallbackArgumentList::GetStringArgument(
    /* [in] */ Int32 index,
    /* [out] */ String* value)
{
    return GetParamValue(index, value, CarDataType_String);
}

ECode CCallbackArgumentList::GetBooleanArgument(
    /* [in] */ Int32 index,
    /* [out] */ Boolean* value)
{
    return GetParamValue(index, value, CarDataType_Boolean);
}

ECode CCallbackArgumentList::GetEMuidArgument(
    /* [in] */ Int32 index,
    /* [out] */ EMuid** value)
{
    return GetParamValue(index, value, CarDataType_EMuid);
}

ECode CCallbackArgumentList::GetEGuidArgument(
    /* [in] */ Int32 index,
    /* [out] */ EGuid** value)
{
    return GetParamValue(index, value, CarDataType_EGuid);
}

ECode CCallbackArgumentList::GetECodeArgument(
    /* [in] */ Int32 index,
    /* [out] */ ECode* value)
{
    return GetParamValue(index, value, CarDataType_ECode);
}

ECode CCallbackArgumentList::GetLocalPtrArgument(
    /* [in] */ Int32 index,
    /* [out] */ LocalPtr* value)
{
    return GetParamValue(index, value, CarDataType_LocalPtr);
}

ECode CCallbackArgumentList::GetEnumArgument(
    /* [in] */ Int32 index,
    /* [out] */ Int32* value)
{
    return GetParamValue(index, value, CarDataType_Enum);
}

ECode CCallbackArgumentList::GetCarArrayArgument(
    /* [in] */ Int32 index,
    /* [out] */ PCarQuintet* value)
{
    return GetParamValue(index, value, CarDataType_CarArray);
}

ECode CCallbackArgumentList::GetStructPtrArgument(
    /* [in] */ Int32 index,
    /* [out] */ PVoid* value)
{
    return GetParamValue(index, value, CarDataType_Struct);
}

ECode CCallbackArgumentList::GetObjectPtrArgument(
    /* [in] */ Int32 index,
    /* [out] */ PInterface* value)
{
    return GetParamValue(index, value, CarDataType_Interface);
}
