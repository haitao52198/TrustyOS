//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CArgumentList.h"
#include "CMethodInfo.h"
#include "CConstructorInfo.h"

CArgumentList::CArgumentList()
    : mParamBuf(NULL)
    , mParamElem(NULL)
    , mParamCount(0)
    , mParamBufSize(0)
    , mIsMethodInfo(FALSE)
{}

CArgumentList::~CArgumentList()
{
    if (mParamBuf) free(mParamBuf);
}

UInt32 CArgumentList::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CArgumentList::Release()
{
    return ElLightRefBase::Release();
}

PInterface CArgumentList::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IArgumentList) {
        return (IArgumentList *)this;
    }
    else {
        return NULL;
    }
}

ECode CArgumentList::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CArgumentList::Init(
    /* [in] */ IFunctionInfo* functionInfo,
    /* [in] */ ParmElement* paramElem,
    /* [in] */ UInt32 paramCount,
    /* [in] */ UInt32 paramBufSize,
    /* [in] */ Boolean isMethodInfo)
{
    mParamElem = paramElem;
    mParamCount = paramCount;

    mParamBuf = (PByte)malloc(paramBufSize);
    if (mParamBuf == NULL) {
        return E_OUT_OF_MEMORY;
    }

    mParamBufSize = paramBufSize;
    memset(mParamBuf, 0, paramBufSize);
    mFunctionInfo = functionInfo;

    mIsMethodInfo = isMethodInfo;

    return NOERROR;
}

ECode CArgumentList::GetFunctionInfo(
    /* [out] */ IFunctionInfo** functionInfo)
{
    if (!functionInfo) {
        return E_INVALID_ARGUMENT;
    }
    *functionInfo = mFunctionInfo;
    (*functionInfo)->AddRef();
    return NOERROR;
}

