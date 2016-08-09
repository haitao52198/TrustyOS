//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CObjInfoList.h"

#include "CModuleInfo.h"
#include "CClassInfo.h"
#include "CInterfaceInfo.h"
#include "CStructInfo.h"
#include "CEnumInfo.h"
#include "CTypeAliasInfo.h"
#include "CConstructorInfo.h"
#include "CIntrinsicInfo.h"
#include "CCppVectorInfo.h"
#include "CCarArrayInfo.h"
#include "CConstantInfo.h"
#include "CConstructorInfo.h"
#include "CLocalTypeInfo.h"
#include "CLocalPtrInfo.h"
#include "CCallbackMethodInfo.h"
#include <pthread.h>
#include <dlfcn.h>

typedef
struct ModuleRsc {
    unsigned int    mSize;
    const char      mClsinfo[0];
} ModuleRsc;

int dlGetClassInfo(void* handle, void** address, int* size)
{
    ModuleRsc** rsc = NULL;

    assert(NULL != address);
    assert(NULL != size);

    rsc = (ModuleRsc **)dlsym(handle, "g_pDllResource");
    if (NULL == rsc) {
        goto E_FAIL_EXIT;
    }

    *address = (void*)&((*rsc)->mClsinfo[0]);
    *size = (*rsc)->mSize;
    return 0;

E_FAIL_EXIT:
    return -1;
}

CObjInfoList::CObjInfoList()
{
    pthread_mutexattr_t recursiveAttr;

    pthread_mutexattr_init(&recursiveAttr);
    pthread_mutexattr_settype(&recursiveAttr, PTHREAD_MUTEX_RECURSIVE);
    Int32 ret = pthread_mutex_init(&mLockTypeAlias, &recursiveAttr);
    if (ret) mIsLockTypeAlias = FALSE;
    else mIsLockTypeAlias = TRUE;

    ret = pthread_mutex_init(&mLockEnum, &recursiveAttr);
    if (ret) mIsLockEnum = FALSE;
    else mIsLockEnum = TRUE;

    ret = pthread_mutex_init(&mLockClass, &recursiveAttr);
    if (ret) mIsLockClass = FALSE;
    else mIsLockClass = TRUE;

    ret = pthread_mutex_init(&mLockStruct, &recursiveAttr);
    if (ret) mIsLockStruct = FALSE;
    else mIsLockStruct = TRUE;

    ret = pthread_mutex_init(&mLockMethod, &recursiveAttr);
    if (ret) mIsLockMethod = FALSE;
    else mIsLockMethod = TRUE;

    ret = pthread_mutex_init(&mLockInterface, &recursiveAttr);
    if (ret) mIsLockInterface = FALSE;
    else mIsLockInterface = TRUE;

    ret = pthread_mutex_init(&mLockModule, &recursiveAttr);
    if (ret) mIsLockModule = FALSE;
    else mIsLockModule = TRUE;

    ret = pthread_mutex_init(&mLockDataType, &recursiveAttr);
    if (ret) mIsLockDataType = FALSE;
    else mIsLockDataType = TRUE;

    ret = pthread_mutex_init(&mLockLocal, &recursiveAttr);
    if (ret) mIsLockLocal = FALSE;
    else mIsLockLocal = TRUE;

    ret = pthread_mutex_init(&mLockClsModule, &recursiveAttr);
    if (ret) mIsLockClsModule = FALSE;
    else mIsLockClsModule = TRUE;

    pthread_mutexattr_destroy(&recursiveAttr);

    memset(mDataTypeInfos, 0, sizeof(IInterface *) * MAX_ITEM_COUNT);
    memset(mLocalTypeInfos, 0, sizeof(IInterface *) * MAX_ITEM_COUNT);
    memset(mCarArrayInfos, 0, sizeof(IInterface *) * MAX_ITEM_COUNT);

    mEnumInfoHead = NULL;
    mCCppVectorHead = NULL;
    mStructInfoHead = NULL;
    mCarArrayInfoHead = NULL;
}

CObjInfoList::~CObjInfoList()
{
    for (Int32 i = 0; i < MAX_ITEM_COUNT; i++) {
        if (mDataTypeInfos[i]) {
            mDataTypeInfos[i]->Release();
        }
        if (mLocalTypeInfos[i]) {
            mLocalTypeInfos[i]->Release();
        }
        if (mCarArrayInfos[i]) {
            mCarArrayInfos[i]->Release();
        }
    }

    mTypeAliasInfos.Clear();
    mEnumInfos.Clear();
    mClassInfos.Clear();
    mStructInfos.Clear();
    mMethodInfos.Clear();
    mIFInfos.Clear();
    mModInfos.Clear();
    mLocalPtrInfos.Clear();
    mClsModule.Clear();

    pthread_mutex_destroy(&mLockTypeAlias);
    pthread_mutex_destroy(&mLockEnum);
    pthread_mutex_destroy(&mLockClass);
    pthread_mutex_destroy(&mLockStruct);
    pthread_mutex_destroy(&mLockMethod);
    pthread_mutex_destroy(&mLockInterface);
    pthread_mutex_destroy(&mLockModule);
    pthread_mutex_destroy(&mLockDataType);
    pthread_mutex_destroy(&mLockLocal);
    pthread_mutex_destroy(&mLockClsModule);
}

UInt32 CObjInfoList::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CObjInfoList::Release()
{
    return ElLightRefBase::Release();
}

