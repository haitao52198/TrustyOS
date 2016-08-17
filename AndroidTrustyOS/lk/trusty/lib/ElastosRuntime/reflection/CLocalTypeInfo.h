//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CLOCALTYPEINFO_H__
#define __CLOCALTYPEINFO_H__

#include "CClsModule.h"

class CLocalTypeInfo
    : public ElLightRefBase
    , public IDataTypeInfo
{
public:
    CLocalTypeInfo(
        /* [in] */ MemorySize size);

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

private:
    MemorySize mSize;
};

#endif // __CLOCALTYPEINFO_H__
