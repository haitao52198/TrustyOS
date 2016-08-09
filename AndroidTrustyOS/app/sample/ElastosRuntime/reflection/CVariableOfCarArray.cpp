//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CVariableOfCarArray.h"
#include "CStructInfo.h"
#include "CObjInfoList.h"

CVariableOfCarArray::CVariableOfCarArray(
    /* [in] */ ICarArrayInfo* typeInfo,
    /* [in] */ PCarQuintet cq,
    /* [in] */ Boolean isAlloc)
{
    mCarArrayInfo = typeInfo;

    mCq = cq;
    mIsAlloc = isAlloc;
    mElementSize = 0;
    mLength = 0;

    AutoPtr<IDataTypeInfo> elementTypeInfo;
    mCarArrayInfo->GetElementTypeInfo((IDataTypeInfo**)&elementTypeInfo);
    assert(elementTypeInfo);
    elementTypeInfo->GetDataType(&mDataType);
    elementTypeInfo->GetSize(&mElementSize);
    assert(mElementSize);
    mLength = cq->mSize / mElementSize;
}

CVariableOfCarArray::~CVariableOfCarArray()
{
    if (mIsAlloc && mCq) {
        free(mCq);
    }
}

UInt32 CVariableOfCarArray::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CVariableOfCarArray::Release()
{
    return ElLightRefBase::Release();
}

PInterface CVariableOfCarArray::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)(IVariableOfCarArray *)this;
    }
    else if (riid == EIID_IVariableOfCarArray) {
        return (IVariableOfCarArray *)this;
    }
    else if (riid == EIID_ICarArraySetter) {
        return (ICarArraySetter *)this;
    }
    else if (riid == EIID_ICarArrayGetter) {
        return (ICarArrayGetter *)this;
    }
    else {
        return NULL;
    }
}

ECode CVariableOfCarArray::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CVariableOfCarArray::GetTypeInfo(
    /* [out] */ IDataTypeInfo** typeInfo)
{
    if (!typeInfo) {
        return E_INVALID_ARGUMENT;
    }

    *typeInfo = mCarArrayInfo;
    REFCOUNT_ADD(*typeInfo);
    return NOERROR;
}

ECode CVariableOfCarArray::GetPayload(
    /* [out] */ PVoid* payload)
{
    if (!payload) {
        return E_INVALID_ARGUMENT;
    }

    *payload = mCq;
    return NOERROR;
}

ECode CVariableOfCarArray::Rebox(
    /* [in] */ PVoid localVariablePtr)
{
    CarQuintetFlag flag = DataTypeToFlag(mDataType);
    PCarQuintet cq = (PCarQuintet)localVariablePtr;
    if (!cq || !(cq->mFlags & flag) || cq->mSize < mElementSize) {
        return E_INVALID_ARGUMENT;
    }

    if (mIsAlloc && mCq) {
        free(mCq);
        mIsAlloc = FALSE;
    }

    mCq = cq;

    return NOERROR;
}

ECode CVariableOfCarArray::GetSetter(
    /* [out] */ ICarArraySetter** setter)
{
    if (!setter) {
        return E_INVALID_ARGUMENT;
    }

    *setter = (ICarArraySetter *)this;
    (*setter)->AddRef();
    return NOERROR;
}

ECode CVariableOfCarArray::GetGetter(
    /* [out] */ ICarArrayGetter** getter)
{
    if (!getter) {
        return E_INVALID_ARGUMENT;
    }

    *getter = (ICarArrayGetter *)this;
    (*getter)->AddRef();
    return NOERROR;
}

//Setter

ECode CVariableOfCarArray::SetUsed(
    /* [in] */ Int32 used)
{
    if (used < 0 || used > mLength) {
        return E_INVALID_ARGUMENT;
    }

    mCq->mUsed = used * mElementSize;

    return NOERROR;
}

ECode CVariableOfCarArray::SetElementValue(
    /* [in] */ Int32 index,
    /* [in] */ void* param,
    /* [in] */ CarDataType type)
{
    if (!param) {
        return E_INVALID_ARGUMENT;
    }

    if (index >= mLength
            || index >= (Int32)(mCq->mUsed / mElementSize)) {
        return E_INVALID_ARGUMENT;
    }

    if (mDataType != type || !mCq->mBuf) {
        return E_INVALID_OPERATION;
    }

    memcpy((PByte)mCq->mBuf + mElementSize * index,
            param, mElementSize);

    return NOERROR;
}

ECode CVariableOfCarArray::SetInt16Element(
    /* [in] */ Int32 index,
    /* [in] */ Int16 value)
{
    return SetElementValue(index, &value, CarDataType_Int16);
}