ECode CObjInfoList::LockHashTable(
    /* [in] */ EntryType type)
{
    switch (type) {
        case EntryType_Module:
            if (mIsLockModule)
                pthread_mutex_lock(&mLockModule);
            break;
        case EntryType_Aspect:
        case EntryType_Aggregatee:
        case EntryType_Class:
            if (mIsLockClass)
                pthread_mutex_lock(&mLockClass);
            break;
        case EntryType_ClassInterface:
        case EntryType_Interface:
            if (mIsLockInterface)
                pthread_mutex_lock(&mLockInterface);
            break;
        case EntryType_Struct:
            if (mIsLockStruct)
                pthread_mutex_lock(&mLockStruct);
            break;
        case EntryType_Constant:
        case EntryType_Enum:
            if (mIsLockEnum)
                pthread_mutex_lock(&mLockEnum);
            break;
        case EntryType_TypeAliase:
            if (mIsLockTypeAlias)
                pthread_mutex_lock(&mLockTypeAlias);
            break;
        case EntryType_Constructor:
        case EntryType_CBMethod:
        case EntryType_Method:
            if (mIsLockMethod)
                pthread_mutex_lock(&mLockMethod);
            break;
        case EntryType_DataType:
            if (mIsLockDataType)
                pthread_mutex_lock(&mLockDataType);
            break;
        case EntryType_Local:
            if (mIsLockLocal)
                pthread_mutex_lock(&mLockLocal);
            break;
        case EntryType_ClsModule:
            if (mIsLockClsModule)
                pthread_mutex_lock(&mLockClsModule);
            break;

        default:
            break;
    }

    return NOERROR;
}

ECode CObjInfoList::UnlockHashTable(
    /* [in] */ EntryType type)
{
    switch (type) {
        case EntryType_Module:
            if (mIsLockModule)
                pthread_mutex_unlock(&mLockModule);
            break;
        case EntryType_Aspect:
        case EntryType_Aggregatee:
        case EntryType_Class:
            if (mIsLockClass)
                pthread_mutex_unlock(&mLockClass);
            break;
        case EntryType_ClassInterface:
        case EntryType_Interface:
            if (mIsLockInterface)
                pthread_mutex_unlock(&mLockInterface);
            break;
        case EntryType_Struct:
            if (mIsLockStruct)
                pthread_mutex_unlock(&mLockStruct);
            break;
        case EntryType_Constant:
        case EntryType_Enum:
            if (mIsLockEnum)
                pthread_mutex_unlock(&mLockEnum);
            break;
        case EntryType_TypeAliase:
            if (mIsLockTypeAlias)
                pthread_mutex_unlock(&mLockTypeAlias);
            break;
        case EntryType_Constructor:
        case EntryType_CBMethod:
        case EntryType_Method:
            if (mIsLockMethod)
                pthread_mutex_unlock(&mLockMethod);
            break;
        case EntryType_DataType:
            if (mIsLockDataType)
                pthread_mutex_unlock(&mLockDataType);
            break;
        case EntryType_Local:
            if (mIsLockLocal)
                pthread_mutex_unlock(&mLockLocal);
            break;
        case EntryType_ClsModule:
            if (mIsLockClsModule)
                pthread_mutex_unlock(&mLockClsModule);
            break;

        default:
            return E_INVALID_ARGUMENT;
    }

    return NOERROR;
}

ECode CObjInfoList::AcquireModuleInfo(
    /* [in] */ const String& name,
    /* [out] */ IModuleInfo** moduleInfo)
{
    if (name.IsNull() || !moduleInfo) {
        return E_INVALID_ARGUMENT;
    }

    ECode ec = NOERROR;

    void* module = dlopen(name.string(), RTLD_NOW);
    if(NULL == module){
        return E_FILE_NOT_FOUND;
    }

    LockHashTable(EntryType_Module);
    IModuleInfo** modInfo = mModInfos.Get(const_cast<char*>(name.string()));
    if (modInfo) {
        dlclose(module);
        *moduleInfo = *modInfo;
        (*moduleInfo)->AddRef();
        UnlockHashTable(EntryType_Module);
        return NOERROR;
    }

    AutoPtr<CModuleInfo> modInfoObj;
    CClsModule* clsModule = NULL;
    IModuleInfo* iModInfo = NULL;

    LockHashTable(EntryType_ClsModule);

    CClsModule** ppCClsModule = mClsModule.Get(const_cast<char*>(name.string()));
    if (ppCClsModule) {
        clsModule = *ppCClsModule;
    }

    if (!clsModule) {
        ppCClsModule = NULL;
        //get cls module metadata
        MemorySize size;
        void* lockRes;
        CLSModule* clsMod;
        Boolean isAllocedClsMod = FALSE;

        if (-1 == dlGetClassInfo(module, &lockRes, &size)) {
            ec = E_DOES_NOT_EXIST;
            goto Exit;
        }

        if (((CLSModule *)lockRes)->mAttribs & CARAttrib_compress) {
            if (RelocFlattedCLS((CLSModule *)lockRes, size, &clsMod) < 0) {
                ec = E_OUT_OF_MEMORY;
                goto Exit;
            }
            isAllocedClsMod = TRUE;
        }
        else {
            clsMod = (CLSModule *)lockRes;
        }

        clsModule = new CClsModule(clsMod, isAllocedClsMod, name, module);
        if (clsModule == NULL) {
            if (isAllocedClsMod) DisposeFlattedCLS(clsMod);
            ec = E_OUT_OF_MEMORY;
            goto Exit;
        }

        if (!mClsModule.Put(const_cast<char*>(name.string()), (CClsModule**)&clsModule)) {
            delete clsModule;
            ec = E_OUT_OF_MEMORY;
            goto Exit;
        }
    }

    modInfoObj = new CModuleInfo(clsModule, name);
    if (modInfoObj == NULL) {
        if (!ppCClsModule && clsModule) delete clsModule;
        ec = E_OUT_OF_MEMORY;
        goto Exit;
    }

    iModInfo = (IModuleInfo*)modInfoObj.Get();
    if (!g_objInfoList.mModInfos.Put(const_cast<char*>(name.string()),
            (IModuleInfo**)&iModInfo)) {
        ec = E_OUT_OF_MEMORY;
        goto Exit;
    }

    *moduleInfo = iModInfo;
    (*moduleInfo)->AddRef();

    ec = NOERROR;

Exit:
    UnlockHashTable(EntryType_ClsModule);
    UnlockHashTable(EntryType_Module);

    return ec;
}

