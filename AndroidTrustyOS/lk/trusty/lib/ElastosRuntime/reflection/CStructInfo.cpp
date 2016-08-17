//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CStructInfo.h"
#include "CFieldInfo.h"
#include "CVariableOfStruct.h"
#include "CCarArrayInfo.h"
#include "CCppVectorInfo.h"

CStructInfo::CStructInfo()
{
    mStructFieldDesc = NULL;
    mSize = 0;
}

CStructInfo::~CStructInfo()
{
    if (mStructFieldDesc) delete [] mStructFieldDesc;

    if (mFieldInfos) {
        for (Int32 i = 0; i < mFieldInfos->GetLength(); i++) {
            if ((*mFieldInfos)[i]) {
                delete (CFieldInfo*)(*mFieldInfos)[i];
                (*mFieldInfos)[i] = NULL;
            }
        }
        ArrayOf<IFieldInfo *>::Free(mFieldInfos);
    }
}

UInt32 CStructInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CStructInfo::Release()
{
    g_objInfoList.LockHashTable(EntryType_Struct);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        if (mClsModule == NULL) {
            g_objInfoList.DetachStructInfo(this);
        }
        else {
            g_objInfoList.RemoveStructInfo(mStructDirEntry);
        }
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_Struct);
    assert(ref >= 0);
    return ref;
}

PInterface CStructInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IStructInfo) {
        return (IStructInfo *)this;
    }
    else if (riid == EIID_IDataTypeInfo) {
        return (IDataTypeInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CStructInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CStructInfo::InitStatic(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ StructDirEntry* structDirEntry)
{
    mClsModule = clsModule;

    Int32 base = mClsModule->mBase;
    mStructDirEntry = structDirEntry;
    mName = adjustNameAddr(base, mStructDirEntry->mName);
    StructDescriptor* desc = adjustStructDescAddr(base, structDirEntry->mDesc);
    StructElement* elem = NULL;

    ECode ec = NOERROR;
    mFieldTypeInfos = ArrayOf<IDataTypeInfo*>::Alloc(desc->mElementCount);
    if (!mFieldTypeInfos) {
        ec = E_OUT_OF_MEMORY;
        goto EExit;
    }

    mFieldNames = ArrayOf<String>::Alloc(desc->mElementCount);
    if (!mFieldNames) {
        ec = E_OUT_OF_MEMORY;
        goto EExit;
    }

    for (Int32 i = 0; i < desc->mElementCount; i++) {
        elem = getStructElementAddr(base, desc->mElements, i);
        (*mFieldNames)[i] = adjustNameAddr(base, elem->mName);

        AutoPtr<IDataTypeInfo> dataTypeInfo;
        ec = g_objInfoList.AcquireDataTypeInfo(mClsModule,
                &elem->mType, (IDataTypeInfo**)&dataTypeInfo, TRUE);
        if (FAILED(ec)) goto EExit;
        mFieldTypeInfos->Set(i, dataTypeInfo);
    }

    ec = InitFieldElement();
    if (FAILED(ec)) goto EExit;

    ec = InitFieldInfos();
    if (FAILED(ec)) goto EExit;

    return NOERROR;

EExit:
    mFieldNames = NULL;
    mFieldTypeInfos = NULL;

    return ec;
}

ECode CStructInfo::InitDynamic(
    /* [in] */ const String& name,
    /* [in] */ ArrayOf<String>* fieldNames,
    /* [in] */ ArrayOf<IDataTypeInfo*>* fieldTypeInfos)
{
    if (fieldNames == NULL || fieldTypeInfos == NULL) {
        return E_INVALID_ARGUMENT;
    }

    mName = name;
    mFieldNames = fieldNames;
    mFieldTypeInfos = fieldTypeInfos;

    ECode ec = InitFieldElement();
    if (FAILED(ec)) goto EExit;

    ec = InitFieldInfos();
    if (FAILED(ec)) goto EExit;

    return NOERROR;

EExit:
    mFieldNames = NULL;
    mFieldTypeInfos = NULL;

    return ec;
}

ECode CStructInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = mName;
    return NOERROR;
}

ECode CStructInfo::GetSize(
    /* [out] */ MemorySize* size)
{
    if (!size) {
        return E_INVALID_ARGUMENT;
    }

    *size = mSize;
    return NOERROR;
}

ECode CStructInfo::GetDataType(
    /* [out] */ CarDataType* dataType)
{
    if (!dataType) {
        return E_INVALID_ARGUMENT;
    }

    *dataType = CarDataType_Struct;
    return NOERROR;
}

ECode CStructInfo::GetModuleInfo(
    /* [out] */ IModuleInfo** moduleInfo)
{
    if (mClsModule) {
        return mClsModule->GetModuleInfo(moduleInfo);
    }
    else {
        return E_INVALID_OPERATION;
    }
}

ECode CStructInfo::GetFieldCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mFieldNames->GetLength();

    return NOERROR;
}

