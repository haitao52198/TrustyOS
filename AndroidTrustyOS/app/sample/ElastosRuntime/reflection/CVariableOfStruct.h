//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CSTRUCTVARIABLE_H__
#define __CSTRUCTVARIABLE_H__

#include "refutil.h"

class CVariableOfStruct
    : public ElLightRefBase
    , public IVariableOfStruct
    , public IStructSetter
    , public IStructGetter
{
public:
    CVariableOfStruct();

    virtual ~CVariableOfStruct();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI Init(
        /* [in] */ IStructInfo* structInfo,
        /* [in] */ PVoid varBuf);

    CARAPI GetTypeInfo(
        /* [out] */ IDataTypeInfo** typeInfo);

    CARAPI GetPayload(
        /* [out] */ PVoid* payload);

    CARAPI Rebox(
        /* [in] */ PVoid localVariablePtr);

    CARAPI GetSetter(
        /* [out] */ IStructSetter** setter);

    CARAPI GetGetter(
        /* [out] */ IStructGetter** getter);

//--------------Setter----------------------------------------------------------

    CARAPI ZeroAllFields();

    CARAPI SetInt16Field(
        /* [in] */ const String& name,
        /* [in] */ Int16 value);

    CARAPI SetInt32Field(
        /* [in] */ const String& name,
        /* [in] */ Int32 value);

    CARAPI SetInt64Field(
        /* [in] */ const String& name,
        /* [in] */ Int64 value);

    CARAPI SetByteField(
        /* [in] */ const String& name,
        /* [in] */ Byte value);

    CARAPI SetFloatField(
        /* [in] */ const String& name,
        /* [in] */ Float value);

    CARAPI SetDoubleField(
        /* [in] */ const String& name,
        /* [in] */ Double value);

    CARAPI SetCharField(
        /* [in] */ const String& name,
        /* [in] */ Char32 value);

    CARAPI SetBooleanField(
        /* [in] */ const String& name,
        /* [in] */ Boolean value);

    CARAPI SetEMuidField(
        /* [in] */ const String& name,
        /* [in] */ EMuid* value);

    CARAPI SetEGuidField(
        /* [in] */ const String& name,
        /* [in] */ EGuid* value);

    CARAPI SetECodeField(
        /* [in] */ const String& name,
        /* [in] */ ECode value);

    CARAPI SetLocalPtrField(
        /* [in] */ const String& name,
        /* [in] */ LocalPtr value);

    CARAPI SetLocalTypeField(
        /* [in] */ const String& name,
        /* [in] */ PVoid value);

    CARAPI SetEnumField(
        /* [in] */ const String& name,
        /* [in] */ Int32 value);

    CARAPI GetStructFieldSetter(
        /* [in] */ const String& name,
        /* [out] */ IStructSetter** setter);

    CARAPI GetCppVectorFieldSetter(
        /* [in] */ const String& name,
        /* [out] */ ICppVectorSetter** setter);

    CARAPI SetFieldValueByName(
        /* [in] */ const String& name,
        /* [in] */ void* param,
        /* [in] */ CarDataType type);

//--------------Getter----------------------------------------------------------

    CARAPI GetInt16Field(
        /* [in] */ const String& name,
        /* [out] */ Int16* value);

    CARAPI GetInt32Field(
        /* [in] */ const String& name,
        /* [out] */ Int32* value);

    CARAPI GetInt64Field(
        /* [in] */ const String& name,
        /* [out] */ Int64* value);

    CARAPI GetByteField(
        /* [in] */ const String& name,
        /* [out] */ Byte* value);

    CARAPI GetFloatField(
        /* [in] */ const String& name,
        /* [out] */ Float* value);

    CARAPI GetDoubleField(
        /* [in] */ const String& name,
        /* [out] */ Double* value);

    CARAPI GetCharField(
        /* [in] */ const String& name,
        /* [out] */ Char32* value);

    CARAPI GetBooleanField(
        /* [in] */ const String& name,
        /* [out] */ Boolean* value);

    CARAPI GetEMuidField(
        /* [in] */ const String& name,
        /* [out] */ EMuid* value);

    CARAPI GetEGuidField(
        /* [in] */ const String& name,
        /* [out] */ EGuid* value);

    CARAPI GetECodeField(
        /* [in] */ const String& name,
        /* [out] */ ECode* value);

    CARAPI GetLocalPtrField(
        /* [in] */ const String& name,
        /* [out] */ LocalPtr* value);

    CARAPI GetLocalTypeField(
        /* [in] */ const String& name,
        /* [out] */ PVoid value);

    CARAPI GetEnumField(
        /* [in] */ const String& name,
        /* [out] */ Int32* value);

    CARAPI GetStructFieldGetter(
        /* [in] */ const String& name,
        /* [out] */ IStructGetter** getter);

    CARAPI GetCppVectorFieldGetter(
        /* [in] */ const String& name,
        /* [out] */ ICppVectorGetter** getter);

    CARAPI GetFieldValueByName(
        /* [in] */ const String& name,
        /* [in] */ void* param,
        /* [in] */ CarDataType type);

    CARAPI GetIndexByName(
        /* [in] */ const String& name,
        /* [out] */ Int32* index);

    CARAPI AcquireCppVectorFieldSGetter(
        /* [in] */ const String& name,
        /* [in] */ Boolean isSetter,
        /* [out] */ IInterface** sGetter);

private:
    AutoPtr<IStructInfo>    mStructInfo;
    StructFieldDesc*        mStructFieldDesc;
    PByte                   mVarBuf;
    Boolean                 mIsAlloc;
    Int32                   mCount;
    UInt32                  mVarSize;

    IInterface**            mCppVectorSGetters;
};

#endif // __CSTRUCTVARIABLE_H__
