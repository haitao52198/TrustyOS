//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CVARIABLEOFCPPVECTOR_H__
#define __CVARIABLEOFCPPVECTOR_H__

#include "CClsModule.h"

class CVariableOfCppVector
    : public ElLightRefBase
    , public ICppVectorSetter
    , public ICppVectorGetter
{
public:
    CVariableOfCppVector(
        /* [in] */ IDataTypeInfo* elementTypeInfo,
        /* [in] */ Int32 length,
        /* [in] */ PVoid varPtr);

    virtual ~CVariableOfCppVector();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI Init();

//--------------Setter----------------------------------------------------------
    CARAPI ZeroAllElements();

    CARAPI SetAllElements(
        /* [in] */ PVoid value,
        /* [in] */ MemorySize size);

    CARAPI SetInt16Element(
        /* [in] */ Int32 index,
        /* [in] */ Int16 value);

    CARAPI SetInt32Element(
        /* [in] */ Int32 index,
        /* [in] */ Int32 value);

    CARAPI SetInt64Element(
        /* [in] */ Int32 index,
        /* [in] */ Int64 value);

    CARAPI SetByteElement(
        /* [in] */ Int32 index,
        /* [in] */ Byte value);

    CARAPI SetFloatElement(
        /* [in] */ Int32 index,
        /* [in] */ Float value);

    CARAPI SetDoubleElement(
        /* [in] */ Int32 index,
        /* [in] */ Double value);

    CARAPI SetCharElement(
        /* [in] */ Int32 index,
        /* [in] */ Char32 value);

    CARAPI SetBooleanElement(
        /* [in] */ Int32 index,
        /* [in] */ Boolean value);

    CARAPI SetEMuidElement(
        /* [in] */ Int32 index,
        /* [in] */ EMuid* value);

    CARAPI SetEGuidElement(
        /* [in] */ Int32 index,
        /* [in] */ EGuid* value);

    CARAPI SetECodeElement(
        /* [in] */ Int32 index,
        /* [in] */ ECode value);

    CARAPI SetLocalPtrElement(
        /* [in] */ Int32 index,
        /* [in] */ LocalPtr value);

    CARAPI SetLocalTypeElement(
        /* [in] */ Int32 index,
        /* [in] */ PVoid value);

    CARAPI SetEnumElement(
        /* [in] */ Int32 index,
        /* [in] */ Int32 value);

    CARAPI GetStructElementSetter(
        /* [in] */ Int32 index,
        /* [out] */ IStructSetter** setter);

    CARAPI GetCppVectorElementSetter(
        /* [in] */ Int32 index,
        /* [out] */ ICppVectorSetter** setter);

    CARAPI SetElementValue(
        /* [in] */ Int32 index,
        /* [in] */ void* param,
        /* [in] */ CarDataType type);

//--------------Getter----------------------------------------------------------

    CARAPI GetLength(
        /* [out] */ Int32* length);

    CARAPI GetRank(
        /* [out] */ Int32* rank);

    CARAPI GetInt16Element(
        /* [in] */ Int32 index,
        /* [out] */ Int16* value);

    CARAPI GetInt32Element(
        /* [in] */ Int32 index,
        /* [out] */ Int32* value);

    CARAPI GetInt64Element(
        /* [in] */ Int32 index,
        /* [out] */ Int64* value);

    CARAPI GetByteElement(
        /* [in] */ Int32 index,
        /* [out] */ Byte* value);

    CARAPI GetFloatElement(
        /* [in] */ Int32 index,
        /* [out] */ Float* value);

    CARAPI GetDoubleElement(
        /* [in] */ Int32 index,
        /* [out] */ Double* value);

    CARAPI GetCharElement(
        /* [in] */ Int32 index,
        /* [out] */ Char32* value);

    CARAPI GetBooleanElement(
        /* [in] */ Int32 index,
        /* [out] */ Boolean* value);

    CARAPI GetEMuidElement(
        /* [in] */ Int32 index,
        /* [out] */ EMuid* value);

    CARAPI GetEGuidElement(
        /* [in] */ Int32 index,
        /* [out] */ EGuid* value);

    CARAPI GetECodeElement(
        /* [in] */ Int32 index,
        /* [out] */ ECode* value);

    CARAPI GetLocalPtrElement(
        /* [in] */ Int32 index,
        /* [out] */ LocalPtr* value);

    CARAPI GetLocalTypeElement(
        /* [in] */ Int32 index,
        /* [out] */ PVoid value);

    CARAPI GetEnumElement(
        /* [in] */ Int32 index,
        /* [out] */ Int32* value);

    CARAPI GetStructElementGetter(
        /* [in] */ Int32 index,
        /* [out] */ IStructGetter** getter);

    CARAPI GetCppVectorElementGetter(
        /* [in] */ Int32 index,
        /* [out] */ ICppVectorGetter** getter);

    CARAPI GetElementValue(
        /* [in] */ Int32 index,
        /* [in] */ void* param,
        /* [in] */ CarDataType type);

    CARAPI GetSetter(
        /* [out] */ ICppVectorSetter** setter);

    CARAPI GetGetter(
        /* [out] */ ICppVectorGetter** getter);

    CARAPI Rebox(
        /* [in] */ PVoid varPtr);

    CARAPI AcquireCppVectorSGetter(
        /* [in] */ Int32 index,
        /* [in] */ Boolean isSetter,
        /* [out] */ IInterface** sGetter);

private:
    AutoPtr<IDataTypeInfo>  mElementTypeInfo;
    CarDataType             mDataType;
    PVoid                   mVarPtr;
    UInt32                  mSize;
    Int32                   mElementSize;
    Int32                   mLength;

    IInterface**            mCppVectorSGetters;
};

#endif // __CVARIABLEOFCPPVECTOR_H__