ECode CObjInfoList:: RemoveModuleInfo(
    /* [in] */ const String& path)
{
    if (path.IsNull()) {
        return E_INVALID_ARGUMENT;
    }
    mModInfos.Remove((PVoid)path.string());
    return NOERROR;
}

ECode CObjInfoList:: RemoveClsModule(
    /* [in] */ const String& path)
{
    if (path.IsNull()) {
        return E_INVALID_ARGUMENT;
    }
    mClsModule.Remove((PVoid)path.string());
    return NOERROR;
}

ECode CObjInfoList::AcquireClassInfo(
    /* [in] */ CClsModule * clsModule,
    /* [in] */ ClassDirEntry* clsDirEntry,
    /* [in, out] */ IInterface** object)
{
    if (!clsModule || !clsDirEntry || !object) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_Class);
    if (*object) {
        UnlockHashTable(EntryType_Class);
        return NOERROR;
    }

    IInterface** obj = mClassInfos.Get(&clsDirEntry);
    if (!obj) {
        IInterface *interfaceObj = NULL;
        AutoPtr<CClassInfo> classInfoObj = new CClassInfo(clsModule, clsDirEntry);
        if (classInfoObj == NULL) {
            UnlockHashTable(EntryType_Class);
            return E_OUT_OF_MEMORY;
        }

        ECode ec = classInfoObj->Init();
        if (FAILED(ec)) {
            UnlockHashTable(EntryType_Class);
            return ec;
        }

        interfaceObj = (IInterface*)classInfoObj.Get();
        if (!mClassInfos.Put(&clsDirEntry, (IInterface**)&interfaceObj)) {
            UnlockHashTable(EntryType_Class);
            return E_OUT_OF_MEMORY;
        }

        *object = interfaceObj;
        (*object)->AddRef();
    }
    else {
        *object = *obj;
        (*object)->AddRef();
    }

    UnlockHashTable(EntryType_Class);

    return NOERROR;
}

ECode CObjInfoList::RemoveClassInfo(
    /* [in] */ ClassDirEntry* clsDirEntry)
{
    if (!clsDirEntry) {
        return E_INVALID_ARGUMENT;
    }

    mClassInfos.Remove(&clsDirEntry);
    return NOERROR;
}

ECode CObjInfoList::AcquireStaticStructInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ StructDirEntry* structDirEntry,
    /* [in, out] */ IInterface** object)
{
    if (!clsModule || !structDirEntry || !object) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_Struct);
    if (*object) {
        UnlockHashTable(EntryType_Struct);
        return NOERROR;
    }

    IInterface** obj = mStructInfos.Get(&structDirEntry);
    if (!obj) {
        IInterface *interfaceObj = NULL;
        AutoPtr<CStructInfo> structInfoObj = new CStructInfo();
        if (structInfoObj == NULL) {
            UnlockHashTable(EntryType_Struct);
            return E_OUT_OF_MEMORY;
        }

        ECode ec = structInfoObj->InitStatic(clsModule, structDirEntry);
        if (FAILED(ec)) {
            UnlockHashTable(EntryType_Struct);
            return ec;
        }

        interfaceObj = (IInterface*)structInfoObj.Get();
        if (!mStructInfos.Put(&structDirEntry, (IInterface**)&interfaceObj)) {
            UnlockHashTable(EntryType_Struct);
            return E_OUT_OF_MEMORY;
        }

        *object = interfaceObj;
        (*object)->AddRef();
    }
    else {
        *object = *obj;
        (*object)->AddRef();
    }

    UnlockHashTable(EntryType_Struct);

    return NOERROR;
}

ECode CObjInfoList::AcquireDynamicStructInfo(
    /* [in] */ const String& name,
    /* [in] */ ArrayOf<String>* fieldNames,
    /* [in] */ ArrayOf<IDataTypeInfo*>* fieldTypeInfos,
    /* [out] */ IStructInfo** structInfo)
{
    if (name.IsNullOrEmpty() || fieldNames == NULL
        || fieldTypeInfos == NULL || !structInfo
        || fieldNames->GetLength() != fieldTypeInfos->GetLength()) {
        return E_INVALID_ARGUMENT;
    }

    InfoLinkNode* node = mStructInfoHead;
    String structName;
    AutoPtr<CStructInfo> structInfoObj;
    Int32 count = 0, i = 0;
    LockHashTable(EntryType_Struct);
    for (; node; node = node->mNext) {
        structInfoObj = (CStructInfo *)node->mInfo;
        structInfoObj->GetName(&structName);

        if (name.Equals(structName)) {
            structInfoObj->GetFieldCount(&count);
            if (count != fieldNames->GetLength()) {
                UnlockHashTable(EntryType_Struct);
                return E_DATAINFO_EXIST;
            }

            AutoPtr< ArrayOf<String> > _fieldNames = structInfoObj->mFieldNames;
            AutoPtr< ArrayOf<IDataTypeInfo*> > _fieldTypeInfos = structInfoObj->mFieldTypeInfos;
            for (i = 0; i < count; i++) {
                if (!(*fieldNames)[i].Equals((*_fieldNames)[i])) {
                    UnlockHashTable(EntryType_Struct);
                    return E_DATAINFO_EXIST;
                }
                if ((*_fieldTypeInfos)[i] != (*fieldTypeInfos)[i]) {
                    UnlockHashTable(EntryType_Struct);
                    return E_DATAINFO_EXIST;
                }
            }

            *structInfo = structInfoObj;
            (*structInfo)->AddRef();
            UnlockHashTable(EntryType_Struct);
            return NOERROR;
        }
    }

    structInfoObj = new CStructInfo();
    if (structInfoObj == NULL) {
        UnlockHashTable(EntryType_Struct);
        return E_OUT_OF_MEMORY;
    }

    ECode ec = structInfoObj->InitDynamic(name, fieldNames, fieldTypeInfos);
    if (FAILED(ec)) {
        UnlockHashTable(EntryType_Struct);
        return ec;
    }

    ec = AddInfoNode(structInfoObj, &mStructInfoHead);
    if (FAILED(ec)) {
        UnlockHashTable(EntryType_Struct);
        return ec;
    }

    *structInfo = structInfoObj;
    (*structInfo)->AddRef();

    UnlockHashTable(EntryType_Struct);

    return NOERROR;
}

