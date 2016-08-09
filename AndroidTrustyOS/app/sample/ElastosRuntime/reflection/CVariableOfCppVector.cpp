//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CVariableOfCppVector.h"
#include "CStructInfo.h"
#include "CObjInfoList.h"

CVariableOfCppVector::CVariableOfCppVector(
    /* [in] */ IDataTypeInfo* elementTypeInfo,
    /* [in] */ Int32 length,
    /* [in] */ PVoid varPtr)
{
    mElementTypeInfo = elementTypeInfo;

    mVarPtr = varPtr;
    mElementSize = 0;
    mLength = length;
    mElementTypeInfo->GetDataType(&mDataType);
    mElementTypeInfo->GetSize(&mElementSize);
    mCppVectorSGetters = NULL;

    mSize = mLength * mElementSize;
}

CVariableOfCppVector::~CVariableOfCppVector()
{
    if (mCppVectorSGetters) {
        for (Int32 i = 0; i < mLength; i++) {
            if (mCppVectorSGetters[i]) mCppVectorSGetters[i]->Release();
        }
        delete [] mCppVectorSGetters;
    }
}

UInt32 CVariableOfCppVector::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CVariableOfCppVector::Release()
{
    return ElLightRefBase::Release();
}

PInterface CVariableOfCppVector::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)(ICppVectorSetter *)this;
    }
    else if (riid == EIID_ICppVectorSetter) {
        return (ICppVectorSetter *)this;
    }
    else if (riid == EIID_ICppVectorGetter) {
        return (ICppVectorGetter *)this;
    }

    return NULL;
}

ECode CVariableOfCppVector::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CVariableOfCppVector::Init()
{
    mCppVectorSGetters = new PInterface[mLength];
    if (!mCppVectorSGetters) {
        return E_OUT_OF_MEMORY;
    }
    memset(mCppVectorSGetters, 0, mLength * sizeof(IInterface*));

    return NOERROR;
}
//--------------Setter----------------------------------------------------------

ECode CVariableOfCppVector::ZeroAllElements()
{
    assert(mVarPtr);
    memset(mVarPtr, 0, mSize);
    return NOERROR;
}

ECode CVariableOfCppVector::SetAllElements(
    /* [in] */ PVoid value,
    /* [in] */ MemorySize size)
{
    if (!value || size < (MemorySize)mSize) {
        return E_INVALID_ARGUMENT;
    }

    assert(mVarPtr);
    memcpy(mVarPtr, value, mSize);
    return NOERROR;
}

ECode CVariableOfCppVector::SetElementValue(
    /* [in] */ Int32 index,
    /* [in] */ void* param,
    /* [in] */ CarDataType type)
{
    if (index < 0 || index >= mLength || !param) {
        return E_INVALID_ARGUMENT;
    }

    if (mDataType != type || !mVarPtr) {
        return E_INVALID_OPERATION;
    }

    memcpy((PByte)mVarPtr + mElementSize * index, param, mElementSize);

    return NOERROR;
}

ECode CVariableOfCppVector::SetInt16Element(
    /* [in] */ Int32 index,
    /* [in] */ Int16 value)
{
    return SetElementValue(index, &value, CarDataType_Int16);
}

ECode CVariableOfCppVector::SetInt32Element(
    /* [in] */ Int32 index,
    /* [in] */ Int32 value)
{
    return SetElementValue(index, &value, CarDataType_Int32);
}

ECode CVariableOfCppVector::SetInt64Element(
    /* [in] */ Int32 index,
    /* [in] */ Int64 value)
{
    return SetElementValue(index, &value, CarDataType_Int64);
}

ECode CVariableOfCppVector::SetByteElement(
    /* [in] */ Int32 index,
    /* [in] */ Byte value)
{
    return SetElementValue(index, &value, CarDataType_Byte);
}

ECode CVariableOfCppVector::SetFloatElement(
    /* [in] */ Int32 index,
    /* [in] */ Float value)
{
    return SetElementValue(index, &value, CarDataType_Float);
}

