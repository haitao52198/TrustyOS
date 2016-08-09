//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CARGLIST_H__
#define __CARGLIST_H__

#include "refutil.h"

class CArgumentList
    : public ElLightRefBase
    , public IArgumentList
{
public:
    CArgumentList();

    virtual ~CArgumentList();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI Init(
        /* [in] */ IFunctionInfo* functionInfo,
        /* [in] */ ParmElement* paramElem,
        /* [in] */ UInt32 paramCount,
        /* [in] */ UInt32 paramBufSize,
        /* [in] */ Boolean methodInfo);

    CARAPI GetFunctionInfo(
        /* [out] */ IFunctionInfo** functionInfo);

    CARAPI SetInputArgumentOfInt16(
        /* [in] */ Int32 index,
        /* [in] */ Int16 value);

    CARAPI SetInputArgumentOfInt32(
        /* [in] */ Int32 index,
        /* [in] */ Int32 value);

    CARAPI SetInputArgumentOfInt64(
        /* [in] */ Int32 index,
        /* [in] */ Int64 value);

    CARAPI SetInputArgumentOfByte(
        /* [in] */ Int32 index,
        /* [in] */ Byte value);

    CARAPI SetInputArgumentOfFloat(
        /* [in] */ Int32 index,
        /* [in] */ Float value);

    CARAPI SetInputArgumentOfDouble(
        /* [in] */ Int32 index,
        /* [in] */ Double value);

    CARAPI SetInputArgumentOfChar(
        /* [in] */ Int32 index,
        /* [in] */ Char32 value);

    CARAPI SetInputArgumentOfString(
        /* [in] */ Int32 index,
        /* [in] */ const String& value);

    CARAPI SetInputArgumentOfBoolean(
        /* [in] */ Int32 index,
        /* [in] */ Boolean value);

    CARAPI SetInputArgumentOfEMuid(
        /* [in] */ Int32 index,
        /* [in] */ EMuid* value);

    CARAPI SetInputArgumentOfEGuid(
        /* [in] */ Int32 index,
        /* [in] */ EGuid* value);

    CARAPI SetInputArgumentOfECode(
        /* [in] */ Int32 index,
        /* [in] */ ECode value);

    CARAPI SetInputArgumentOfLocalPtr(
        /* [in] */ Int32 index,
        /* [in] */ LocalPtr value);

    CARAPI SetInputArgumentOfLocalType(
        /* [in] */ Int32 index,
        /* [in] */ PVoid value);

    CARAPI SetInputArgumentOfEnum(
        /* [in] */ Int32 index,
        /* [in] */ Int32 value);

    CARAPI SetInputArgumentOfCarArray(
        /* [in] */ Int32 index,
        /* [in] */ PCarQuintet value);

    CARAPI SetInputArgumentOfStructPtr(
        /* [in] */ Int32 index,
        /* [in] */ PVoid value);

    CARAPI SetInputArgumentOfObjectPtr(
        /* [in] */ Int32 index,
        /* [in] */ PInterface value);

    CARAPI SetOutputArgumentOfInt16Ptr(
        /* [in] */ Int32 index,
        /* [out] */ Int16* value);

    CARAPI SetOutputArgumentOfInt32Ptr(
        /* [in] */ Int32 index,
        /* [out] */ Int32* value);

    CARAPI SetOutputArgumentOfInt64Ptr(
        /* [in] */ Int32 index,
        /* [out] */ Int64* value);

    CARAPI SetOutputArgumentOfBytePtr(
        /* [in] */ Int32 index,
        /* [out] */ Byte* value);

    CARAPI SetOutputArgumentOfFloatPtr(
        /* [in] */ Int32 index,
        /* [out] */ Float* value);

    CARAPI SetOutputArgumentOfDoublePtr(
        /* [in] */ Int32 index,
        /* [out] */ Double* value);

    CARAPI SetOutputArgumentOfCharPtr(
        /* [in] */ Int32 index,
        /* [out] */ Char32* value);

    CARAPI SetOutputArgumentOfStringPtr(
        /* [in] */ Int32 index,
        /* [out] */ String* value);

    CARAPI SetOutputArgumentOfBooleanPtr(
        /* [in] */ Int32 index,
        /* [out] */ Boolean* value);

    CARAPI SetOutputArgumentOfEMuidPtr(
        /* [in] */ Int32 index,
        /* [out] */ EMuid* value);

    CARAPI SetOutputArgumentOfEGuidPtr(
        /* [in] */ Int32 index,
        /* [out] */ EGuid* value);

    CARAPI SetOutputArgumentOfECodePtr(
        /* [in] */ Int32 index,
        /* [out] */ ECode* value);

    CARAPI SetOutputArgumentOfLocalPtrPtr(
        /* [in] */ Int32 index,
        /* [out] */ LocalPtr* value);

    CARAPI SetOutputArgumentOfLocalTypePtr(
        /* [in] */ Int32 index,
        /* [out] */ PVoid value);

    CARAPI SetOutputArgumentOfEnumPtr(
        /* [in] */ Int32 index,
        /* [out] */ Int32* value);

    CARAPI SetOutputArgumentOfCarArrayPtr(
        /* [in] */ Int32 index,
        /* [out] */ PCarQuintet value);

    CARAPI SetOutputArgumentOfCarArrayPtrPtr(
        /* [in] */ Int32 index,
        /* [out] */ PCarQuintet* value);

    CARAPI SetOutputArgumentOfStructPtr(
        /* [in] */ Int32 index,
        /* [out] */ PVoid value);

    CARAPI SetOutputArgumentOfStructPtrPtr(
        /* [in] */ Int32 index,
        /* [out] */ PVoid* value);

    CARAPI SetOutputArgumentOfObjectPtrPtr(
        /* [in] */ Int32 index,
        /* [out] */ PInterface* value);

    CARAPI SetParamValue(
        /* [in] */ Int32 index,
        /* [in] */ PVoid param,
        /* [in] */ CarDataType type,
        /* [in] */ ParamIOAttribute attrib,
        /* [in] */ Int32 pointer = 0);

public:
    Byte*           mParamBuf;

private:
    ParmElement*    mParamElem;
    UInt32          mParamCount;
    UInt32          mParamBufSize;
    AutoPtr<IFunctionInfo>  mFunctionInfo;
    Boolean         mIsMethodInfo;
};

#endif // __CARGLIST_H__