ECode CObjInfoList::RemoveStructInfo(
    /* [in] */ StructDirEntry* structDirEntry)
{
    if (!structDirEntry) {
        return E_INVALID_ARGUMENT;
    }

    mStructInfos.Remove(&structDirEntry);
    return NOERROR;
}

ECode CObjInfoList::AcquireStaticEnumInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ EnumDirEntry* enumDirEntry,
    /* [in, out] */ IInterface** object)
{
    if (!clsModule || !enumDirEntry || !object) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_Enum);
    if (*object) {
        UnlockHashTable(EntryType_Enum);
        return NOERROR;
    }

    IInterface** obj = mEnumInfos.Get(&enumDirEntry);
    if (!obj) {
        IInterface *interfaceObj = NULL;
        AutoPtr<CEnumInfo> enumInfoObj = new CEnumInfo();
        if (enumInfoObj == NULL) {
            UnlockHashTable(EntryType_Enum);
            return E_OUT_OF_MEMORY;
        }

        ECode ec = enumInfoObj->InitStatic(clsModule, enumDirEntry);
        if (FAILED(ec)) {
            UnlockHashTable(EntryType_Enum);
            return ec;
        }

        interfaceObj = (IInterface*)enumInfoObj.Get();
        if (!mEnumInfos.Put(&enumDirEntry, (IInterface**)&interfaceObj)) {
            UnlockHashTable(EntryType_Enum);
            return E_OUT_OF_MEMORY;
        }

        *object = interfaceObj;
        (*object)->AddRef();
    }
    else {
        *object = *obj;
        (*object)->AddRef();
    }

    UnlockHashTable(EntryType_Enum);

    return NOERROR;
}

ECode CObjInfoList::RemoveEnumInfo(
    /* [in] */ EnumDirEntry* enumDirEntry)
{
    if (!enumDirEntry) {
        return E_INVALID_ARGUMENT;
    }

    mEnumInfos.Remove(&enumDirEntry);
    return NOERROR;
}

ECode CObjInfoList::AcquireDynamicEnumInfo(
    /* [in] */ const String& fullName,
    /* [in] */ ArrayOf<String>* itemNames,
    /* [in] */ ArrayOf<Int32>* itemValues,
    /* [out] */ IEnumInfo** enumInfo)
{
    if (fullName.IsNull() || itemNames == NULL
        || itemValues == NULL || !enumInfo
        || itemNames->GetLength() != itemValues->GetLength()) {
        return E_INVALID_ARGUMENT;
    }

    InfoLinkNode* node = mEnumInfoHead;
    String enumName;
    String enumNamespace;
    AutoPtr<CEnumInfo> enumInfoObj;
    Int32 count = 0, i = 0;

    LockHashTable(EntryType_Enum);
    for (; node; node = node->mNext) {
        enumInfoObj = (CEnumInfo *)node->mInfo;
        enumInfoObj->GetName(&enumName);
        enumInfoObj->GetNamespace(&enumNamespace);

        Int32 index = fullName.LastIndexOf(".");
        String name = index > 0 ? fullName.Substring(index + 1) : fullName;
        String nameSpace = index > 0 ? fullName.Substring(0, index - 1) : String("");
        if (name.Equals(enumName) && nameSpace.Equals(enumNamespace)) {
            enumInfoObj->GetItemCount(&count);
            if (count != itemNames->GetLength()) {
                if (!name.IsEmpty()) {
                    UnlockHashTable(EntryType_Enum);
                    return E_DATAINFO_EXIST;
                }
                else {
                    continue;
                }
            }

            AutoPtr< ArrayOf<String> > _itemNames = enumInfoObj->mItemNames;
            AutoPtr< ArrayOf<Int32> > _itemValues = enumInfoObj->mItemValues;
            for (i = 0; i < count; i++) {
                if (!(*itemNames)[i].Equals((*_itemNames)[i])) {
                    if (!name.IsEmpty()) {
                        UnlockHashTable(EntryType_Enum);
                        return E_DATAINFO_EXIST;
                    }
                    else {
                        continue;
                    }
                }
                if ((*itemValues)[i] != (*_itemValues)[i]) {
                    if (!name.IsEmpty()) {
                        UnlockHashTable(EntryType_Enum);
                        return E_DATAINFO_EXIST;
                    }
                    else {
                        continue;
                    }
                }
            }

            *enumInfo = enumInfoObj;
            (*enumInfo)->AddRef();
            return NOERROR;
        }
    }

    enumInfoObj = new CEnumInfo();
    if (enumInfoObj == NULL) {
        UnlockHashTable(EntryType_Enum);
        return E_OUT_OF_MEMORY;
    }

    ECode ec = enumInfoObj->InitDynamic(fullName, itemNames, itemValues);
    if (FAILED(ec)) {
        UnlockHashTable(EntryType_Enum);
        return ec;
    }

    ec = AddInfoNode(enumInfoObj, &mEnumInfoHead);
    if (FAILED(ec)) {
        UnlockHashTable(EntryType_Enum);
        return ec;
    }

    *enumInfo = enumInfoObj;
    (*enumInfo)->AddRef();

    UnlockHashTable(EntryType_Enum);

    return NOERROR;
}

