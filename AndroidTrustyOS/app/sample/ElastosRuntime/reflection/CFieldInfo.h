//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CFIELDINFO_H__
#define __CFIELDINFO_H__

#include "refutil.h"

class CFieldInfo : public IFieldInfo
{
public:
    CFieldInfo(
        /* [in] */ IStructInfo* structInfo,
        /* [in] */ const String& name,
        /* [in] */ IDataTypeInfo* typeInfo);

    virtual ~CFieldInfo();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI GetName(
        /* [out] */ String* name);

    CARAPI GetTypeInfo(
        /* [out] */ IDataTypeInfo** typeInfo);

private:
    IStructInfo*    mStructInfo;
    String          mName;
    IDataTypeInfo*  mTypeInfo;
};

#endif // __CFIELDINFO_H__