ECode CStructInfo::GetAllFieldInfos(
    /* [out] */ ArrayOf<IFieldInfo *>* fieldInfos)
{
    if (!fieldInfos || !fieldInfos->GetLength()) {
        return E_INVALID_ARGUMENT;
    }

    fieldInfos->Copy(mFieldInfos);

    return NOERROR;
}

ECode CStructInfo::GetFieldInfo(
    /* [in] */ const String& name,
    /* [out] */ IFieldInfo** fieldInfo)
{
    if (name.IsNull() || !fieldInfo) {
        return E_INVALID_ARGUMENT;
    }

    for (Int32 i = 0; i < mFieldNames->GetLength(); i++) {
        if ((*mFieldNames)[i].Equals(name)) {
            *fieldInfo = (*mFieldInfos)[i];
            (*fieldInfo)->AddRef();
            return NOERROR;
        }
    }

    return E_DOES_NOT_EXIST;
}

ECode CStructInfo::GetMaxAlignSize(
    /* [out] */ MemorySize* alignSize)
{
    Int32 size = 0, align = 1;

    CarDataType dataType;
    for (Int32 i = 0; i < mFieldTypeInfos->GetLength(); i++) {
        ((IDataTypeInfo *)(*mFieldTypeInfos)[i])->GetDataType(&dataType);
        if (dataType == CarDataType_Struct) {
            ((CStructInfo *)(*mFieldTypeInfos)[i])->GetMaxAlignSize(&size);
        }
        else if (dataType == CarDataType_ArrayOf) {
            ((CCarArrayInfo *)(*mFieldTypeInfos)[i])->GetMaxAlignSize(&size);
        }
        else if (dataType == CarDataType_CppVector) {
            ((CCppVectorInfo *)(*mFieldTypeInfos)[i])->GetMaxAlignSize(&size);
        }
        else {
            ((IDataTypeInfo *)(*mFieldTypeInfos)[i])->GetSize(&size);
        }

        if (size > align) align = size;
    }

    if (align > ALIGN_BOUND) align = ALIGN_BOUND;

    *alignSize = align;

    return NOERROR;
}

ECode CStructInfo::InitFieldInfos()
{
    Int32 i = 0, count = mFieldTypeInfos->GetLength();
    mFieldInfos = ArrayOf<IFieldInfo *>::Alloc(count);
    if (mFieldInfos == NULL) {
        return E_OUT_OF_MEMORY;
    }

    for (i = 0; i < count; i++) {
        // do not use mFieldInfos->Set(i, xxx), because it will call
        // xxx->AddRef()
        (*mFieldInfos)[i] = new CFieldInfo(this, (*mFieldNames)[i],
                (IDataTypeInfo*)(*mFieldTypeInfos)[i]);
        if (!(*mFieldInfos)[i]) goto EExit;
    }

    return NOERROR;

EExit:
    for (i = 0; i < count; i++) {
        if ((*mFieldInfos)[i]) {
            delete (CFieldInfo*)(*mFieldInfos)[i];
            (*mFieldInfos)[i] = NULL;
        }
    }
    ArrayOf<IFieldInfo *>::Free(mFieldInfos);
    mFieldInfos = NULL;

    return NOERROR;
}