ECode CVariableOfCppVector::SetDoubleElement(
    /* [in] */ Int32 index,
    /* [in] */ Double value)
{
    return SetElementValue(index, &value, CarDataType_Double);
}

ECode CVariableOfCppVector::SetCharElement(
    /* [in] */ Int32 index,
    /* [in] */ Char32 value)
{
    return SetElementValue(index, &value, CarDataType_Char32);
}

ECode CVariableOfCppVector::SetBooleanElement(
    /* [in] */ Int32 index,
    /* [in] */ Boolean value)
{
    return SetElementValue(index, &value, CarDataType_Boolean);
}

ECode CVariableOfCppVector::SetEMuidElement(
    /* [in] */ Int32 index,
    /* [in] */ EMuid* value)
{
    return SetElementValue(index, (PVoid)value, CarDataType_EMuid);
}

ECode CVariableOfCppVector::SetEGuidElement(
    /* [in] */ Int32 index,
    /* [in] */ EGuid* value)
{
    return SetElementValue(index, (PVoid)value, CarDataType_EGuid);
}

ECode CVariableOfCppVector::SetECodeElement(
    /* [in] */ Int32 index,
    /* [in] */ ECode value)
{
    return SetElementValue(index, &value, CarDataType_ECode);
}

ECode CVariableOfCppVector::SetLocalPtrElement(
    /* [in] */ Int32 index,
    /* [in] */ LocalPtr value)
{
    return SetElementValue(index, &value, CarDataType_LocalPtr);
}

ECode CVariableOfCppVector::SetLocalTypeElement(
    /* [in] */ Int32 index,
    /* [in] */ PVoid value)
{
    return SetElementValue(index, value, CarDataType_LocalType);
}

ECode CVariableOfCppVector::SetEnumElement(
    /* [in] */ Int32 index,
    /* [in] */ Int32 value)
{
    return SetElementValue(index, &value, CarDataType_Enum);
}

ECode CVariableOfCppVector::GetStructElementSetter(
    /* [in] */ Int32 index,
    /* [out] */ IStructSetter** setter)
{
    if (index < 0 || index >= mLength || !setter) {
        return E_INVALID_ARGUMENT;
    }

    if (mDataType != CarDataType_Struct || !mVarPtr) {
        return E_INVALID_OPERATION;
    }

    CStructInfo* structInfo = (CStructInfo *)mElementTypeInfo.Get();

    AutoPtr<IVariableOfStruct> variable;
    ECode ec = structInfo->CreateVariableBox(
            (PByte)mVarPtr + mElementSize * index, (IVariableOfStruct**)&variable);
    if (FAILED(ec)) {
        return ec;
    }

    return variable->GetSetter(setter);
}

ECode CVariableOfCppVector::GetCppVectorElementSetter(
    /* [in] */ Int32 index,
    /* [out] */ ICppVectorSetter** setter)
{
    return AcquireCppVectorSGetter(index, TRUE, (IInterface**)setter);
}

//--------------Getter----------------------------------------------------------

ECode CVariableOfCppVector::GetLength(
    /* [out] */ Int32* length)
{
    if (!length) {
        return E_INVALID_ARGUMENT;
    }

    *length = mLength;

    return NOERROR;
}

ECode CVariableOfCppVector::GetRank(
    /* [out] */ Int32* retRank)
{
    if (!retRank) {
        return E_INVALID_ARGUMENT;
    }

    ECode ec = NOERROR;
    Int32 rank = 1;
    CarDataType type = mDataType;
    AutoPtr<IDataTypeInfo> elementTypeInfo;
    AutoPtr<IDataTypeInfo> elementTypeInfo2 = mElementTypeInfo;

    while (type == CarDataType_CppVector) {
        rank++;
        elementTypeInfo = NULL;
        ec = ((ICppVectorInfo *)elementTypeInfo2.Get())->GetElementTypeInfo(
                (IDataTypeInfo**)&elementTypeInfo);
        if (FAILED(ec)) return ec;

        ec = elementTypeInfo->GetDataType(&type);
        if (FAILED(ec)) {
            return ec;
        }
        elementTypeInfo2 = elementTypeInfo;
    }

    *retRank = rank;

    return NOERROR;
}

