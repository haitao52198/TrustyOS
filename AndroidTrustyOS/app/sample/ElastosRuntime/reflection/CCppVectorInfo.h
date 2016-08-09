//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CCPPVECTORINFO_H__
#define __CCPPVECTORINFO_H__

#include "CClsModule.h"

class CCppVectorInfo
    : public ElLightRefBase
    , public ICppVectorInfo
{
public:
    CCppVectorInfo(
        /* [in] */ IDataTypeInfo* elementTypeInfo,
        /* [in] */ Int32 length);

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

    CARAPI GetLength(
        /* [out] */ Int32* length);

    CARAPI GetMaxAlignSize(
        /* [out] */ MemorySize* alignSize);

private:
    AutoPtr<IDataTypeInfo>  mElementTypeInfo;
    Int32                   mLength;
    Int32                   mSize;
};

#endif // __CCPPVECTORINFO_H__
