//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CENUMINFO_H__
#define __CENUMINFO_H__

#include "CEntryList.h"

class CEnumInfo
    : public ElLightRefBase
    , public IEnumInfo
{
public:
    CEnumInfo();

    virtual ~CEnumInfo();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI InitStatic(
        /* [in] */ CClsModule* clsModule,
        /* [in] */ EnumDirEntry* enumDirEntry);

    CARAPI InitDynamic(
        /* [in] */ const String& fullName,
        /* [in] */ ArrayOf<String>* itemNames,
        /* [in] */ ArrayOf<Int32>* itemValues);

    CARAPI GetSize(
        /* [out] */ MemorySize* size);

    CARAPI GetDataType(
        /* [out] */ CarDataType* dataType);

    CARAPI GetName(
        /* [out] */ String* name);

    CARAPI GetNamespace(
        /* [out] */ String* ns);

    CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo** moduleInfo);

    CARAPI GetItemCount(
        /* [out] */ Int32* count);

    CARAPI GetAllItemInfos(
        /* [out] */ ArrayOf<IEnumItemInfo *>* itemInfos);

    CARAPI GetItemInfo(
        /* [in] */ const String& name,
        /* [out] */ IEnumItemInfo** enumItemInfo);

    CARAPI InitItemInfos();

public:
    AutoPtr< ArrayOf<String> >  mItemNames;
    AutoPtr< ArrayOf<Int32> >   mItemValues;

private:
    AutoPtr<CClsModule>         mClsModule;
    EnumDirEntry*               mEnumDirEntry;
    ArrayOf<IEnumItemInfo *>*   mItemInfos;

    String                      mName;
    String                      mNamespace;
};

#endif // __CENUMINFO_H__
