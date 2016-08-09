//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CSTRUCTINFO_H__
#define __CSTRUCTINFO_H__

#include "CEntryList.h"

class CStructInfo
    : public ElLightRefBase
    , public IStructInfo
{
public:
    CStructInfo();

    virtual ~CStructInfo();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI InitStatic(
        /* [in] */ CClsModule* clsModule,
        /* [in] */ StructDirEntry* structDirEntry);

    CARAPI InitDynamic(
        /* [in] */ const String& name,
        /* [in] */ ArrayOf<String>* fieldNames,
        /* [in] */ ArrayOf<IDataTypeInfo*>* fieldTypeInfos);

    CARAPI GetName(
        /* [out] */ String* name);

    CARAPI GetSize(
        /* [out] */ MemorySize* size);

    CARAPI GetDataType(
        /* [out] */ CarDataType* dataType);

    CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo** moduleInfo);

    CARAPI GetFieldCount(
        /* [out] */ Int32* count);

    CARAPI GetAllFieldInfos(
        /* [out] */ ArrayOf<IFieldInfo *>* fieldInfos);

    CARAPI GetFieldInfo(
        /* [in] */ const String& name,
        /* [out] */ IFieldInfo** fieldInfo);

    CARAPI CreateVariable(
        /* [out] */ IVariableOfStruct** variable);

    CARAPI CreateVariableBox(
        /* [in] */ PVoid variableDescriptor,
        /* [out] */ IVariableOfStruct** variable);

    CARAPI InitFieldInfos();

    CARAPI InitFieldElement();

    CARAPI CreateVarBox(
        /* [in] */ PVoid varPtr,
        /* [out] */ IVariableOfStruct** variable);

    CARAPI GetMaxAlignSize(
        /* [out] */ MemorySize* alignSize);

public:
    AutoPtr< ArrayOf<String> >          mFieldNames;
    AutoPtr< ArrayOf<IDataTypeInfo*> >  mFieldTypeInfos;
    StructFieldDesc*                    mStructFieldDesc;
    UInt32                              mSize;

private:
    AutoPtr<CClsModule>                 mClsModule;
    StructDirEntry*                     mStructDirEntry;

    ArrayOf<IFieldInfo *>*              mFieldInfos;
    String                              mName;
};

#endif // __CSTRUCTINFO_H__
