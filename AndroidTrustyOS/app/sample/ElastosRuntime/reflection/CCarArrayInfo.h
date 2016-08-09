//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CCARARRAYINFO_H__
#define __CCARARRAYINFO_H__

#include "CClsModule.h"

class CCarArrayInfo
    : public ElLightRefBase
    , public ICarArrayInfo
{
public:
    CCarArrayInfo(
        /* [in] */ CarDataType quintetType,
        /* [in] */ IDataTypeInfo* elementTypeInfo,
        /* [in] */ CarDataType dataType = CarDataType_Struct);

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI GetName(
        /* [out] */ String* name);

    CARAPI GetSize(
        /* [out] */ MemorySize* size);

    CARAPI GetDataType(
        /* [out] */ CarDataType* dataType);

    CARAPI GetElementTypeInfo(
        /* [out] */ IDataTypeInfo** elementTypeInfo);

    CARAPI CreateVariable(
        /* [in] */ Int32 capacity,
        /* [out] */ IVariableOfCarArray** variable);

    CARAPI CreateVariableBox(
        /* [in] */ PCarQuintet variableDescriptor,
        /* [out] */ IVariableOfCarArray** variable);

    CARAPI GetMaxAlignSize(
        /* [out] */ MemorySize* alignSize);

private:
    AutoPtr<IDataTypeInfo>  mElementTypeInfo;
    CarDataType             mElementDataType;
    CarDataType             mQuintetType;
};

#endif // __CCARARRAYINFO_H__