ECode CObjInfoList::AcquireTypeAliasInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ AliasDirEntry* aliasDirEntry,
    /* [in, out] */ IInterface** object)
{
    if (!clsModule || !aliasDirEntry || !object) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_TypeAliase);
    if (*object) {
        UnlockHashTable(EntryType_TypeAliase);
        return NOERROR;
    }

    IInterface** obj = mTypeAliasInfos.Get(&aliasDirEntry);
    if (!obj) {
        IInterface *interfaceObj = NULL;
        AutoPtr<CTypeAliasInfo> aliasInfoObj = new CTypeAliasInfo(clsModule,
                aliasDirEntry);
        if (aliasInfoObj == NULL) {
            UnlockHashTable(EntryType_TypeAliase);
            return E_OUT_OF_MEMORY;
        }

        interfaceObj = (IInterface*)aliasInfoObj.Get();
        if (!mTypeAliasInfos.Put(&aliasDirEntry, (IInterface**)&interfaceObj)) {
            UnlockHashTable(EntryType_TypeAliase);
            return E_OUT_OF_MEMORY;
        }

        *object = interfaceObj;
        (*object)->AddRef();
    }
    else {
        *object = *obj;
        (*object)->AddRef();
    }

    UnlockHashTable(EntryType_TypeAliase);

    return NOERROR;
}

ECode CObjInfoList::RemoveTypeAliasInfo(
    /* [in] */ AliasDirEntry* aliasDirEntry)
{
    if (!aliasDirEntry) {
        return E_INVALID_ARGUMENT;
    }

    mTypeAliasInfos.Remove(&aliasDirEntry);
    return NOERROR;
}

ECode CObjInfoList::AcquireInterfaceInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ UInt32 index,
    /* [in, out] */ IInterface** object)
{
    if (!clsModule || !object) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_Interface);
    if (*object) {
        UnlockHashTable(EntryType_Interface);
        return NOERROR;
    }

    InterfaceDirEntry* ifDir = getInterfaceDirAddr(clsModule->mBase,
            clsModule->mClsMod->mInterfaceDirs, index);
    EIID iid = adjustInterfaceDescAddr(clsModule->mBase, ifDir->mDesc)->mIID;

    IInterface** obj = mIFInfos.Get(&iid);
    if (!obj) {
        IInterface *interfaceObj = NULL;
        AutoPtr<CInterfaceInfo> ifInfoObj = new CInterfaceInfo(clsModule, index);
        if (ifInfoObj == NULL) {
            UnlockHashTable(EntryType_Interface);
            return E_OUT_OF_MEMORY;
        }

        ECode ec = ifInfoObj->Init();
        if (FAILED(ec)) {
            UnlockHashTable(EntryType_Interface);
            return ec;
        }

        interfaceObj = (IInterface*)ifInfoObj.Get();
        if (!mIFInfos.Put(&iid, (IInterface**)&interfaceObj)) {
            UnlockHashTable(EntryType_Interface);
            return E_OUT_OF_MEMORY;
        }

        *object = interfaceObj;
        (*object)->AddRef();
    }
    else {
        *object = *obj;
        (*object)->AddRef();
    }

    UnlockHashTable(EntryType_Interface);

    return NOERROR;
}

ECode CObjInfoList::RemoveInterfaceInfo(
    /* [in] */ EIID iid)
{
    mIFInfos.Remove(&iid);
    return NOERROR;
}

ECode CObjInfoList::AcquireMethodInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ MethodDescriptor* methodDescriptor,
    /* [in] */ UInt32 index,
    /* [in, out] */ IInterface** object)
{
    if (!clsModule || !methodDescriptor || !object) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_Method);
    if (*object) {
        UnlockHashTable(EntryType_Method);
        return NOERROR;
    }

    UInt64 keyValue;
    memcpy(&keyValue, &methodDescriptor, 4);
    memcpy((PByte)&keyValue + 4, &index, 4);
    IInterface** obj = mMethodInfos.Get(&keyValue);
    if (!obj) {
        IMethodInfo* iMethodInfo = NULL;
        AutoPtr<CMethodInfo> methodInfoObj = new CMethodInfo(clsModule,
                methodDescriptor, index);
        if (methodInfoObj == NULL) {
            UnlockHashTable(EntryType_Method);
            return E_OUT_OF_MEMORY;
        }

        ECode ec = methodInfoObj->Init();
        if (FAILED(ec)) {
            UnlockHashTable(EntryType_Method);
            return ec;
        }

        iMethodInfo = (IMethodInfo*)methodInfoObj.Get();
        if (!mMethodInfos.Put(&keyValue, (IInterface**)&iMethodInfo)) {
            UnlockHashTable(EntryType_Method);
            return E_OUT_OF_MEMORY;
        }
        *object = iMethodInfo;
        (*object)->AddRef();
    }
    else {
        *object = *obj;
        (*object)->AddRef();
    }

    UnlockHashTable(EntryType_Method);

    return NOERROR;
}

ECode CObjInfoList::RemoveMethodInfo(
    /* [in] */ MethodDescriptor* methodDescriptor,
    /* [in] */ UInt32 index)
{
    if (!methodDescriptor) {
        return E_INVALID_ARGUMENT;
    }

    UInt64 keyValue;
    memcpy(&keyValue, &methodDescriptor, 4);
    memcpy((PByte)&keyValue + 4, &index, 4);
    mMethodInfos.Remove(&keyValue);

    return NOERROR;
}