CAR_INLINE UInt32 AdjuctElemOffset(UInt32 offset, UInt32 elemSize, UInt32 align)
{
    if (elemSize > align) elemSize = align;
    UInt32 mode = offset % elemSize;
    if (mode != 0) {
        offset = offset - mode + elemSize;
    }

    return offset;
}

ECode CStructInfo::InitFieldElement()
{
    Int32 count = mFieldTypeInfos->GetLength();
    mStructFieldDesc = new StructFieldDesc[count];
    if (!mStructFieldDesc) {
        return E_OUT_OF_MEMORY;
    }

    UInt32 offset = 0;
    ECode ec = NOERROR;

    CarDataType dataType;
    Int32 align, elemtAlign, elemtSize;
    ec = GetMaxAlignSize(&align);
    if (FAILED(ec)) return ec;

    for (Int32 i = 0; i < count; i++) {
        ((IDataTypeInfo *)(*mFieldTypeInfos)[i])->GetDataType(&dataType);
        if (dataType == CarDataType_Struct) {
            ((CStructInfo *)(*mFieldTypeInfos)[i])->GetMaxAlignSize(&elemtAlign);
            ((CStructInfo *)(*mFieldTypeInfos)[i])->GetSize(&elemtSize);
        }
        else if (dataType == CarDataType_ArrayOf) {
            ((CCarArrayInfo *)(*mFieldTypeInfos)[i])->GetMaxAlignSize(&elemtAlign);
            ((CCarArrayInfo *)(*mFieldTypeInfos)[i])->GetSize(&elemtSize);
        }
        else if (dataType == CarDataType_CppVector) {
            ((CCppVectorInfo *)(*mFieldTypeInfos)[i])->GetMaxAlignSize(&elemtAlign);
            ((CCppVectorInfo *)(*mFieldTypeInfos)[i])->GetSize(&elemtSize);
        }
        else {
            ((IDataTypeInfo *)(*mFieldTypeInfos)[i])->GetSize(&elemtSize);
            elemtAlign = align;
        }

        mStructFieldDesc[i].mType = dataType;
        mStructFieldDesc[i].mSize = elemtSize;
        //Adjuct the Element Size
        offset = AdjuctElemOffset(offset, elemtSize, elemtAlign);

        mStructFieldDesc[i].mPos = offset;
        offset += mStructFieldDesc[i].mSize;

        mStructFieldDesc[i].mName = (char *)(*mFieldNames)[i].string();
    }

    mSize = AdjuctElemOffset(offset, align, align);

    return NOERROR;
}

ECode CStructInfo::CreateVarBox(
    /* [in] */ PVoid variableDescriptor,
    /* [out] */ IVariableOfStruct** variable)
{
    AutoPtr<CVariableOfStruct> structBox = new CVariableOfStruct();
    if (structBox == NULL) {
        return E_OUT_OF_MEMORY;
    }

    ECode ec = structBox->Init(this, variableDescriptor);
    if (FAILED(ec)) {
        return ec;
    }

    *variable = (IVariableOfStruct *)structBox;
    (*variable)->AddRef();

    return NOERROR;
}

ECode CStructInfo::CreateVariable(
    /* [out] */ IVariableOfStruct** variable)
{
    if (!variable) {
        return E_INVALID_ARGUMENT;
    }

    return CreateVarBox(NULL, variable);
}

ECode CStructInfo::CreateVariableBox(
    /* [in] */ PVoid variableDescriptor,
    /* [out] */ IVariableOfStruct** variable)
{
    if (!variableDescriptor || !variable) {
        return E_INVALID_ARGUMENT;
    }

    return CreateVarBox(variableDescriptor, variable);
}
