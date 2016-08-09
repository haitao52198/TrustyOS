//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CVariableOfStruct.h"
#include "CStructInfo.h"
#include "CVariableOfCppVector.h"

CVariableOfStruct::CVariableOfStruct()
{
    mVarBuf = NULL;
    mIsAlloc = FALSE;
    mVarSize = 0;
    mCount = 0;
    mCppVectorSGetters = NULL;
}

CVariableOfStruct::~CVariableOfStruct()
{
    if (mIsAlloc && mVarBuf) free(mVarBuf);
    if (mCppVectorSGetters) {
        for (Int32 i = 0; i < mCount; i++) {
            if (mCppVectorSGetters[i]) mCppVectorSGetters[i]->Release();
        }
        delete [] mCppVectorSGetters;
    }
}

UInt32 CVariableOfStruct::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CVariableOfStruct::Release()
{
    return ElLightRefBase::Release();
}

PInterface CVariableOfStruct::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)(IVariableOfStruct *)this;
    }
    else if (riid == EIID_IVariableOfStruct) {
        return (IVariableOfStruct *)this;
    }
    else if (riid == EIID_IStructSetter) {
        return (IStructSetter *)this;
    }
    else if (riid == EIID_IStructGetter) {
        return (IStructGetter *)this;
    }
    return NULL;
}

ECode CVariableOfStruct::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CVariableOfStruct::Init(
    /* [in] */ IStructInfo* structInfo,
    /* [in] */ PVoid varBuf)
{
    ECode ec = structInfo->GetFieldCount(&mCount);
    if (FAILED(ec)) return ec;

    mStructInfo = structInfo;
    mStructFieldDesc = ((CStructInfo *)mStructInfo.Get())->mStructFieldDesc;
    mVarSize = ((CStructInfo *)mStructInfo.Get())->mSize;
    if (!varBuf) {
        mVarBuf = (PByte)malloc(mVarSize);
        if (mVarBuf == NULL) {
            return E_OUT_OF_MEMORY;
        }

        mIsAlloc = TRUE;
    }
    else {
        mVarBuf = (PByte)varBuf;
        mIsAlloc = FALSE;
    }

    //Get CppVector fields Info
    mCppVectorSGetters = new PInterface[mCount];
    if (!mCppVectorSGetters) {
        delete mVarBuf;
        mVarBuf = NULL;
        return E_OUT_OF_MEMORY;
    }
    memset(mCppVectorSGetters, 0, mCount * sizeof(IInterface*));

    return NOERROR;
}

ECode CVariableOfStruct::GetTypeInfo(
    /* [out] */ IDataTypeInfo** typeInfo)
{
    if (!typeInfo) {
        return E_INVALID_ARGUMENT;
    }

    *typeInfo = mStructInfo;
    (*typeInfo)->AddRef();
    return NOERROR;
}

ECode CVariableOfStruct::GetPayload(
    /* [out] */ PVoid* payload)
{
    if (!payload) {
        return E_INVALID_ARGUMENT;
    }

    *payload = mVarBuf;

    return NOERROR;
}

ECode CVariableOfStruct::Rebox(
    /* [in] */ PVoid localVariablePtr)
{
    if (!localVariablePtr) {
        return E_INVALID_OPERATION;
    }

    if (mIsAlloc && mVarBuf) {
        free(mVarBuf);
        mIsAlloc = FALSE;
    }

    mVarBuf = (PByte)localVariablePtr;

    if (mCppVectorSGetters) {
        for (Int32 i = 0; i < mCount; i++) {
            if (mCppVectorSGetters[i]) {
                ((CVariableOfCppVector *)(ICppVectorSetter *)mCppVectorSGetters[i])->Rebox(
                        mVarBuf + mStructFieldDesc[i].mPos);
            }
        }
    }

    return NOERROR;
}

ECode CVariableOfStruct::GetSetter(
    /* [out] */ IStructSetter** setter)
{
    if (!setter) {
        return E_INVALID_ARGUMENT;
    }

    *setter = (IStructSetter *)this;
    (*setter)->AddRef();

    return NOERROR;
}

ECode CVariableOfStruct::GetGetter(
    /* [out] */ IStructGetter** getter)
{
    if (!getter) {
        return E_INVALID_ARGUMENT;
    }

    *getter = (IStructGetter *)this;
    (*getter)->AddRef();

    return NOERROR;
}

//--------------Setter----------------------------------------------------------

ECode CVariableOfStruct::GetIndexByName(
    /* [in] */ const String& name,
    /* [out] */ Int32* index)
{
    Int32 count = 0;
    ECode ec = mStructInfo->GetFieldCount(&count);
    if (FAILED(ec)) return ec;

    for (Int32 i = 0; i < count; i++) {
        if (name.Equals(mStructFieldDesc[i].mName)) {
            *index = i;
            return NOERROR;
        }
    }

    return E_DOES_NOT_EXIST;
}