ECode CObjInfoList::AcquireDataTypeInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ TypeDescriptor* typeDesc,
    /* [out] */ IDataTypeInfo** dataTypeInfo,
    /* [in] */ Boolean isCheckLocalPtr)
{
    if (!clsModule || !typeDesc || !dataTypeInfo) {
        return E_INVALID_ARGUMENT;
    }

    ECode ec = NOERROR;
    CLSModule* clsMod = clsModule->mClsMod;
    Int32 pointer = typeDesc->mPointer;
    if (typeDesc->mType == Type_alias) {
        ec = clsModule->AliasToOriginal(typeDesc, &typeDesc);
        if (FAILED(ec)) return ec;
        if (isCheckLocalPtr) pointer += typeDesc->mPointer;
    }

    CarDataType type = GetCarDataType(typeDesc->mType);
    if (isCheckLocalPtr) {
        if (type == CarDataType_Interface) {
            if (pointer > 1) {
                type = CarDataType_LocalPtr;
                pointer -= 1;
            }
        }
        else if (pointer > 0) {
            type = CarDataType_LocalPtr;
        }
    }

    *dataTypeInfo = NULL;
    //LocalPtr
    if (type == CarDataType_LocalPtr) {
        return g_objInfoList.AcquireLocalPtrInfo(clsModule, typeDesc,
                pointer, (ILocalPtrInfo **)dataTypeInfo);
    }
    //LocalType
    else if (type == CarDataType_LocalType) {
        MemorySize size = GetDataTypeSize(clsModule, typeDesc);
        return AcquireLocalTypeInfo(typeDesc->mType, size, dataTypeInfo);
    }
    //StructInfo
    else if (type == CarDataType_Struct) {
        StructDirEntry* structDir = getStructDirAddr(clsModule->mBase,
                clsModule->mClsMod->mStructDirs, typeDesc->mIndex);
        ec = AcquireStaticStructInfo(clsModule,
                structDir, (IInterface **)dataTypeInfo);
    }
    //InterfaceInfo
    else if (type == CarDataType_Interface) {
        ec = AcquireInterfaceInfo(clsModule,
                typeDesc->mIndex, (IInterface **)dataTypeInfo);
    }
    //EnumInfo
    else if (type == CarDataType_Enum) {
        EnumDirEntry* enumDir = getEnumDirAddr(clsModule->mBase,
                clsMod->mEnumDirs, typeDesc->mIndex);
        ec = AcquireStaticEnumInfo(clsModule,
                enumDir, (IInterface **)dataTypeInfo);
    }
    //CppVectorInfo
    else if (type == CarDataType_CppVector) {
        AutoPtr<IDataTypeInfo> elemInfo;
        ArrayDirEntry* arrayDir = getArrayDirAddr(clsModule->mBase,
                clsMod->mArrayDirs, typeDesc->mIndex);
        Int32 length = arrayDir->mElementCount;
        TypeDescriptor* type = &arrayDir->mType;
        ec = AcquireDataTypeInfo(clsModule, type, (IDataTypeInfo**)&elemInfo,
                isCheckLocalPtr);
        if (FAILED(ec)) return ec;
        ec = AcquireCppVectorInfo(elemInfo, length,
                (ICppVectorInfo**)dataTypeInfo);
    }
    //CarArrayInfo
    else if (type == CarDataType_ArrayOf) {
        AutoPtr<IDataTypeInfo> elemInfo;
        ec = AcquireCarArrayElementTypeInfo(clsModule, typeDesc,
                (IDataTypeInfo**)&elemInfo);
        if (FAILED(ec)) return ec;
        ec = AcquireCarArrayInfo(type, elemInfo, (ICarArrayInfo**)dataTypeInfo);
    }
    //IntrinsicInfo
    else {
        CarDataType type = GetCarDataType(typeDesc->mType);

        if (type != CarDataType_LocalType) {
            UInt32 size = GetDataTypeSize(clsModule, typeDesc);
            ec = AcquireIntrinsicInfo(type, dataTypeInfo, size);
        }
    }

    return ec;
}

ECode CObjInfoList::AcquireIntrinsicInfo(
    /* [in] */ CarDataType dataType,
    /* [out] */ IDataTypeInfo** dataTypeInfo,
    /* [in] */ UInt32 size)
{
    if (!dataTypeInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (g_cDataTypeList[dataType].mSize) {
        size = g_cDataTypeList[dataType].mSize;
    }

    LockHashTable(EntryType_DataType);
    if (!mDataTypeInfos[dataType]) {
        mDataTypeInfos[dataType] = new CIntrinsicInfo(dataType, size);

        if (!mDataTypeInfos[dataType]) {
            UnlockHashTable(EntryType_DataType);
            return E_OUT_OF_MEMORY;
        }
        mDataTypeInfos[dataType]->AddRef();
    }
    UnlockHashTable(EntryType_DataType);

    *dataTypeInfo = mDataTypeInfos[dataType];
    (*dataTypeInfo)->AddRef();

    return NOERROR;
}

ECode CObjInfoList::AcquireLocalTypeInfo(
    /* [in] */ CARDataType type,
    /* [in] */ MemorySize size,
    /* [out] */ IDataTypeInfo** dataTypeInfo)
{
    if (!dataTypeInfo) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_Local);
    if (!mLocalTypeInfos[type]) {
        mLocalTypeInfos[type] = new CLocalTypeInfo(size);
        if (!mLocalTypeInfos[type]) {
            UnlockHashTable(EntryType_Local);
            return E_OUT_OF_MEMORY;
        }
        mLocalTypeInfos[type]->AddRef();
    }
    UnlockHashTable(EntryType_Local);

    *dataTypeInfo = mLocalTypeInfos[type];
    (*dataTypeInfo)->AddRef();

    return NOERROR;
}

