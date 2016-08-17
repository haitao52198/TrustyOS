//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CLOCALPTRINFO_H__
#define __CLOCALPTRINFO_H__

#include "CClsModule.h"

class CLocalPtrInfo
    : public ElLightRefBase
    , public ILocalPtrInfo
{
public:
    CLocalPtrInfo(
        /* [in] */ CClsModule* clsModule,
        /* [in] */ TypeDescriptor* typeDescriptor,
        /* [in] */ Int32 pointer);

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

    CARAPI GetTargetTypeInfo(
        /* [out] */ IDataTypeInfo** dateTypeInfo);

    CARAPI GetPtrLevel(
        /* [out] */ Int32* level);

private:
    AutoPtr<CClsModule> mClsModule;
    TypeDescriptor*     mTypeDescriptor;
    Int32               mPointer;
};

#endif // __CLOCALPTRINFO_H__
