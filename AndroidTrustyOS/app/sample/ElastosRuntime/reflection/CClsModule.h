//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CCLSMODULE_H__
#define __CCLSMODULE_H__

#include "refutil.h"

class CClsModule : public ElLightRefBase
{
public:
    CClsModule(
        /* [in] */ CLSModule* clsMod,
        /* [in] */ Boolean allocedClsMod,
        /* [in] */ const String& path,
        /* [in] */ Void* module);

    ~CClsModule();

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo** moduleInfo);

    CARAPI InitOrgType();

    CARAPI AliasToOriginal(
        /* [in] */ TypeDescriptor* typeDype,
        /* [out] */ TypeDescriptor** orgTypeDesc);

public:
    CLSModule*      mClsMod;
    Boolean         mAllocedClsMode;
    Int32           mBase;

private:
    TypeAliasDesc*  mTypeAliasList;
    String          mPath;
    Void*           mModule;
};

#endif // __CCLSMODULE_H__