ECode CObjInfoList::AcquireLocalPtrInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ TypeDescriptor* typeDescriptor,
    /* [in] */ Int32 pointer,
    /* [out] */ ILocalPtrInfo** localPtrInfo)
{
    if (!clsModule || !typeDescriptor || !localPtrInfo) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_Local);

    UInt64 keyValue;
    memcpy(&keyValue, &typeDescriptor, 4);
    memcpy((PByte)&keyValue + 4, &pointer, 4);
    IInterface** obj = mLocalPtrInfos.Get(&keyValue);
    if (!obj) {
        ILocalPtrInfo *interfaceObj = NULL;
        AutoPtr<CLocalPtrInfo> localPtrInfoObj = new CLocalPtrInfo(clsModule,
                typeDescriptor, pointer);
        if (localPtrInfoObj == NULL) {
            UnlockHashTable(EntryType_Local);
            return E_OUT_OF_MEMORY;
        }

        interfaceObj = (ILocalPtrInfo*)localPtrInfoObj.Get();
        if (!mLocalPtrInfos.Put(&keyValue, (IInterface**)&interfaceObj)) {
            UnlockHashTable(EntryType_Local);
            return E_OUT_OF_MEMORY;
        }
        *localPtrInfo = interfaceObj;
        (*localPtrInfo)->AddRef();
    }
    else {
        *localPtrInfo = (ILocalPtrInfo *)*obj;
        (*localPtrInfo)->AddRef();
    }

    UnlockHashTable(EntryType_Local);

    return NOERROR;
}

ECode CObjInfoList::RemoveLocalPtrInfo(
    /* [in] */ TypeDescriptor* typeDescriptor,
    /* [in] */ Int32 pointer)
{
    if (!typeDescriptor) {
        return E_INVALID_ARGUMENT;
    }

    UInt64 keyValue;
    memcpy(&keyValue, &typeDescriptor, 4);
    memcpy((PByte)&keyValue + 4, &pointer, 4);
    mLocalPtrInfos.Remove(&keyValue);

    return NOERROR;
}

ECode CObjInfoList::AcquireCppVectorInfo(
    /* [in] */ IDataTypeInfo* elementTypeInfo,
    /* [in] */ Int32 length,
    /* [out] */ ICppVectorInfo** cppVectorInfo)
{
    if (!elementTypeInfo || !cppVectorInfo) {
        return E_INVALID_ARGUMENT;
    }

    InfoLinkNode* node = mCCppVectorHead;
    Int32 len = 0;
    AutoPtr<ICppVectorInfo> cppVectorInfoObj;
    ECode ec = NOERROR;
    LockHashTable(EntryType_DataType);
    for (; node; node = node->mNext) {
        cppVectorInfoObj = (ICppVectorInfo *)node->mInfo;
        cppVectorInfoObj->GetLength(&len);
        if (len == length) {
            AutoPtr<IDataTypeInfo> elementInfo;
            ec = cppVectorInfoObj->GetElementTypeInfo((IDataTypeInfo**)&elementInfo);
            if (FAILED(ec)) {
                UnlockHashTable(EntryType_DataType);
                return ec;
            }
            if (elementInfo.Get() == elementTypeInfo) {
                *cppVectorInfo = cppVectorInfoObj;
                (*cppVectorInfo)->AddRef();
                UnlockHashTable(EntryType_DataType);
                return NOERROR;
            }
        }
    }

    cppVectorInfoObj = new CCppVectorInfo(elementTypeInfo, length);
    if (cppVectorInfoObj == NULL) {
        UnlockHashTable(EntryType_DataType);
        return E_OUT_OF_MEMORY;
    }

    ec = AddInfoNode(cppVectorInfoObj, &mCCppVectorHead);
    if (FAILED(ec)) {
        UnlockHashTable(EntryType_DataType);
        return ec;
    }

    *cppVectorInfo = cppVectorInfoObj;
    (*cppVectorInfo)->AddRef();
    UnlockHashTable(EntryType_DataType);

    return NOERROR;
}

ECode CObjInfoList::AcquireCarArrayElementTypeInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ TypeDescriptor* typeDesc,
    /* [out] */ IDataTypeInfo** elementTypeInfo)
{
    CarDataType type;
    switch (typeDesc->mType) {
        case Type_ArrayOf:
            return AcquireDataTypeInfo(clsModule,
                    adjustNestedTypeAddr(clsModule->mBase, typeDesc->mNestedType),
                    elementTypeInfo);
        default:
            return E_INVALID_OPERATION;
    }

    return AcquireIntrinsicInfo(type, elementTypeInfo);
}

ECode CObjInfoList::AcquireCarArrayInfo(
    /* [in] */ CarDataType quintetType,
    /* [in] */ IDataTypeInfo* elementTypeInfo,
    /* [out] */ ICarArrayInfo** carArrayInfo)
{

    if ((quintetType != CarDataType_ArrayOf) || !elementTypeInfo || !carArrayInfo) {
        return E_INVALID_ARGUMENT;
    }

    InfoLinkNode* node = mCarArrayInfoHead;
    AutoPtr<ICarArrayInfo> carArrayInfoObj;

    CarDataType dataType;
    ECode ec = elementTypeInfo->GetDataType(&dataType);
    if (FAILED(ec)) return ec;

    if (dataType != CarDataType_Struct) {
        LockHashTable(EntryType_DataType);
        if (!mCarArrayInfos[dataType]) {
            mCarArrayInfos[dataType] =
                    new CCarArrayInfo(quintetType, elementTypeInfo, dataType);
            if (!mCarArrayInfos[dataType]) {
                UnlockHashTable(EntryType_DataType);
                return E_OUT_OF_MEMORY;
            }
            mCarArrayInfos[dataType]->AddRef();
        }
        UnlockHashTable(EntryType_DataType);

        *carArrayInfo = mCarArrayInfos[dataType];
        (*carArrayInfo)->AddRef();
    }
    else {
        AutoPtr<IDataTypeInfo> elementInfo;
        LockHashTable(EntryType_DataType);
        for (; node; node = node->mNext) {
            carArrayInfoObj = (ICarArrayInfo *)node->mInfo;
            elementInfo = NULL;
            ec = carArrayInfoObj->GetElementTypeInfo((IDataTypeInfo**)&elementInfo);
            if (FAILED(ec)) {
                UnlockHashTable(EntryType_DataType);
                return ec;
            }
            if (elementInfo.Get() == elementTypeInfo) {
                *carArrayInfo = carArrayInfoObj;
                (*carArrayInfo)->AddRef();
                UnlockHashTable(EntryType_DataType);
                return NOERROR;
            }
        }

        carArrayInfoObj = new CCarArrayInfo(quintetType, elementTypeInfo);
        if (carArrayInfoObj == NULL) {
            UnlockHashTable(EntryType_DataType);
            return E_OUT_OF_MEMORY;
        }

        ec = AddInfoNode(carArrayInfoObj, &mCarArrayInfoHead);
        if (FAILED(ec)) {
            UnlockHashTable(EntryType_DataType);
            return ec;
        }

        *carArrayInfo = carArrayInfoObj;
        (*carArrayInfo)->AddRef();
        UnlockHashTable(EntryType_DataType);
    }

    return NOERROR;
}