ECode CVariableOfCppVector::GetElementValue(
    /* [in] */ Int32 index,
    /* [in] */ void* param,
    /* [in] */ CarDataType type)
{
    if (index < 0 || index >= mLength || !param) {
        return E_INVALID_ARGUMENT;
    }

    if (mDataType != type || !mVarPtr) {
        return E_INVALID_OPERATION;
    }

    memcpy(param, (PByte)mVarPtr + mElementSize * index, mElementSize);

    return NOERROR;
}

ECode CVariableOfCppVector::GetInt16Element(
    /* [in] */ Int32 index,
    /* [out] */ Int16* value)
{
    return GetElementValue(index, value, CarDataType_Int16);
}

ECode CVariableOfCppVector::GetInt32Element(
    /* [in] */ Int32 index,
    /* [out] */ Int32* value)
{
    return GetElementValue(index, value, CarDataType_Int32);
}

ECode CVariableOfCppVector::GetInt64Element(
    /* [in] */ Int32 index,
    /* [out] */ Int64* value)
{
    return GetElementValue(index, value, CarDataType_Int64);
}

ECode CVariableOfCppVector::GetByteElement(
    /* [in] */ Int32 index,
    /* [out] */ Byte* value)
{
    return GetElementValue(index, value, CarDataType_Byte);
}

ECode CVariableOfCppVector::GetFloatElement(
    /* [in] */ Int32 index,
    /* [out] */ Float* value)
{
    return GetElementValue(index, value, CarDataType_Float);
}

ECode CVariableOfCppVector::GetDoubleElement(
    /* [in] */ Int32 index,
    /* [out] */ Double* value)
{
    return GetElementValue(index, value, CarDataType_Double);
}

ECode CVariableOfCppVector::GetCharElement(
    /* [in] */ Int32 index,
    /* [out] */ Char32* value)
{
    return GetElementValue(index, value, CarDataType_Char32);
}

ECode CVariableOfCppVector::GetBooleanElement(
    /* [in] */ Int32 index,
    /* [out] */ Boolean* value)
{
    return GetElementValue(index, value, CarDataType_Boolean);
}

ECode CVariableOfCppVector::GetEMuidElement(
    /* [in] */ Int32 index,
    /* [out] */ EMuid* value)
{
    return GetElementValue(index, value, CarDataType_EMuid);
}

ECode CVariableOfCppVector::GetEGuidElement(
    /* [in] */ Int32 index,
    /* [out] */ EGuid* value)
{
    return GetElementValue(index, value, CarDataType_EGuid);
}

ECode CVariableOfCppVector::GetECodeElement(
    /* [in] */ Int32 index,
    /* [out] */ ECode* value)
{
    return GetElementValue(index, value, CarDataType_ECode);
}

ECode CVariableOfCppVector::GetLocalPtrElement(
    /* [in] */ Int32 index,
    /* [out] */ LocalPtr* value)
{
    return GetElementValue(index, value, CarDataType_LocalPtr);
}

ECode CVariableOfCppVector::GetLocalTypeElement(
    /* [in] */ Int32 index,
    /* [out] */ PVoid value)
{
    return GetElementValue(index, value, CarDataType_LocalType);
}

ECode CVariableOfCppVector::GetEnumElement(
    /* [in] */ Int32 index,
    /* [out] */ Int32* value)
{
    return GetElementValue(index, value, CarDataType_Enum);
}