ECode CVariableOfStruct::SetFieldValueByName(
    /* [in] */ const String& name,
    /* [in] */ void* param,
    /* [in] */ CarDataType type)
{
    if(name.IsNull() || !param) {
        return E_INVALID_ARGUMENT;
    }

    if (!mVarBuf) {
        return E_INVALID_OPERATION;
    }

    Int32 index = 0;
    ECode ec = GetIndexByName(name, &index);
    if (FAILED(ec)) return ec;

    if (type != mStructFieldDesc[index].mType) {
        return E_INVALID_ARGUMENT;
    }

    memcpy(mVarBuf + mStructFieldDesc[index].mPos, param,
            mStructFieldDesc[index].mSize);

    return NOERROR;
}

ECode CVariableOfStruct::ZeroAllFields()
{
    if (!mVarBuf) {
        return E_INVALID_OPERATION;
    }
    else {
        memset(mVarBuf, 0, mVarSize);
        return NOERROR;
    }
}

ECode CVariableOfStruct::SetInt16Field(
    /* [in] */ const String& name,
    /* [in] */ Int16 value)
{
    return SetFieldValueByName(name, &value, CarDataType_Int16);
}

ECode CVariableOfStruct::SetInt32Field(
    /* [in] */ const String& name,
    /* [in] */ Int32 value)
{
    return SetFieldValueByName(name, &value, CarDataType_Int32);
}

ECode CVariableOfStruct::SetInt64Field(
    /* [in] */ const String& name,
    /* [in] */ Int64 value)
{
    return SetFieldValueByName(name, &value, CarDataType_Int64);
}

ECode CVariableOfStruct::SetByteField(
    /* [in] */ const String& name,
    /* [in] */ Byte value)
{
    return SetFieldValueByName(name, &value, CarDataType_Byte);
}

ECode CVariableOfStruct::SetFloatField(
    /* [in] */ const String& name,
    /* [in] */ Float value)
{
    return SetFieldValueByName(name, &value, CarDataType_Float);
}

ECode CVariableOfStruct::SetDoubleField(
    /* [in] */ const String& name,
    /* [in] */ Double value)
{
    return SetFieldValueByName(name, &value, CarDataType_Double);
}

ECode CVariableOfStruct::SetCharField(
    /* [in] */ const String& name,
    /* [in] */ Char32 value)
{
    return SetFieldValueByName(name, &value, CarDataType_Char32);
}

ECode CVariableOfStruct::SetBooleanField(
    /* [in] */ const String& name,
    /* [in] */ Boolean value)
{
    return SetFieldValueByName(name, &value, CarDataType_Boolean);
}

ECode CVariableOfStruct::SetEMuidField(
    /* [in] */ const String& name,
    /* [in] */ EMuid* value)
{
    return SetFieldValueByName(name, &value, CarDataType_EMuid);
}

ECode CVariableOfStruct::SetEGuidField(
    /* [in] */ const String& name,
    /* [in] */ EGuid* value)
{
    return SetFieldValueByName(name, &value, CarDataType_EGuid);
}

ECode CVariableOfStruct::SetECodeField(
    /* [in] */ const String& name,
    /* [in] */ ECode value)
{
    return SetFieldValueByName(name, &value, CarDataType_ECode);
}

ECode CVariableOfStruct::SetLocalPtrField(
    /* [in] */ const String& name,
    /* [in] */ LocalPtr value)
{
    return SetFieldValueByName(name, &value, CarDataType_LocalPtr);
}

ECode CVariableOfStruct::SetLocalTypeField(
    /* [in] */ const String& name,
    /* [in] */ PVoid value)
{
    return SetFieldValueByName(name, value, CarDataType_LocalType);
}

ECode CVariableOfStruct::SetEnumField(
    /* [in] */ const String& name,
    /* [in] */ Int32 value)
{
    return SetFieldValueByName(name, &value, CarDataType_Enum);
}