ECode CVariableOfCarArray::SetInt32Element(
    /* [in] */ Int32 index,
    /* [in] */ Int32 value)
{
    return SetElementValue(index, &value, CarDataType_Int32);
}

ECode CVariableOfCarArray::SetInt64Element(
    /* [in] */ Int32 index,
    /* [in] */ Int64 value)
{
    return SetElementValue(index, &value, CarDataType_Int64);
}

ECode CVariableOfCarArray::SetByteElement(
    /* [in] */ Int32 index,
    /* [in] */ Byte value)
{
    return SetElementValue(index, &value, CarDataType_Byte);
}

ECode CVariableOfCarArray::SetFloatElement(
    /* [in] */ Int32 index,
    /* [in] */ Float value)
{
    return SetElementValue(index, &value, CarDataType_Float);
}

ECode CVariableOfCarArray::SetDoubleElement(
    /* [in] */ Int32 index,
    /* [in] */ Double value)
{
    return SetElementValue(index, &value, CarDataType_Double);
}

ECode CVariableOfCarArray::SetEnumElement(
    /* [in] */ Int32 index,
    /* [in] */ Int32 value)
{
    return SetElementValue(index, &value, CarDataType_Enum);
}

ECode CVariableOfCarArray::SetCharElement(
    /* [in] */ Int32 index,
    /* [in] */ Char32 value)
{
    return SetElementValue(index, &value, CarDataType_Char32);
}

ECode CVariableOfCarArray::SetStringElement(
    /* [in] */ Int32 index,
    /* [in] */ const String& value)
{
    return SetElementValue(index, reinterpret_cast<void*>(const_cast<String*>(&value)), CarDataType_String);
}

ECode CVariableOfCarArray::SetBooleanElement(
    /* [in] */ Int32 index,
    /* [in] */ Boolean value)
{
    return SetElementValue(index, &value, CarDataType_Boolean);
}

ECode CVariableOfCarArray::SetEMuidElement(
    /* [in] */ Int32 index,
    /* [in] */ EMuid* value)
{
    return SetElementValue(index, &value, CarDataType_EMuid);
}

ECode CVariableOfCarArray::SetEGuidElement(
    /* [in] */ Int32 index,
    /* [in] */ EGuid* value)
{
    return SetElementValue(index, &value, CarDataType_EGuid);
}

ECode CVariableOfCarArray::SetECodeElement(
    /* [in] */ Int32 index,
    /* [in] */ ECode value)
{
    return SetElementValue(index, &value, CarDataType_ECode);
}

ECode CVariableOfCarArray::SetLocalTypeElement(
    /* [in] */ Int32 index,
    /* [in] */ PVoid value)
{
    return SetElementValue(index, value, CarDataType_LocalType);
}

ECode CVariableOfCarArray::SetObjectPtrElement(
    /* [in] */ Int32 index,
    /* [in] */ PInterface value)
{
    return SetElementValue(index, &value, CarDataType_Interface);
}

ECode CVariableOfCarArray::GetStructElementSetter(
    /* [in] */ Int32 index,
    /* [out] */ IStructSetter** setter)
{
    if (!setter || mDataType != CarDataType_Struct) {
        return E_INVALID_ARGUMENT;
    }

    AutoPtr<IDataTypeInfo> elementTypeInfo;
    ECode ec = mCarArrayInfo->GetElementTypeInfo((IDataTypeInfo**)&elementTypeInfo);
    if (FAILED(ec)) {
        return ec;
    }

    CStructInfo* structInfo = (CStructInfo *)elementTypeInfo.Get();

    AutoPtr<IVariableOfStruct> variable;
    ec = structInfo->CreateVariableBox(
            (PByte)mCq->mBuf + mElementSize * index, (IVariableOfStruct**)&variable);
    if (FAILED(ec)) {
        return ec;
    }

    return variable->GetSetter(setter);
}

//Getter

ECode CVariableOfCarArray::GetCapacity(
    /* [out] */ Int32* capacity)
{
    if (!capacity) {
        return E_INVALID_ARGUMENT;
    }

    *capacity = mLength;

    return NOERROR;
}

ECode CVariableOfCarArray::GetUsed(
    /* [out] */ Int32* used)
{
    if (!used) {
        return E_INVALID_ARGUMENT;
    }

    *used = mCq->mUsed / mElementSize;

    return NOERROR;
}

ECode CVariableOfCarArray::IsEmpty(
    /* [out] */ Boolean* isEmpty)
{
    if (!isEmpty) {
        return E_INVALID_ARGUMENT;
    }

    *isEmpty = (mCq->mBuf == NULL);

    return NOERROR;
}