ECode CVariableOfCppVector::GetStructElementGetter(
    /* [in] */ Int32 index,
    /* [out] */ IStructGetter** getter)
{
    if (index < 0 || index >= mLength || !getter) {
        return E_INVALID_ARGUMENT;
    }

    if (mDataType != CarDataType_Struct || !mVarPtr) {
        return E_INVALID_OPERATION;
    }

    CStructInfo* structInfo = (CStructInfo *)mElementTypeInfo.Get();

    AutoPtr<IVariableOfStruct> variable;
    ECode ec = structInfo->CreateVariableBox(
            (PByte)mVarPtr + mElementSize * index, (IVariableOfStruct**)&variable);
    if (FAILED(ec)) {
        return ec;
    }

    return variable->GetGetter(getter);
}

ECode CVariableOfCppVector::GetCppVectorElementGetter(
    /* [in] */ Int32 index,
    /* [out] */ ICppVectorGetter** getter)
{
    return AcquireCppVectorSGetter(index, FALSE, (IInterface**)getter);
}

ECode CVariableOfCppVector::Rebox(
    /* [in] */ PVoid varPtr)
{
    if (!varPtr) {
        return E_INVALID_OPERATION;
    }

    mVarPtr = varPtr;
    return NOERROR;
}

ECode CVariableOfCppVector::GetSetter(
    /* [out] */ ICppVectorSetter** setter)
{
    if (!setter) {
        return E_INVALID_ARGUMENT;
    }

    *setter = (ICppVectorSetter *)this;
    (*setter)->AddRef();
    return NOERROR;
}

ECode CVariableOfCppVector::GetGetter(
    /* [out] */ ICppVectorGetter** getter)
{
    if (!getter) {
        return E_INVALID_ARGUMENT;
    }

    *getter = (ICppVectorGetter *)this;
    (*getter)->AddRef();
    return NOERROR;
}

ECode CVariableOfCppVector::AcquireCppVectorSGetter(
    /* [in] */ Int32 index,
    /* [in] */ Boolean isSetter,
    /* [out] */ IInterface** sGetter)
{
    if (index < 0 || index >= mLength || !sGetter) {
        return E_INVALID_ARGUMENT;
    }

    if (mDataType != CarDataType_CppVector || !mVarPtr) {
        return E_INVALID_OPERATION;
    }

    assert(mCppVectorSGetters);

    g_objInfoList.LockHashTable(EntryType_Struct);

    if (!mCppVectorSGetters[index]) {
        Int32 length = 0;
        ECode ec = ((ICppVectorInfo *)mElementTypeInfo.Get())->GetLength(&length);
        if (FAILED(ec)) {
            g_objInfoList.UnlockHashTable(EntryType_Struct);
            return ec;
        }

        AutoPtr<IDataTypeInfo> elementTypeInfo;
        ec = ((ICppVectorInfo *)mElementTypeInfo.Get())->GetElementTypeInfo(
                (IDataTypeInfo**)&elementTypeInfo);
        if (FAILED(ec)) {
            g_objInfoList.UnlockHashTable(EntryType_Struct);
            return ec;
        }

        AutoPtr<CVariableOfCppVector> sGetterObj = new CVariableOfCppVector(
                elementTypeInfo, length,
                (PByte)mVarPtr  + mElementSize * index);
        if (sGetterObj == NULL) {
            g_objInfoList.UnlockHashTable(EntryType_Struct);
            return E_OUT_OF_MEMORY;
        }

        ec = sGetterObj->Init();
        if (FAILED(ec)) {
            g_objInfoList.UnlockHashTable(EntryType_Struct);
            return ec;
        }

        if (isSetter) {
            sGetterObj->GetSetter(
                    (ICppVectorSetter**)&mCppVectorSGetters[index]);
        }
        else {
            sGetterObj->GetGetter(
                    (ICppVectorGetter**)&mCppVectorSGetters[index]);
        }
    }

    *sGetter = mCppVectorSGetters[index];
    (*sGetter)->AddRef();

    g_objInfoList.UnlockHashTable(EntryType_Struct);

    return NOERROR;
}
