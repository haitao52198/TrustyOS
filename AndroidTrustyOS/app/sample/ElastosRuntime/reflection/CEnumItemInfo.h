//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CENUMITEMINFO_H__
#define __CENUMITEMINFO_H__

#include "refutil.h"

class CEnumItemInfo : public IEnumItemInfo
{
public:
    CEnumItemInfo(
        /* [in] */ IEnumInfo* enumInfo,
        /* [in] */ const String& name,
        /* [in] */ Int32 value);

    virtual ~CEnumItemInfo();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI GetName(
        /* [out] */ String* name);

    CARAPI GetEnumInfo(
        /* [out] */ IEnumInfo** enumInfo);

    CARAPI GetValue(
        /* [out] */ Int32* value);

private:
    IEnumInfo*  mEnumInfo;
    String      mName;
    Int32       mValue;
};

#endif // __CENUMITEMINFO_H__
