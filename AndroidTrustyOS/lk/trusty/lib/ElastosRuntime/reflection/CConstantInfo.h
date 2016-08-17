//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CCONSTANTINFO_H__
#define __CCONSTANTINFO_H__

#include "CClsModule.h"

class CConstantInfo
    : public ElLightRefBase
    , public IConstantInfo
{
public:
    CConstantInfo(
        /* [in] */ CClsModule* clsModule,
        /* [in] */ ConstDirEntry* constDirEntry);

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI GetName(
        /* [out] */ String* name);

    CARAPI GetValue(
        /* [out] */ Int32* value);

    CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo** moduleInfo);

private:
    AutoPtr<CClsModule> mClsModule;
    ConstDirEntry*      mConstDirEntry;
};

#endif // __CCONSTANTINFO_H__