ECode CVariableOfStruct::GetStructFieldSetter(
    /* [in] */ const String& name,
    /* [out] */ IStructSetter** setter)
{
    if (name.IsNull() || !setter) {
        return E_INVALID_ARGUMENT;
    }

    Int32 index = 0;
    ECode ec = GetIndexByName(name, &index);
    if (FAILED(ec)) return ec;

    if (mStructFieldDesc[index].mType != CarDataType_Struct) {
        return E_INVALID_ARGUMENT;
    }

    AutoPtr<IFieldInfo> fieldInfo;
    AutoPtr<IStructInfo> structInfo;
    AutoPtr<IDataTypeInfo> dataTypeInfo;
    ec = mStructInfo->GetFieldInfo(name, (IFieldInfo**)&fieldInfo);
    if (FAILED(ec)) return ec;

    ec = fieldInfo->GetTypeInfo((IDataTypeInfo**)&dataTypeInfo);
    if (FAILED(ec)) return ec;

    structInfo = IStructInfo::Probe(dataTypeInfo);
    AutoPtr<IVariableOfStruct> variable;
    ec = structInfo->CreateVariableBox(
            mVarBuf + mStructFieldDesc[index].mPos, (IVariableOfStruct**)&variable);
    if (FAILED(ec)) return ec;

    return variable->GetSetter(setter);
}

ECode CVariableOfStruct::GetCppVectorFieldSetter(
    /* [in] */ const String& name,
    /* [out] */ ICppVectorSetter** setter)
{
    return AcquireCppVectorFieldSGetter(name, TRUE, (IInterface**)setter);
}

//--------------Getter----------------------------------------------------------

ECode CVariableOfStruct::GetFieldValueByName(
    /* [in] */ const String& name,
    /* [in] */ void* param,
    /* [in] */ CarDataType type)
{
    if(name.IsNull() || !param) {
        return E_INVALID_ARGUMENT;
    }

    if (!mVarBuf) {
        return E_INVALID_OPERATION;
    }

    Int32 index = 0;
    ECode ec = GetIndexByName(name, &index);
    if (FAILED(ec)) return ec;

    if (type != mStructFieldDesc[index].mType) {
        return E_INVALID_ARGUMENT;
    }

    memcpy(param, mVarBuf + mStructFieldDesc[index].mPos,
            mStructFieldDesc[index].mSize);

    return NOERROR;
}

ECode CVariableOfStruct::GetInt16Field(
    /* [in] */ const String& name,
    /* [out] */ Int16* value)
{
    return GetFieldValueByName(name, value, CarDataType_Int16);
}

ECode CVariableOfStruct::GetInt32Field(
    /* [in] */ const String& name,
    /* [out] */ Int32* value)
{
    return GetFieldValueByName(name, value, CarDataType_Int32);
}

ECode CVariableOfStruct::GetInt64Field(
    /* [in] */ const String& name,
    /* [out] */ Int64* value)
{
    return GetFieldValueByName(name, value, CarDataType_Int64);
}

ECode CVariableOfStruct::GetByteField(
    /* [in] */ const String& name,
    /* [out] */ Byte* value)
{
    return GetFieldValueByName(name, value, CarDataType_Byte);
}

ECode CVariableOfStruct::GetFloatField(
    /* [in] */ const String& name,
    /* [out] */ Float* value)
{
    return GetFieldValueByName(name, value, CarDataType_Float);
}

ECode CVariableOfStruct::GetDoubleField(
    /* [in] */ const String& name,
    /* [out] */ Double* value)
{
    return GetFieldValueByName(name, value, CarDataType_Double);
}

ECode CVariableOfStruct::GetCharField(
    /* [in] */ const String& name,
    /* [out] */ Char32* value)
{
    return GetFieldValueByName(name, value, CarDataType_Char32);
}

ECode CVariableOfStruct::GetBooleanField(
    /* [in] */ const String& name,
    /* [out] */ Boolean* value)
{
    return GetFieldValueByName(name, value, CarDataType_Boolean);
}

ECode CVariableOfStruct::GetEMuidField(
    /* [in] */ const String& name,
    /* [out] */ EMuid* value)
{
    return GetFieldValueByName(name, value, CarDataType_EMuid);
}

ECode CVariableOfStruct::GetEGuidField(
    /* [in] */ const String& name,
    /* [out] */ EGuid* value)
{
    return GetFieldValueByName(name, value, CarDataType_EGuid);
}

ECode CVariableOfStruct::GetECodeField(
    /* [in] */ const String& name,
    /* [out] */ ECode* value)
{
    return GetFieldValueByName(name, value, CarDataType_ECode);
}

ECode CVariableOfStruct::GetLocalPtrField(
    /* [in] */ const String& name,
    /* [out] */ LocalPtr* value)
{
    return GetFieldValueByName(name, value, CarDataType_LocalPtr);
}

ECode CVariableOfStruct::GetLocalTypeField(
    /* [in] */ const String& name,
    /* [out] */ PVoid value)
{
    return GetFieldValueByName(name, value, CarDataType_LocalType);
}

ECode CVariableOfStruct::GetEnumField(
    /* [in] */ const String& name,
    /* [out] */ Int32* value)
{
    return GetFieldValueByName(name, value, CarDataType_Enum);
}

