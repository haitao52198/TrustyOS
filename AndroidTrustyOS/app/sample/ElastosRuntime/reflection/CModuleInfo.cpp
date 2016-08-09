//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================
#include "CModuleInfo.h"
// #include "_pubcrt.h"
#include <utils/Log.h>

CModuleInfo::CModuleInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ const String& path)
{
    mClsModule = clsModule;
    mClsMod = clsModule->mClsMod;
    mPath = path;
    mAliasCount = 0;
}

UInt32 CModuleInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CModuleInfo::Release()
{
    g_objInfoList.LockHashTable(EntryType_Module);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        g_objInfoList.RemoveModuleInfo(mPath);
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_Module);
    assert(ref >= 0);
    return ref;
}

PInterface CModuleInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IModuleInfo) {
        return (IModuleInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CModuleInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CModuleInfo::GetPath(
    /* [out] */ String* path)
{
    *path = mPath;
    return NOERROR;
}

ECode CModuleInfo::GetVersion(
    /* [out] */ Int32* major,
    /* [out] */ Int32* minor,
    /* [out] */ Int32* build,
    /* [out] */ Int32* revision)
{
    *major = mClsMod->mMajorVersion;
    *minor = mClsMod->mMinorVersion;
    return NOERROR;
}

ECode CModuleInfo::GetClassCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mClsMod->mClassCount;
    return NOERROR;
}

ECode CModuleInfo::AcquireClassList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_Class);
    if (!mClassList) {
        mClassList = new CEntryList(EntryType_Class,
                mClsMod, mClsMod->mClassCount, mClsModule);
        if (!mClassList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_Class);

    return ec;
}

ECode CModuleInfo::GetAllClassInfos(
    /* [out] */ ArrayOf<IClassInfo *>* classInfos)
{
    ECode ec = AcquireClassList();
    if (FAILED(ec)) return ec;

    return mClassList->GetAllObjInfos((PTypeInfos)classInfos);
}

ECode CModuleInfo::GetClassInfo(
    /* [in] */ const String& fullName,
    /* [out] */ IClassInfo** classInfo)
{
    if (fullName.IsNull() || NULL == classInfo) {
        return E_INVALID_ARGUMENT;
    }

    ECode ec = AcquireClassList();
    if (FAILED(ec)) return ec;

    if (fullName.IndexOf(';') >= 0 || fullName.IndexOf('/') >= 0) {
        ALOGE("GetClassInfo fullName = %s is invalid", fullName.string());
        assert(0);
    }
    return mClassList->AcquireObjByName(fullName, (IInterface **)classInfo);
}

ECode CModuleInfo::GetInterfaceCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mClsMod->mInterfaceCount;
    return NOERROR;
}

ECode CModuleInfo::AcquireInterfaceList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_Interface);
    if (!mInterfaceList) {
        mInterfaceList = new CEntryList(EntryType_Interface,
                mClsMod, mClsMod->mInterfaceCount, mClsModule);
        if (!mInterfaceList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_Interface);

    return ec;
}

ECode CModuleInfo::GetAllInterfaceInfos(
    /* [out] */ ArrayOf<IInterfaceInfo *>* interfaceInfos)
{
    ECode ec = AcquireInterfaceList();
    if (FAILED(ec)) return ec;

    return mInterfaceList->GetAllObjInfos((PTypeInfos)interfaceInfos);
}

ECode CModuleInfo::GetInterfaceInfo(
    /* [in] */ const String& fullName,
    /* [out] */ IInterfaceInfo** interfaceInfo)
{
    if (fullName.IsNull() || NULL == interfaceInfo) {
        return E_INVALID_ARGUMENT;
    }

    ECode ec = AcquireInterfaceList();
    if (FAILED(ec)) return ec;

    if (fullName.IndexOf(';') >= 0 || fullName.IndexOf('/') >= 0) {
        ALOGE("GetInterfaceInfo fullName = %s is invalid", fullName.string());
        assert(0);
    }
    return mInterfaceList->AcquireObjByName(fullName, (IInterface **)interfaceInfo);
}

ECode CModuleInfo::GetStructCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mClsMod->mStructCount;
    return NOERROR;
}

ECode CModuleInfo::AcquireStructList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_Struct);
    if (!mStructList) {
        mStructList = new CEntryList(EntryType_Struct,
                mClsMod, mClsMod->mStructCount, mClsModule);
        if (!mStructList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_Struct);

    return ec;
}

ECode CModuleInfo::GetAllStructInfos(
    /* [out] */ ArrayOf<IStructInfo *>* structInfos)
{
    ECode ec = AcquireStructList();
    if (FAILED(ec)) return ec;

    return mStructList->GetAllObjInfos((PTypeInfos)structInfos);
}

ECode CModuleInfo::GetStructInfo(
    /* [in] */ const String& name,
    /* [out] */ IStructInfo** structInfo)
{
    if (name.IsNull() || !structInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!mClsMod->mStructCount) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec = AcquireStructList();
    if (FAILED(ec)) return ec;

    return mStructList->AcquireObjByName(name, (IInterface **)structInfo);
}

