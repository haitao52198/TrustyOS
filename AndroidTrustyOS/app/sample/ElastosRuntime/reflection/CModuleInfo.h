//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CMODULEINFO_H__
#define __CMODULEINFO_H__

#include "CEntryList.h"

class CModuleInfo
    : public ElLightRefBase
    , public IModuleInfo
{
public:
    CModuleInfo(
        /* [in] */ CClsModule* clsModule,
        /* [in] */ const String& path);

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI GetPath(
        /* [out] */ String* path);

    CARAPI GetVersion(
        /* [out] */ Int32* major,
        /* [out] */ Int32* minor,
        /* [out] */ Int32* build,
        /* [out] */ Int32* revision);

    CARAPI GetClassCount(
        /* [out] */ Int32* count);

    CARAPI GetAllClassInfos(
        /* [out] */ ArrayOf<IClassInfo *>* classInfos);

    CARAPI GetClassInfo(
        /* [in] */ const String& fullName,
        /* [out] */ IClassInfo** classInfo);

    CARAPI GetInterfaceCount(
        /* [out] */ Int32* count);

    CARAPI GetAllInterfaceInfos(
        /* [out] */ ArrayOf<IInterfaceInfo *>* interfaceInfos);

    CARAPI GetInterfaceInfo(
        /* [in] */ const String& fullName,
        /* [out] */ IInterfaceInfo** interfaceInfo);

    CARAPI GetStructCount(
        /* [out] */ Int32* count);

    CARAPI GetAllStructInfos(
        /* [out] */ ArrayOf<IStructInfo *>* structInfos);

    CARAPI GetStructInfo(
        /* [in] */ const String& name,
        /* [out] */ IStructInfo** structInfo);

    CARAPI GetEnumCount(
        /* [out] */ Int32* count);

    CARAPI GetAllEnumInfos(
        /* [out] */ ArrayOf<IEnumInfo *>* enumInfos);

    CARAPI GetEnumInfo(
        /* [in] */ const String& fullName,
        /* [out] */ IEnumInfo** enumInfo);

    CARAPI GetTypeAliasCount(
        /* [out] */ Int32* count);

    CARAPI GetAllTypeAliasInfos(
        /* [out] */ ArrayOf<ITypeAliasInfo *>* typeAliasInfos);

    CARAPI GetTypeAliasInfo(
        /* [in] */ const String& name,
        /* [out] */ ITypeAliasInfo** typeAliasInfo);

    CARAPI GetConstantCount(
        /* [out] */ Int32* count);

    CARAPI GetAllConstantInfos(
        /* [out] */ ArrayOf<IConstantInfo *>* constantInfos);

    CARAPI GetConstantInfo(
        /* [in] */ const String& name,
        /* [out] */ IConstantInfo** constantInfo);

    CARAPI GetImportModuleInfoCount(
        /* [out] */ Int32* count);

    CARAPI GetAllImportModuleInfos(
        /* [out] */ ArrayOf<IModuleInfo *>* moduleInfos);

    CARAPI AcquireClassList();

    CARAPI AcquireInterfaceList();

    CARAPI AcquireStructList();

    CARAPI AcquireEnumList();

    CARAPI AcquireAliasList();

    CARAPI AcquireConstantList();

public:
    AutoPtr<CClsModule> mClsModule;
    CLSModule*          mClsMod;

private:
    AutoPtr<CEntryList> mClassList;
    AutoPtr<CEntryList> mInterfaceList;
    AutoPtr<CEntryList> mStructList;
    AutoPtr<CEntryList> mEnumList;
    AutoPtr<CEntryList> mAliasList;
    AutoPtr<CEntryList> mConstantList;

    String              mPath;
    Int32               mAliasCount;
};

#endif // __CMODULEINFO_H__
