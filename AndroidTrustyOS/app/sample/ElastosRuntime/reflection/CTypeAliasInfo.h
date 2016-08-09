//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CTYPEALIASINFO_H__
#define __CTYPEALIASINFO_H__

#include "CClsModule.h"

class CTypeAliasInfo
    : public ElLightRefBase
    , public ITypeAliasInfo
{
public:
    CTypeAliasInfo(
        /* [in] */ CClsModule* clsModule,
        /* [in] */ AliasDirEntry* aliasDirEntry);

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

    CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo** moduleInfo);

    CARAPI IsDummy(
        /* [out] */ Boolean* isDummy);

    CARAPI GetPtrLevel(
        /* [out] */ Int32* level);

private:
    AutoPtr<CClsModule> mClsModule;
    AliasDirEntry*      mAliasDirEntry;
};

#endif // __CTYPEALIASINFO_H__