ECode CModuleInfo::GetEnumCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mClsMod->mEnumCount;
    return NOERROR;
}

ECode CModuleInfo::AcquireEnumList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_Enum);
    if (!mEnumList) {
        mEnumList = new CEntryList(EntryType_Enum,
                mClsMod, mClsMod->mEnumCount, mClsModule);
        if (!mEnumList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_Enum);

    return ec;
}

ECode CModuleInfo::GetAllEnumInfos(
    /* [out] */ ArrayOf<IEnumInfo *>* enumInfos)
{
    ECode ec = AcquireEnumList();
    if (FAILED(ec)) return ec;

    return mEnumList->GetAllObjInfos((PTypeInfos)enumInfos);
}

ECode CModuleInfo::GetEnumInfo(
    /* [in] */ const String& fullName,
    /* [out] */ IEnumInfo** enumInfo)
{
    if (fullName.IsNull() || !enumInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!mClsMod->mEnumCount) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec = AcquireEnumList();
    if (FAILED(ec)) return ec;

    if (fullName.IndexOf(';') >= 0 || fullName.IndexOf('/') >= 0) {
        ALOGE("GetEnumInfo fullName = %s is invalid", fullName.string());
        assert(0);
    }
    return mEnumList->AcquireObjByName(fullName, (IInterface **)enumInfo);
}

ECode CModuleInfo::GetTypeAliasCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    ECode ec = AcquireAliasList();
    if (FAILED(ec)) return ec;

    *count = mAliasCount;
    return NOERROR;
}

ECode CModuleInfo::AcquireAliasList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_TypeAliase);
    if (!mAliasList) {
        mAliasCount = 0;
        for (Int32 i = 0; i < mClsMod->mAliasCount; i++) {
            if (!IsSysAlaisType(mClsModule, i)) mAliasCount++;
        }

        mAliasList = new CEntryList(EntryType_TypeAliase,
                mClsMod, mAliasCount, mClsModule);
        if (!mAliasList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_TypeAliase);

    return ec;
}

ECode CModuleInfo::GetAllTypeAliasInfos(
    /* [out] */ ArrayOf<ITypeAliasInfo *>* typeAliasInfos)
{
    ECode ec = AcquireAliasList();
    if (FAILED(ec)) return ec;

    return mAliasList->GetAllObjInfos((PTypeInfos)typeAliasInfos);
}

ECode CModuleInfo::GetTypeAliasInfo(
    /* [in] */ const String& name,
    /* [out] */ ITypeAliasInfo** typeAliasInfo)
{
    if (name.IsNull() || !typeAliasInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!mClsMod->mAliasCount) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec = AcquireAliasList();
    if (FAILED(ec)) return ec;

    return mAliasList->AcquireObjByName(name, (IInterface **)typeAliasInfo);
}

ECode CModuleInfo::GetConstantCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mClsMod->mConstCount;
    return NOERROR;
}

ECode CModuleInfo::AcquireConstantList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_Constant);
    if (!mConstantList) {
        mConstantList = new CEntryList(EntryType_Constant,
                mClsMod, mClsMod->mConstCount, mClsModule);
        if (!mConstantList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_Constant);

    return ec;
}

ECode CModuleInfo::GetAllConstantInfos(
    /* [out] */ ArrayOf<IConstantInfo *>* constantInfos)
{
    ECode ec = AcquireConstantList();
    if (FAILED(ec)) return ec;

    return mConstantList->GetAllObjInfos((PTypeInfos)constantInfos);
}

ECode CModuleInfo::GetConstantInfo(
    /* [in] */ const String& name,
    /* [out] */ IConstantInfo** constantInfo)
{
    if (name.IsNull() || !constantInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!mClsMod->mConstCount) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec = AcquireConstantList();
    if (FAILED(ec)) return ec;

    return mConstantList->AcquireObjByName(name, (IInterface **)constantInfo);
}

ECode CModuleInfo::GetImportModuleInfoCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mClsMod->mLibraryCount;
    return NOERROR;
}

ECode CModuleInfo::GetAllImportModuleInfos(
    /* [out] */ ArrayOf<IModuleInfo *>* moduleInfos)
{
    if (!moduleInfos) {
        return E_INVALID_ARGUMENT;
    }

    Int32 capacity = moduleInfos->GetLength();
    if (!capacity) {
        return E_INVALID_ARGUMENT;
    }

    Int32 totalCount = mClsMod->mLibraryCount;
    if (!totalCount) {
        return NOERROR;
    }

    Int32 count = capacity < totalCount ? capacity : totalCount;
    ECode ec = NOERROR;
    for (Int32 i = 0; i < count; i++) {
        String libNames(getLibNameAddr(mClsModule->mBase, mClsMod->mLibraryNames, i));
        AutoPtr<IModuleInfo> object;
        ec = g_objInfoList.AcquireModuleInfo(libNames, (IModuleInfo**)&object);
        if (FAILED(ec)) return ec;
        moduleInfos->Set(i, object);
    }

    return NOERROR;
}