ECode CObjInfoList::AcquireConstantInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ ConstDirEntry* constDirEntry,
    /* [in, out] */ IInterface** object)
{
    if (!clsModule || !constDirEntry || !object) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_Constant);
    if (*object) {
        UnlockHashTable(EntryType_Constant);
        return NOERROR;
    }

    CConstantInfo* constantInfoObj = new CConstantInfo(clsModule, constDirEntry);
    if (constantInfoObj == NULL) {
        UnlockHashTable(EntryType_Constant);
        return E_OUT_OF_MEMORY;
    }

    *object = constantInfoObj;
    (*object)->AddRef();
    UnlockHashTable(EntryType_Constant);

    return NOERROR;
}

ECode CObjInfoList::AcquireConstructorInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ MethodDescriptor* methodDescriptor,
    /* [in] */ UInt32 index,
    /* [in] */ ClassID* clsId,
    /* [in, out] */ IInterface** object)
{
    if (!clsModule || !methodDescriptor || !clsId || !object) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_Constructor);
    if (*object) {
        UnlockHashTable(EntryType_Constructor);
        return NOERROR;
    }

    AutoPtr<CConstructorInfo> constructInfoObj = new CConstructorInfo();
    if (constructInfoObj == NULL) {
        UnlockHashTable(EntryType_Constructor);
        return E_OUT_OF_MEMORY;
    }

    ECode ec = constructInfoObj->Init(clsModule,
            methodDescriptor, index, clsId);
    if (FAILED(ec)) {
        UnlockHashTable(EntryType_Constructor);
        return ec;
    }

    *object = constructInfoObj;
    (*object)->AddRef();
    UnlockHashTable(EntryType_Constructor);

    return NOERROR;
}

ECode CObjInfoList::AcquireCBMethodInfoInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ UInt32 eventNum,
    /* [in] */ MethodDescriptor* methodDescriptor,
    /* [in] */ UInt32 index,
    /* [in, out] */ IInterface** object)
{
    if (!clsModule || !methodDescriptor || !object) {
        return E_INVALID_ARGUMENT;
    }

    LockHashTable(EntryType_CBMethod);
    if (*object) {
        UnlockHashTable(EntryType_CBMethod);
        return NOERROR;
    }

    AutoPtr<CCallbackMethodInfo> cbMethodInfoObj = new CCallbackMethodInfo();
    if (cbMethodInfoObj == NULL) {
        UnlockHashTable(EntryType_CBMethod);
        return E_OUT_OF_MEMORY;
    }

    ECode ec = cbMethodInfoObj->Init(clsModule,
            eventNum, methodDescriptor, index);
    if (FAILED(ec)) {
        UnlockHashTable(EntryType_CBMethod);
        return ec;
    }

    *object = cbMethodInfoObj;
    (*object)->AddRef();
    UnlockHashTable(EntryType_CBMethod);

    return NOERROR;
}

ECode CObjInfoList::AddInfoNode(
    /* [in] */ IDataTypeInfo* info,
    /* [in] */ InfoLinkNode** linkHead)
{
    InfoLinkNode* node = new InfoLinkNode();
    if (!node) {
        return E_OUT_OF_MEMORY;
    }

    node->mInfo = info;
    node->mNext = NULL;
    if (!*linkHead) {
        *linkHead = node;
    }
    else {
        node->mNext = *linkHead;
        *linkHead = node;
    }

    return NOERROR;
}

ECode CObjInfoList::DeleteInfoNode(
    /* [in] */ IDataTypeInfo* info,
    /* [in] */ InfoLinkNode** linkHead)
{
    InfoLinkNode* preNode = *linkHead;
    InfoLinkNode* node = preNode;

    while (node) {
        if (info == node->mInfo) {
            if (node == preNode) {
                *linkHead = node->mNext;
            }
            else {
                preNode->mNext = node->mNext;
            }
            delete node;
            break;
        }
        preNode = node;
        node = node->mNext;
    }

    return NOERROR;
}

ECode CObjInfoList::DetachEnumInfo(
    /* [in] */ IEnumInfo* enumInfo)
{
    return DeleteInfoNode(enumInfo, &mEnumInfoHead);
}

ECode CObjInfoList::DetachCppVectorInfo(
    /* [in] */ ICppVectorInfo* cppVectorInfo)
{
    return DeleteInfoNode(cppVectorInfo, &mCCppVectorHead);
}

ECode CObjInfoList::DetachStructInfo(
    /* [in] */ IStructInfo* structInfo)
{
    return DeleteInfoNode(structInfo, &mStructInfoHead);
}

ECode CObjInfoList::DetachCarArrayInfo(
    /* [in] */ ICarArrayInfo* carArrayInfo)
{
    return DeleteInfoNode(carArrayInfo, &mCarArrayInfoHead);
}