ECode CVariableOfCarArray::GetElementValue(
    /* [in] */ Int32 index,
    /* [in] */ void* param,
    /* [in] */ CarDataType type)
{
    if (!param) {
        return E_INVALID_ARGUMENT;
    }

    if (index >= mLength
            || index >= (Int32)(mCq->mUsed / mElementSize)) {
        return E_INVALID_ARGUMENT;
    }

    if (mDataType != type || !mCq->mBuf) {
        return E_INVALID_OPERATION;
    }

    memcpy(param, (PByte)mCq->mBuf + mElementSize * index,
            mElementSize);

    return NOERROR;
}

ECode CVariableOfCarArray::GetInt16Element(
    /* [in] */ Int32 index,
    /* [out] */ Int16* value)
{
    return GetElementValue(index, value, CarDataType_Int16);
}

ECode CVariableOfCarArray::GetInt32Element(
    /* [in] */ Int32 index,
    /* [out] */ Int32* value)
{
    return GetElementValue(index, value, CarDataType_Int32);
}

ECode CVariableOfCarArray::GetInt64Element(
    /* [in] */ Int32 index,
    /* [out] */ Int64* value)
{
    return GetElementValue(index, value, CarDataType_Int64);
}

ECode CVariableOfCarArray::GetByteElement(
    /* [in] */ Int32 index,
    /* [out] */ Byte* value)
{
    return GetElementValue(index, value, CarDataType_Byte);
}

ECode CVariableOfCarArray::GetFloatElement(
    /* [in] */ Int32 index,
    /* [out] */ Float* value)
{
    return GetElementValue(index, value, CarDataType_Float);
}

ECode CVariableOfCarArray::GetDoubleElement(
    /* [in] */ Int32 index,
    /* [out] */ Double* value)
{
    return GetElementValue(index, value, CarDataType_Double);
}

ECode CVariableOfCarArray::GetEnumElement(
    /* [in] */ Int32 index,
    /* [out] */ Int32* value)
{
    return GetElementValue(index, value, CarDataType_Enum);
}

ECode CVariableOfCarArray::GetCharElement(
    /* [in] */ Int32 index,
    /* [out] */ Char32* value)
{
    return GetElementValue(index, value, CarDataType_Char32);
}

ECode CVariableOfCarArray::GetStringElement(
    /* [in] */ Int32 index,
    /* [out] */ String* value)
{
    return GetElementValue(index, value, CarDataType_String);
}

ECode CVariableOfCarArray::GetBooleanElement(
    /* [in] */ Int32 index,
    /* [out] */ Boolean* value)
{
    return GetElementValue(index, value, CarDataType_Boolean);
}

ECode CVariableOfCarArray::GetEMuidElement(
    /* [in] */ Int32 index,
    /* [out] */ EMuid* value)
{
    return GetElementValue(index, value, CarDataType_EMuid);
}

ECode CVariableOfCarArray::GetEGuidElement(
    /* [in] */ Int32 index,
    /* [out] */ EGuid* value)
{
    return GetElementValue(index, value, CarDataType_EGuid);
}

ECode CVariableOfCarArray::GetECodeElement(
    /* [in] */ Int32 index,
    /* [out] */ ECode* value)
{
    return GetElementValue(index, value, CarDataType_ECode);
}

ECode CVariableOfCarArray::GetLocalTypeElement(
    /* [in] */ Int32 index,
    /* [out] */ PVoid value)
{
    return GetElementValue(index, value, CarDataType_LocalType);
}

ECode CVariableOfCarArray::GetObjectPtrElement(
    /* [in] */ Int32 index,
    /* [out] */ PInterface* value)
{
    return GetElementValue(index, value, CarDataType_Interface);
}

ECode CVariableOfCarArray::GetStructElementGetter(
    /* [in] */ Int32 index,
    /* [out] */ IStructGetter** getter)
{
    if (!getter || mDataType != CarDataType_Struct) {
        return E_INVALID_ARGUMENT;
    }

    AutoPtr<IDataTypeInfo> elementTypeInfo;
    ECode ec = mCarArrayInfo->GetElementTypeInfo((IDataTypeInfo**)&elementTypeInfo);
    if (FAILED(ec)) {
        return ec;
    }

    CStructInfo* structInfo = (CStructInfo *)elementTypeInfo.Get();

    AutoPtr<IVariableOfStruct> variable;
    ec = structInfo->CreateVariableBox(
            (PByte)mCq->mBuf + mElementSize * index, (IVariableOfStruct**)&variable);
    if (FAILED(ec)) {
        return ec;
    }

    return variable->GetGetter(getter);
}
