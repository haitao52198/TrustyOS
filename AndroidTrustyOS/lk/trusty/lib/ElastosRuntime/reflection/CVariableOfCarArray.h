//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CCARARRAYVARIABLE_H__
#define __CCARARRAYVARIABLE_H__

#include "CClsModule.h"

class CVariableOfCarArray
    : public ElLightRefBase
    , public IVariableOfCarArray
    , public ICarArraySetter
    , public ICarArrayGetter
{
public:
    CVariableOfCarArray(
        /* [in] */ ICarArrayInfo* typeInfo,
        /* [in] */ PCarQuintet varPtr,
        /* [in] */ Boolean isAlloc);

    virtual ~CVariableOfCarArray();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI GetTypeInfo(
        /* [out] */ IDataTypeInfo** typeInfo);

    CARAPI GetPayload(
        /* [out] */ PVoid* payload);

    CARAPI Rebox(
        /* [in] */ PVoid localVariablePtr);

    CARAPI GetSetter(
        /* [out] */ ICarArraySetter** setter);

    CARAPI GetGetter(
        /* [out] */ ICarArrayGetter** getter);

    CARAPI SetUsed(
        /* [in] */ Int32 used);

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

    CARAPI SetEnumElement(
        /* [in] */ Int32 index,
        /* [in] */ Int32 value);

    CARAPI SetCharElement(
        /* [in] */ Int32 index,
        /* [in] */ Char32 value);

    CARAPI SetStringElement(
        /* [in] */ Int32 index,
        /* [in] */ const String& value);

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

    CARAPI SetLocalTypeElement(
        /* [in] */ Int32 index,
        /* [in] */ PVoid value);

    CARAPI SetObjectPtrElement(
        /* [in] */ Int32 index,
        /* [in] */ PInterface value);

    CARAPI GetStructElementSetter(
        /* [in] */ Int32 index,
        /* [out] */ IStructSetter** setter);

    CARAPI GetCapacity(
        /* [out] */ Int32* capacity);

    CARAPI GetUsed(
        /* [out] */ Int32* used);

    CARAPI IsEmpty(
        /* [out] */ Boolean* isEmpty);

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

    CARAPI GetEnumElement(
        /* [in] */ Int32 index,
        /* [out] */ Int32* value);

    CARAPI GetCharElement(
        /* [in] */ Int32 index,
        /* [out] */ Char32* value);

    CARAPI GetStringElement(
        /* [in] */ Int32 index,
        /* [out] */ String* value);

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

    CARAPI GetLocalTypeElement(
        /* [in] */ Int32 index,
        /* [out] */ PVoid value);

    CARAPI GetObjectPtrElement(
        /* [in] */ Int32 index,
        /* [out] */ PInterface* value);

    CARAPI GetStructElementGetter(
        /* [in] */ Int32 index,
        /* [out] */ IStructGetter** getter);

    CARAPI SetElementValue(
        /* [in] */ Int32 index,
        /* [in] */ void* param,
        /* [in] */ CarDataType type);

    CARAPI GetElementValue(
        /* [in] */ Int32 index,
        /* [in] */ void* param,
        /* [in] */ CarDataType type);

private:
    AutoPtr<ICarArrayInfo>  mCarArrayInfo;
    PCarQuintet             mCq;
    Boolean                 mIsAlloc;
    Int32                   mElementSize;
    Int32                   mLength;

    CarDataType             mDataType;
};

#endif // __CCARARRAYVARIABLE_H__