ECode CArgumentList::SetParamValue(
    /* [in] */ Int32 index,
    /* [in] */ PVoid param,
    /* [in] */ CarDataType type,
    /* [in] */ ParamIOAttribute attrib,
    /* [in] */ Int32 pointer)
{
    if (type == CarDataType_CarArray
        && mParamElem[index].mType == CarDataType_ArrayOf) {
        type = mParamElem[index].mType;
    }

    if (index < 0 || index >= (Int32)mParamCount
        || (mParamElem[index].mAttrib != attrib)
        || (type != mParamElem[index].mType)
        || (type != CarDataType_LocalPtr
        && mParamElem[index].mPointer != pointer)) {
        return E_INVALID_ARGUMENT;
    }

    if (!mParamBuf || mParamElem[index].mPos
        + mParamElem[index].mSize > mParamBufSize) {
        return E_INVALID_OPERATION;
    }

    if (mParamElem[index].mSize == 1) {
        *(Byte *)(mParamBuf + mParamElem[index].mPos) = *(Byte *)param;
    }
    else if (mParamElem[index].mSize == 2) {
        *(UInt16 *)(mParamBuf + mParamElem[index].mPos) = *(UInt16 *)param;
    }
    else if (mParamElem[index].mSize == 4) {
        if (type == CarDataType_String) {
            *(String **)(mParamBuf + mParamElem[index].mPos) = (String *)param;
        }
        else {
            *(UInt32 *)(mParamBuf + mParamElem[index].mPos) = *(UInt32 *)param;
        }
    }
    else if (mParamElem[index].mSize == 8) {
        *(UInt64 *)(mParamBuf + mParamElem[index].mPos) = *(UInt64 *)param;
    }
    else {
        return E_INVALID_OPERATION;
    }

    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfInt16(
    /* [in] */ Int32 index,
    /* [in] */ Int16 value)
{
    UInt32 nParam = value;
    return SetParamValue(index, &nParam, CarDataType_Int16, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfInt32(
    /* [in] */ Int32 index,
    /* [in] */ Int32 value)
{
    return SetParamValue(index, &value, CarDataType_Int32, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfInt64(
    /* [in] */ Int32 index,
    /* [in] */ Int64 value)
{
    return SetParamValue(index, &value, CarDataType_Int64, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfByte(
    /* [in] */ Int32 index,
    /* [in] */ Byte value)
{
    UInt32 nParam = value;
    return SetParamValue(index, &nParam, CarDataType_Byte, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfFloat(
    /* [in] */ Int32 index,
    /* [in] */ Float value)
{
    return SetParamValue(index, &value, CarDataType_Float, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfDouble(
    /* [in] */ Int32 index,
    /* [in] */ Double value)
{
    return SetParamValue(index, &value, CarDataType_Double, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfChar(
    /* [in] */ Int32 index,
    /* [in] */ Char32 value)
{
    UInt32 nParam = value;
    return SetParamValue(index, &nParam, CarDataType_Char32, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfString(
    /* [in] */ Int32 index,
    /* [in] */ const String& value)
{
    return SetParamValue(index, reinterpret_cast<PVoid>(const_cast<String*>(&value)),
            CarDataType_String, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfBoolean(
    /* [in] */ Int32 index,
    /* [in] */ Boolean value)
{
    UInt32 nParam = value;
    return SetParamValue(index, &nParam, CarDataType_Boolean, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfEMuid(
    /* [in] */ Int32 index,
    /* [in] */ EMuid* value)
{
    return SetParamValue(index, &value, CarDataType_EMuid, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfEGuid(
    /* [in] */ Int32 index,
    /* [in] */ EGuid* value)
{
    return SetParamValue(index, &value, CarDataType_EGuid, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfECode(
    /* [in] */ Int32 index,
    /* [in] */ ECode value)
{
    return SetParamValue(index, &value, CarDataType_ECode, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfLocalPtr(
    /* [in] */ Int32 index,
    /* [in] */ LocalPtr value)
{
    return SetParamValue(index, &value, CarDataType_LocalPtr,
            ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfLocalType(
    /* [in] */ Int32 index,
    /* [in] */ PVoid value)
{
    return SetParamValue(index, value, CarDataType_LocalType,
            ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfEnum(
    /* [in] */ Int32 index,
    /* [in] */ Int32 value)
{
    return SetParamValue(index, &value, CarDataType_Enum, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfCarArray(
    /* [in] */ Int32 index,
    /* [in] */ PCarQuintet value)
{
    return SetParamValue(index, &value, CarDataType_CarArray,
            ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfStructPtr(
    /* [in] */ Int32 index,
    /* [in] */ PVoid value)
{
    return SetParamValue(index, &value, CarDataType_Struct, ParamIOAttribute_In);
}

ECode CArgumentList::SetInputArgumentOfObjectPtr(
    /* [in] */ Int32 index,
    /* [in] */ PInterface value)
{
    if (CarDataType_Interface != mParamElem[index].mType) {
        return E_INVALID_ARGUMENT;
    }

    if (value) {
        CMethodInfo* methodInfo = NULL;
        ECode ec = NOERROR;
        if (mIsMethodInfo) {
            methodInfo = (CMethodInfo *)mFunctionInfo.Get();
        }
        else {
            methodInfo = ((CConstructorInfo *)mFunctionInfo.Get())->mMethodInfo;
        }

        Int32 base = methodInfo->mClsModule->mBase;
        TypeDescriptor* typeDesc = &(getParamDescAddr(base,
                methodInfo->mMethodDescriptor->mParams, index)->mType);
        if (typeDesc->mType == Type_alias) {
            ec = methodInfo->mClsModule->AliasToOriginal(typeDesc, &typeDesc);
            if (FAILED(ec)) return ec;
        }

        InterfaceDirEntry* ifDir = getInterfaceDirAddr(base,
                methodInfo->mClsMod->mInterfaceDirs, typeDesc->mIndex);
        EIID iid = adjustInterfaceDescAddr(base, ifDir->mDesc)->mIID;
        value = value->Probe(iid);
        if (!value) return E_NO_INTERFACE;
    }

    return SetParamValue(index, &value, CarDataType_Interface,
            ParamIOAttribute_In, 1);
}

ECode CArgumentList::SetOutputArgumentOfInt16Ptr(
    /* [in] */ Int32 index,
    /* [out] */ Int16* value)
{
    return SetParamValue(index, &value, CarDataType_Int16,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfInt32Ptr(
    /* [in] */ Int32 index,
    /* [out] */ Int32* value)
{
    return SetParamValue(index, &value, CarDataType_Int32,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfInt64Ptr(
    /* [in] */ Int32 index,
    /* [out] */ Int64* value)
{
    return SetParamValue(index, &value, CarDataType_Int64,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfBytePtr(
    /* [in] */ Int32 index,
    /* [out] */ Byte* value)
{
    return SetParamValue(index, &value, CarDataType_Byte,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfFloatPtr(
    /* [in] */ Int32 index,
    /* [out] */ Float* value)
{
    return SetParamValue(index, &value, CarDataType_Float,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfDoublePtr(
    /* [in] */ Int32 index,
    /* [out] */ Double* value)
{
    return SetParamValue(index, &value, CarDataType_Double,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfCharPtr(
    /* [in] */ Int32 index,
    /* [out] */ Char32* value)
{
    return SetParamValue(index, &value, CarDataType_Char32,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfStringPtr(
    /* [in] */ Int32 index,
    /* [out] */ String* value)
{
    return SetParamValue(index, value, CarDataType_String,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfBooleanPtr(
    /* [in] */ Int32 index,
    /* [out] */ Boolean* value)
{
    return SetParamValue(index, &value, CarDataType_Boolean,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfEMuidPtr(
    /* [in] */ Int32 index,
    /* [out] */ EMuid* value)
{
    return SetParamValue(index, &value, CarDataType_EMuid,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfEGuidPtr(
    /* [in] */ Int32 index,
    /* [out] */ EGuid* value)
{
    return SetParamValue(index, &value, CarDataType_EGuid,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfECodePtr(
    /* [in] */ Int32 index,
    /* [out] */ ECode* value)
{
    return SetParamValue(index, &value, CarDataType_ECode,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfLocalPtrPtr(
    /* [in] */ Int32 index,
    /* [out] */ LocalPtr* value)
{
    return SetParamValue(index, &value, CarDataType_LocalPtr,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfLocalTypePtr(
    /* [in] */ Int32 index,
    /* [out] */ PVoid value)
{
    return SetParamValue(index, &value, CarDataType_LocalType,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfEnumPtr(
    /* [in] */ Int32 index,
    /* [out] */ Int32* value)
{
    return SetParamValue(index, &value, CarDataType_Enum,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfCarArrayPtr(
    /* [in] */ Int32 index,
    /* [out] */ PCarQuintet value)
{
    return SetParamValue(index, &value, CarDataType_CarArray,
            ParamIOAttribute_CallerAllocOut, 0);
}

ECode CArgumentList::SetOutputArgumentOfCarArrayPtrPtr(
    /* [in] */ Int32 index,
    /* [out] */ PCarQuintet* value)
{
    return SetParamValue(index, &value, CarDataType_CarArray,
            ParamIOAttribute_CalleeAllocOut, 2);
}

ECode CArgumentList::SetOutputArgumentOfStructPtr(
    /* [in] */ Int32 index,
    /* [out] */ PVoid value)
{
    return SetParamValue(index, &value, CarDataType_Struct,
            ParamIOAttribute_CallerAllocOut, 1);
}

ECode CArgumentList::SetOutputArgumentOfStructPtrPtr(
    /* [in] */ Int32 index,
    /* [out] */ PVoid* value)
{
    return SetParamValue(index, &value, CarDataType_Struct,
            ParamIOAttribute_CalleeAllocOut, 2);
}

ECode CArgumentList::SetOutputArgumentOfObjectPtrPtr(
    /* [in] */ Int32 index,
    /* [out] */ PInterface* value)
{
    return SetParamValue(index, &value, CarDataType_Interface,
            ParamIOAttribute_CallerAllocOut, 2);
}