ECode CVariableOfStruct::GetStructFieldGetter(
    /* [in] */ const String& name,
    /* [out] */ IStructGetter** getter)
{
    if (name.IsNull() || !getter) {
        return E_INVALID_ARGUMENT;
    }

    Int32 index = 0;
    ECode ec = GetIndexByName(name, &index);
    if (FAILED(ec)) return ec;

    if (mStructFieldDesc[index].mType != CarDataType_Struct) {
        return E_INVALID_ARGUMENT;
    }

    AutoPtr<IFieldInfo> fieldInfo;
    AutoPtr<IStructInfo> structInfo;
    AutoPtr<IDataTypeInfo> dataTypeInfo;
    ec = mStructInfo->GetFieldInfo(name, (IFieldInfo**)&fieldInfo);
    if (FAILED(ec)) return ec;

    ec = fieldInfo->GetTypeInfo((IDataTypeInfo**)&dataTypeInfo);
    if (FAILED(ec)) return ec;

    structInfo = IStructInfo::Probe(dataTypeInfo);
    AutoPtr<IVariableOfStruct> variable;
    ec = structInfo->CreateVariableBox(
            mVarBuf + mStructFieldDesc[index].mPos, (IVariableOfStruct**)&variable);
    if (FAILED(ec)) return ec;

    return variable->GetGetter(getter);
}

ECode CVariableOfStruct::GetCppVectorFieldGetter(
    /* [in] */ const String& name,
    /* [out] */ ICppVectorGetter** getter)
{
    return AcquireCppVectorFieldSGetter(name, FALSE, (IInterface**)getter);
}

ECode CVariableOfStruct::AcquireCppVectorFieldSGetter(
    /* [in] */ const String& name,
    /* [in] */ Boolean isSetter,
    /* [out] */ IInterface** sGetter)
{
    if (name.IsNull() || !sGetter) {
        return E_INVALID_ARGUMENT;
    }

    Int32 index = 0;
    ECode ec = GetIndexByName(name, &index);
    if (FAILED(ec)) return ec;

    if (mStructFieldDesc[index].mType != CarDataType_CppVector) {
        return E_INVALID_ARGUMENT;
    }

    assert(mCppVectorSGetters);

    g_objInfoList.LockHashTable(EntryType_Struct);

    CVariableOfCppVector* sGetterObj = (CVariableOfCppVector *)
            (IVariableOfCarArray *)mCppVectorSGetters[index];
    if (!sGetterObj) {
        AutoPtr<IFieldInfo> fieldInfo;
        ec = mStructInfo->GetFieldInfo(name, (IFieldInfo**)&fieldInfo);
        if (FAILED(ec)) {
            g_objInfoList.UnlockHashTable(EntryType_Struct);
            return ec;
        }

        AutoPtr<IDataTypeInfo> typeInfoObj;
        fieldInfo->GetTypeInfo((IDataTypeInfo**)&typeInfoObj);
        if (FAILED(ec)) {
            g_objInfoList.UnlockHashTable(EntryType_Struct);
            return ec;
        }
        ICppVectorInfo* typeInfo = ICppVectorInfo::Probe(typeInfoObj);

        Int32 length = 0;
        ec = typeInfo->GetLength(&length);
        if (FAILED(ec)) {
            g_objInfoList.UnlockHashTable(EntryType_Struct);
            return ec;
        }

        AutoPtr<IDataTypeInfo> elementTypeInfo;
        ec = typeInfo->GetElementTypeInfo((IDataTypeInfo**)&elementTypeInfo);
        if (FAILED(ec)) {
            g_objInfoList.UnlockHashTable(EntryType_Struct);
            return ec;
        }

        sGetterObj = new CVariableOfCppVector(
                elementTypeInfo, length,
                mVarBuf + mStructFieldDesc[index].mPos);
        if (sGetterObj == NULL) {
            g_objInfoList.UnlockHashTable(EntryType_Struct);
            return E_OUT_OF_MEMORY;
        }

        ec = sGetterObj->Init();
        if (FAILED(ec)) {
            delete sGetterObj;
            g_objInfoList.UnlockHashTable(EntryType_Struct);
            return ec;
        }

        sGetterObj->AddRef();
        mCppVectorSGetters[index] = (IVariableOfCarArray *)sGetterObj;
    }

    if (isSetter) {
        sGetterObj->GetSetter((ICppVectorSetter**)sGetter);
    }
    else {
        sGetterObj->GetGetter((ICppVectorGetter**)sGetter);
    }

    g_objInfoList.UnlockHashTable(EntryType_Struct);

    return NOERROR;
}
