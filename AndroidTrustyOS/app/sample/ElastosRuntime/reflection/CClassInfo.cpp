//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CClassInfo.h"
#include "CCallbackMethodInfo.h"
#include "CConstructorInfo.h"

CClassInfo::CClassInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ ClassDirEntry* clsDirEntry)
{
    mClsModule = clsModule;
    mClsMod = mClsModule->mClsMod;
    mClassDirEntry = clsDirEntry;
    mIFList = NULL;
    mCBIFList = NULL;

    mCBMethodDesc = NULL;

    mMethodCount = 0;
    mCBMethodCount = 0;
    mCBIFCount = 0;
    mIFCount = 0;

    mBase = mClsModule->mBase;
    mDesc = adjustClassDescAddr(mBase, mClassDirEntry->mDesc);

    mClsId.mUunm = mUrn;
    mClsId.mClsid = mDesc->mClsid;
    strcpy(mClsId.mUunm, adjustNameAddr(mBase, mClsMod->mUunm));
}

CClassInfo::~CClassInfo()
{
    if (mCBMethodDesc) delete [] mCBMethodDesc;
    if (mIFList) delete[] mIFList;
    if (mCBIFList) delete[] mCBIFList;
}

UInt32 CClassInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CClassInfo::Release()
{
    g_objInfoList.LockHashTable(EntryType_Class);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        g_objInfoList.RemoveClassInfo(mClassDirEntry);
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_Class);
    assert(ref >= 0);
    return ref;
}

PInterface CClassInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IClassInfo) {
        return (IClassInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CClassInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CClassInfo::Init()
{
    return CreateIFList();
}

ECode CClassInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = adjustNameAddr(mBase, mClassDirEntry->mName);
    return NOERROR;
}

ECode CClassInfo::GetNamespace(
    /* [out] */ String* ns)
{
    if (ns == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *ns = adjustNameAddr(mBase, mClassDirEntry->mNameSpace);
    return NOERROR;
}

ECode CClassInfo::GetId(
    /* [out] */ ClassID* clsid)
{
    if (!clsid) {
        return E_INVALID_ARGUMENT;
    }

    clsid->mClsid =  mDesc->mClsid;
    strcpy(clsid->mUunm,  adjustNameAddr(mBase, mClsMod->mUunm));

    return NOERROR;
}

ECode CClassInfo::GetModuleInfo(
    /* [out] */ IModuleInfo** moduleInfo)
{
    return mClsModule->GetModuleInfo(moduleInfo);
}

ECode CClassInfo::GetClassLoader(
    /* [out] */ IInterface** loader)
{
    if (!loader) {
        return E_INVALID_ARGUMENT;
    }

    *loader = mClassLoader;
    REFCOUNT_ADD(*loader);

    return NOERROR;
}

ECode CClassInfo::SetClassLoader(
    /* [in] */ IInterface* loader)
{
    mClassLoader = loader;
    return NOERROR;
}

ECode CClassInfo::IsSingleton(
    /* [out] */ Boolean* isSingleton)
{
    if (!isSingleton) {
        return E_INVALID_ARGUMENT;
    }

    if (mDesc->mAttribs & ClassAttrib_singleton) {
        *isSingleton = TRUE;
    }
    else {
        *isSingleton = FALSE;
    }

    return NOERROR;
}

ECode CClassInfo::GetThreadingModel(
    /* [out] */ ThreadingModel* threadingModel)
{
    if (!threadingModel) {
        return E_INVALID_ARGUMENT;
    }

    if (mDesc->mAttribs & ClassAttrib_freethreaded) {
        *threadingModel = ThreadingModel_ThreadSafe;
    }
    else if (mDesc->mAttribs & ClassAttrib_naked) {
        *threadingModel = ThreadingModel_Naked;
    }
    else {
        return E_DOES_NOT_EXIST;
    }

    return NOERROR;
}

ECode CClassInfo::IsPrivate(
    /* [out] */ Boolean* isPrivate)
{
    if (!isPrivate) {
        return E_INVALID_ARGUMENT;
    }

    if (mDesc->mAttribs & ClassAttrib_private) {
        *isPrivate = TRUE;
    }
    else {
        *isPrivate = FALSE;
    }

    return NOERROR;
}

ECode CClassInfo::IsReturnValue(
    /* [out] */ Boolean* isReturnValue)
{
    // TODO: Add your code here
    return E_NOT_IMPLEMENTED;
}

ECode CClassInfo::IsBaseClass(
    /* [out] */ Boolean* isBaseClass)
{
    if (!isBaseClass) {
        return E_INVALID_ARGUMENT;
    }

    if (mDesc->mAttribs & ClassAttrib_hasvirtual) {
        *isBaseClass = TRUE;
    }
    else {
        *isBaseClass = FALSE;
    }

    return NOERROR;
}

ECode CClassInfo::HasBaseClass(
    /* [out] */ Boolean* hasBaseClass)
{
    if (!hasBaseClass) {
        return E_INVALID_ARGUMENT;
    }

    if (mDesc->mAttribs & ClassAttrib_hasparent) {
        *hasBaseClass = TRUE;
    }
    else {
        *hasBaseClass = FALSE;
    }

    return NOERROR;
}

ECode CClassInfo::GetBaseClassInfo(
    /* [out] */ IClassInfo** baseClassInfo)
{
    if (!baseClassInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!(mDesc->mAttribs & ClassAttrib_hasparent)) {
        return E_DOES_NOT_EXIST;
    }

    *baseClassInfo = NULL;
    return g_objInfoList.AcquireClassInfo(mClsModule,
            getClassDirAddr(mBase, mClsMod->mClassDirs, mDesc->mParentIndex),
            (IInterface **)baseClassInfo);
}

ECode CClassInfo::IsGeneric(
    /* [out] */ Boolean* isGeneric)
{
    if (!isGeneric) {
        return E_INVALID_ARGUMENT;
    }

    if (mDesc->mAttribs & ClassAttrib_t_generic) {
        *isGeneric = TRUE;
    }
    else {
        *isGeneric = FALSE;
    }

    return NOERROR;
}

ECode CClassInfo::HasGeneric(
    /* [out] */ Boolean* hasGeneric)
{
    if (!hasGeneric) {
        return E_INVALID_ARGUMENT;
    }
    *hasGeneric = FALSE;

    ECode ec = NOERROR;
    AutoPtr<IClassInfo> parentClassInfo;
    AutoPtr<IClassInfo> thisClassInfo = (IClassInfo*)this->Probe(EIID_IClassInfo);
    Boolean hasBaseClass = FALSE;
    while (TRUE) {
        ec = thisClassInfo->HasBaseClass(&hasBaseClass);
        if (FAILED(ec)) break;

        if (hasBaseClass) {
            parentClassInfo = NULL;
            ec = thisClassInfo->GetBaseClassInfo((IClassInfo**)&parentClassInfo);
            if (FAILED(ec)) break;

            ec = parentClassInfo->IsGeneric(hasGeneric);
            if (FAILED(ec)) break;

            if (*hasGeneric) {
                mGenericInfo = parentClassInfo;
                break;
            }
            else {
                thisClassInfo = parentClassInfo;
            }
        }
        else {
            break;
        }
    }

    return ec;
}

ECode CClassInfo::GetGenericInfo(
    /* [out] */ IClassInfo** genericInfo)
{
    if (!genericInfo) {
        return E_INVALID_ARGUMENT;
    }

    *genericInfo = mGenericInfo;
    REFCOUNT_ADD(*genericInfo);

    return NOERROR;
}

ECode CClassInfo::IsRegime(
    /* [out] */ Boolean* isRegime)
{
    if (!isRegime) {
        return E_INVALID_ARGUMENT;
    }

    if (mDesc->mAttribs & ClassAttrib_t_regime) {
        *isRegime = TRUE;
    }
    else {
        *isRegime = FALSE;
    }

    return NOERROR;
}

ECode CClassInfo::GetAspectCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mDesc->mAspectCount;

    return NOERROR;
}

ECode CClassInfo::AcquireAspectList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_Aspect);
    if (!mAspectList) {
        mAspectList = new CEntryList(EntryType_Aspect,
            mDesc, mDesc->mAspectCount, mClsModule);
        if (!mAspectList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_Aspect);

    return ec;
}

ECode CClassInfo::GetAllAspectInfos(
    /* [out] */ ArrayOf<IClassInfo *>* aspectInfos)
{
    ECode ec = AcquireAspectList();
    if (FAILED(ec)) return ec;

    return mAspectList->GetAllObjInfos((PTypeInfos)aspectInfos);
}

ECode CClassInfo::GetAspectInfo(
    /* [in] */ const String& name,
    /* [out] */ IClassInfo** aspectInfo)
{
    if (name.IsNull() || !aspectInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!mDesc->mAspectCount) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec = AcquireAspectList();
    if (FAILED(ec)) return ec;

    return mAspectList->AcquireObjByName(name, (IInterface **)aspectInfo);
}

ECode CClassInfo::IsAspect(
    /* [out] */ Boolean* isAspect)
{
    if (!isAspect) {
        return E_INVALID_ARGUMENT;
    }

    if (mDesc->mAttribs & ClassAttrib_t_aspect) {
        *isAspect = TRUE;
    }
    else {
        *isAspect = FALSE;
    }

    return NOERROR;
}

ECode CClassInfo::GetAggregateeCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mDesc->mAggregateCount;

    return NOERROR;
}

ECode CClassInfo::AcquireAggregateeList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_Aggregatee);
    if (!mAggregateeList) {
        mAggregateeList = new CEntryList(EntryType_Aggregatee,
                mDesc, mDesc->mAggregateCount, mClsModule);
        if (!mAggregateeList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_Aggregatee);

    return ec;
}

ECode CClassInfo::GetAllAggregateeInfos(
    /* [out] */ ArrayOf<IClassInfo *>* aggregateeInfos)
{
    ECode ec = AcquireAggregateeList();
    if (FAILED(ec)) return ec;

    return mAggregateeList->GetAllObjInfos((PTypeInfos)aggregateeInfos);
}

ECode CClassInfo::GetAggregateeInfo(
    /* [in] */ const String& name,
    /* [out] */ IClassInfo** aggregateeInfo)
{
    if (name.IsNull() || !aggregateeInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!mDesc->mAggregateCount) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec = AcquireAggregateeList();
    if (FAILED(ec)) return ec;

    return mAggregateeList->AcquireObjByName(name,
            (IInterface **)aggregateeInfo);
}

ECode CClassInfo::AcquireConstructorList()
{
    if (!(mDesc->mAttribs & ClassAttrib_hasctor)) {
        return NOERROR;
    }

    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_Class);
    if (!mCtorClassInfo) {
        AutoPtr<IModuleInfo> pModuleInfo;

        ec = mClsModule->GetModuleInfo((IModuleInfo**)&pModuleInfo);
        if (FAILED(ec)) {
            g_objInfoList.UnlockHashTable(EntryType_Class);
            return ec;
        }

        String clsName, ns;
        clsName = adjustNameAddr(mBase, mClassDirEntry->mName);
        clsName += "ClassObject";
        ns = adjustNameAddr(mBase, mClassDirEntry->mNameSpace);

        String relectionClsName(ns);
        if (!ns.IsNullOrEmpty())
            relectionClsName += ".";
        relectionClsName += clsName;
        AutoPtr<IClassInfo> classInfo;
        ec = pModuleInfo->GetClassInfo(relectionClsName, (IClassInfo**)&classInfo);
        if (FAILED(ec)) {
            g_objInfoList.UnlockHashTable(EntryType_Class);
            return ec;
        }

        mCtorClassInfo = (CClassInfo*)classInfo.Get();
    }
    g_objInfoList.UnlockHashTable(EntryType_Class);

    mCtorList = NULL;
    return mCtorClassInfo->AcquireSpecialMethodList(
            EntryType_Constructor, (CEntryList**)&mCtorList);
}

ECode CClassInfo::GetConstructorCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    if (!(mDesc->mAttribs & ClassAttrib_hasctor)) {
        *count = 0;
    }
    else {
        ECode ec = AcquireConstructorList();
        if (FAILED(ec)) return ec;

        *count = mCtorList->mTotalCount;
    }

    return NOERROR;
}

ECode CClassInfo::GetAllConstructorInfos(
    /* [out] */ ArrayOf<IConstructorInfo *>* constructorInfos)
{
    if (!constructorInfos) {
        return E_INVALID_ARGUMENT;
    }

    if (!(mDesc->mAttribs & ClassAttrib_hasctor)) {
        return NOERROR;
    }

    ECode ec = AcquireConstructorList();
    if (FAILED(ec)) return ec;

    ec = mCtorList->GetAllObjInfos((PTypeInfos)constructorInfos);
    if (FAILED(ec)) return ec;

    Int32 size = constructorInfos->GetLength();
    for (Int32 i = 0; i < size; i++) {
        CConstructorInfo* consInfoObj = (CConstructorInfo*)(*constructorInfos)[i];
        consInfoObj->mInstClsId.mClsid = mClsId.mClsid;
        strcpy(consInfoObj->mInstClsId.mUunm, mClsId.mUunm);
    }

    return NOERROR;
}

ECode CClassInfo::GetConstructorInfoByParamNames(
    /* [in] */ const String& name,
    /* [out] */ IConstructorInfo** constructorInfo)
{
    if (name.IsNull() || !constructorInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!(mDesc->mAttribs & ClassAttrib_hasctor)) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec = AcquireConstructorList();
    if (FAILED(ec)) return ec;

    String objName("CreateObjectWith");
    objName += name;

    ec = mCtorList->AcquireObjByName(objName,
            (IInterface **)constructorInfo);
    if (FAILED(ec)) return ec;

    CConstructorInfo* consInfoObj = (CConstructorInfo*)(*constructorInfo);
    consInfoObj->mInstClsId.mClsid = mClsId.mClsid;
    strcpy(consInfoObj->mInstClsId.mUunm, mClsId.mUunm);

    return NOERROR;
}

ECode CClassInfo::GetConstructorInfoByParamCount(
    /* [in] */ Int32 count,
    /* [out] */ IConstructorInfo** retConstructorInfo)
{
    if (!retConstructorInfo) {
        return E_INVALID_ARGUMENT;
    }
    *retConstructorInfo = NULL;
    if (count < 0) {
        return E_INVALID_ARGUMENT;
    }

    if (!(mDesc->mAttribs & ClassAttrib_hasctor)) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec = AcquireConstructorList();
    if (FAILED(ec)) return ec;

    AutoPtr<IConstructorInfo> constructorInfo;

    Int32 iCount = 0;
    for (UInt32 i = 0; i < mCtorList->mTotalCount; i++) {
        AutoPtr<IInterface> obj;
        ec = mCtorList->AcquireObjByIndex(i, (IInterface**)&obj);
        if (FAILED(ec)) return ec;

        constructorInfo = IConstructorInfo::Probe(obj);
        ec = constructorInfo->GetParamCount(&iCount);
        if (FAILED(ec)) {
            return ec;
        }
        if (iCount == count) {
            if (NULL == *retConstructorInfo) {
                *retConstructorInfo = constructorInfo;
                (*retConstructorInfo)->AddRef();
            }
            else {
                (*retConstructorInfo)->Release();
                *retConstructorInfo = NULL;
                return E_OUT_OF_NUMBER;
            }
        }
    }

    if (*retConstructorInfo == NULL) {
        return E_DOES_NOT_EXIST;
    }

    CConstructorInfo* consInfoObj = (CConstructorInfo*)(*retConstructorInfo);
    consInfoObj->mInstClsId.mClsid = mClsId.mClsid;
    strcpy(consInfoObj->mInstClsId.mUunm, mClsId.mUunm);

    return NOERROR;
}

ECode CClassInfo::GetInterfaceCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mIFCount;
    return NOERROR;
}

ECode CClassInfo::AcquireInterfaceList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_ClassInterface);
    if (!mInterfaceList) {
        mInterfaceList = new CEntryList(EntryType_ClassInterface,
                mDesc, mIFCount, mClsModule, mIFList, mIFCount);
        if (!mInterfaceList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_ClassInterface);

    return ec;
}

ECode CClassInfo::GetAllInterfaceInfos(
    /* [out] */ ArrayOf<IInterfaceInfo *>* interfaceInfos)
{
    ECode ec = AcquireInterfaceList();
    if (FAILED(ec)) return ec;

    return mInterfaceList->GetAllObjInfos((PTypeInfos)interfaceInfos);
}

ECode CClassInfo::GetInterfaceInfo(
    /* [in] */ const String& fullName,
    /* [out] */ IInterfaceInfo** interfaceInfo)
{
    if (fullName.IsNull() || !interfaceInfo) {
        return E_INVALID_ARGUMENT;
    }

    ECode ec = AcquireInterfaceList();
    if (FAILED(ec)) return ec;

    assert(fullName.IndexOf(';') < 0 && fullName.IndexOf('/') < 0);
    return mInterfaceList->AcquireObjByName(fullName, (IInterface**)interfaceInfo);
}

ECode CClassInfo::HasInterfaceInfo(
    /* [in] */ IInterfaceInfo* interfaceInfo,
    /* [out] */ Boolean* result)
{
    if (!result) {
        return E_INVALID_ARGUMENT;
    }

    for (Int32 i = 0; i < mIFCount; i++) {
        AutoPtr<IInterfaceInfo> obj;
        mInterfaceList->AcquireObjByIndex(i, (IInterface**)&obj);
        IInterfaceInfo* itfInfo = IInterfaceInfo::Probe(obj);
        if (interfaceInfo == itfInfo) {
            *result = TRUE;
            return NOERROR;
        }
    }
    *result = FALSE;
    return NOERROR;
}

ECode CClassInfo::GetCallbackInterfaceCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mCBIFCount;

    return NOERROR;
}

ECode CClassInfo::AcquireCBInterfaceList()
{
    if (!mCBIFList) {
        return NOERROR;
    }

    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_ClassInterface);
    if (!mCBInterfaceList) {
        mCBInterfaceList = new CEntryList(EntryType_ClassInterface,
            mDesc, mCBIFCount, mClsModule, mCBIFList, mCBIFCount);
        if (!mCBInterfaceList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_ClassInterface);

    return ec;
}

ECode CClassInfo::GetAllCallbackInterfaceInfos(
    /* [out] */ ArrayOf<IInterfaceInfo *>* callbackInterfaceInfos)
{
    ECode ec = AcquireCBInterfaceList();
    if (FAILED(ec)) return ec;

    return mCBInterfaceList->GetAllObjInfos((PTypeInfos)callbackInterfaceInfos);
}

ECode CClassInfo::GetCallbackInterfaceInfo(
    /* [in] */ const String& name,
    /* [out] */ IInterfaceInfo** callbackInterfaceInfo)
{
    if (name.IsNull() || !callbackInterfaceInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!mCBIFList) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec = AcquireCBInterfaceList();
    if (FAILED(ec)) return ec;

    return mCBInterfaceList->AcquireObjByName(name,
            (IInterface **)callbackInterfaceInfo);
}

ECode CClassInfo::GetMethodCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mMethodCount;

    return NOERROR;
}

ECode CClassInfo::AcquireMethodList()
{
    mMethodList = NULL;
    return AcquireSpecialMethodList(EntryType_Method, (CEntryList**)&mMethodList);
}

ECode CClassInfo::AcquireSpecialMethodList(
    /* [in] */ EntryType type,
    /* [out] */ CEntryList** entryList)
{
    ECode ec = NOERROR;

    g_objInfoList.LockHashTable(type);
    if (!*entryList) {
        UInt32 methodCount = mMethodCount;
        if (type == EntryType_Constructor) {
            //delete functions of IInterface
            methodCount -= mIFList[0].mDesc->mMethodCount;

            //delete functions of IClassObject
            methodCount -= mIFList[1].mDesc->mMethodCount;
        }

        IFIndexEntry* ifList = NULL;
        UInt32 listCount = 0;;

        if (type == EntryType_Constructor) {
            //the index of customer class object interface is 2
            ifList = &mIFList[2];
            listCount = mIFCount - 2;
        }
        else {
            ifList = mIFList;
            listCount = mIFCount;
        }

        *entryList = new CEntryList(type,
                mDesc, methodCount, mClsModule, ifList, listCount, this);
        if (*entryList) {
            (*entryList)->AddRef();
        }
        else {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(type);

    return ec;
}

ECode CClassInfo::GetAllMethodInfos(
    /* [out] */ ArrayOf<IMethodInfo *>* methodInfos)
{
    ECode ec = AcquireMethodList();
    if (FAILED(ec)) return ec;

    return mMethodList->GetAllObjInfos((PTypeInfos)methodInfos);
}

ECode CClassInfo::GetMethodInfo(
    /* [in] */ const String& name,
    /* [in] */ const String& signature,
    /* [out] */ IMethodInfo** methodInfo)
{
    if (name.IsNull() || signature.IsNull() || !methodInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!mMethodCount) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec = AcquireMethodList();
    if (FAILED(ec)) return ec;

    String strName = name + signature;
    return mMethodList->AcquireObjByName(strName, (IInterface **)methodInfo);
}

ECode CClassInfo::GetCallbackMethodCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mCBMethodCount;

    return NOERROR;
}

ECode CClassInfo::AcquireCBMethodList()
{
    ECode ec = NOERROR;
    g_objInfoList.LockHashTable(EntryType_CBMethod);
    if (!mCBMethodList) {
        mCBMethodList = new CEntryList(EntryType_CBMethod,
                mClassDirEntry->mDesc, mCBMethodCount, mClsModule,
                mCBIFList, mCBIFCount, this);
        if (!mCBMethodList) {
            ec = E_OUT_OF_MEMORY;
        }
    }
    g_objInfoList.UnlockHashTable(EntryType_CBMethod);

    return ec;
}

ECode CClassInfo::GetAllCallbackMethodInfos(
    /* [out] */ ArrayOf<ICallbackMethodInfo*>* callbackMethodInfos)
{
    ECode ec = AcquireCBMethodList();
    if (FAILED(ec)) return ec;

    return mCBMethodList->GetAllObjInfos((PTypeInfos)callbackMethodInfos);
}

ECode CClassInfo::GetCallbackMethodInfo(
    /* [in] */ const String& name,
    /* [out] */ ICallbackMethodInfo** callbackMethodInfo)
{
    if (name.IsNull() || !callbackMethodInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!mCBMethodCount) {
        return E_DOES_NOT_EXIST;
    }

    ECode ec =AcquireCBMethodList();
    if (FAILED(ec)) return ec;

    return mCBMethodList->AcquireObjByName(name, (IInterface **)callbackMethodInfo);
}

ECode CClassInfo::RemoveAllCallbackHandlers(
    /* [in] */ PInterface server)
{
    return _CObject_RemoveAllCallbacks(server);
}

ECode CClassInfo::CreateObjInRgm(
    /* [in] */ PRegime rgm,
    /* [out] */ PInterface* object)
{
    Int32 index = getCIFAddr(mBase, mDesc->mInterfaces, 0)->mIndex;
    InterfaceDirEntry* ifDir = getInterfaceDirAddr(mBase,
            mClsMod->mInterfaceDirs, index);
    EIID iid = adjustInterfaceDescAddr(mBase, ifDir->mDesc)->mIID;

    return _CObject_CreateInstance(mClsId, rgm, iid, object);
}

ECode CClassInfo::CreateObject(
    /* [out] */ PInterface* object)
{
    if (!object) {
        return E_INVALID_ARGUMENT;
    }

    return CreateObjInRgm(RGM_SAME_DOMAIN, object);
}

ECode CClassInfo::CreateObjectInRegime(
    /* [in] */ PRegime rgm,
    /* [out] */ PInterface* object)
{
    if (IS_INVALID_REGIME(rgm) || !object) {
        return E_INVALID_ARGUMENT;
    }

    return CreateObjInRgm(rgm, object);
}

ECode CClassInfo::CreateIFList()
{
    if (mIFList) {
        return NOERROR;
    }

    UInt32* indexList =
            (UInt32 *)alloca(mClsMod->mInterfaceCount * sizeof(UInt32));
    if (indexList == NULL) {
        return E_OUT_OF_MEMORY;
    }

    IFIndexEntry* allIFList = (IFIndexEntry *)
            alloca(mClsMod->mInterfaceCount * sizeof(IFIndexEntry));
    if (allIFList == NULL) {
        return E_OUT_OF_MEMORY;
    }

    Int32 i, j, k, n = 0, iNo, listCount = 0;
    UInt32 index = 0, eventNum = 1, beginNo = METHOD_START_NO;
    Boolean isCallBack = FALSE;
    mIFCount = 0;
    mCBIFCount = 0;
    ClassInterface* cifDir = NULL;
    InterfaceDirEntry* ifDir = NULL;

    for (i = 0; i < mDesc->mInterfaceCount; i++) {
        cifDir = getCIFAddr(mBase, mDesc->mInterfaces, i);
        if (cifDir->mAttribs & ClassInterfaceAttrib_callback) {
            isCallBack = TRUE;
        }
        else {
            isCallBack = FALSE;
        }

        index = cifDir->mIndex;

        iNo = 0;
        while (index != 0) {
            indexList[iNo++] = index;
            ifDir = getInterfaceDirAddr(mBase,
                    mClsMod->mInterfaceDirs, index);
            index = adjustInterfaceDescAddr(mBase, ifDir->mDesc)->mParentIndex;
        }

        indexList[iNo] = 0;

        //Save the indexList to mIFList
        for (j = iNo; j >= 0; j--) {
            index = indexList[j];
            if (listCount != 0) {
                //If the same inteface in list, continue
                for (k = 0; k < listCount; k++) {
                    if (allIFList[k].mIndex == index) {
                        beginNo = allIFList[k].mBeginNo
                                + allIFList[k].mDesc->mMethodCount;
                        if (!isCallBack) {
                            if (!(allIFList[k].mAttribs & IFAttrib_normal)) {
                                mIFCount++;
                                allIFList[k].mAttribs |= IFAttrib_normal;
                            }
                        }
                        else {
                            if (!(allIFList[k].mAttribs & IFAttrib_callback)) {
                                mCBIFCount++;
                                allIFList[k].mAttribs |= IFAttrib_callback;
                            }
                        }

                        break;
                    }
                }

                if (k < listCount) {
                    continue;
                }
            }

            allIFList[listCount].mIndex = index;
            allIFList[listCount].mBeginNo = beginNo;
            ifDir = getInterfaceDirAddr(mBase, mClsMod->mInterfaceDirs, index);
            allIFList[listCount].mName = adjustNameAddr(mBase, ifDir->mName);
            allIFList[listCount].mNameSpace = adjustNameAddr(mBase, ifDir->mNameSpace);
            allIFList[listCount].mDesc = adjustInterfaceDescAddr(mBase, ifDir->mDesc);

            if (!isCallBack) {
                mIFCount++;
                allIFList[listCount].mAttribs = IFAttrib_normal;
            }
            else {
                mCBIFCount++;
                allIFList[listCount].mAttribs = IFAttrib_callback;
            }
            beginNo +=  allIFList[listCount].mDesc->mMethodCount;

            listCount++;
        }
    }

    mIFList = new IFIndexEntry[mIFCount];
    if (!mIFList) goto EExit;

    if (mCBIFCount) {
        mCBIFCount--;
        mCBIFList = new IFIndexEntry[mCBIFCount];
        if (!mCBIFList) goto EExit;
    }

    mMethodCount = 0;
    mCBMethodCount = 0;
    j = 0;
    k = 0;
    for (i = 0; i < listCount; i++) {
        if (allIFList[i].mAttribs & IFAttrib_normal) {
            memcpy(&mIFList[j], &allIFList[i], sizeof(IFIndexEntry));
            mMethodCount += allIFList[i].mDesc->mMethodCount;
            j++;
        }
        if (i && mCBIFCount && (allIFList[i].mAttribs & IFAttrib_callback)) {
            memcpy(&mCBIFList[k], &allIFList[i], sizeof(IFIndexEntry));
            mCBMethodCount += allIFList[i].mDesc->mMethodCount;
            k++;
        }
    }

    //Set Callback Method Info
    mCBMethodDesc = new CBMethodDesc[mCBMethodCount];
    if (!mCBMethodDesc) goto EExit;
    memset(mCBMethodDesc, 0, mCBMethodCount * sizeof(CBMethodDesc));

    for (i = 0; i < k; i++) {
        for (j = 0; j < mCBIFList[i].mDesc->mMethodCount; j++) {
            mCBMethodDesc[n].mDesc = getMethodDescAddr(mBase,
                    mCBIFList[i].mDesc->mMethods, j);
            mCBMethodDesc[n].mIndex = MK_METHOD_INDEX(mCBIFList[i].mIndex,
                    mCBIFList[i].mBeginNo + j);
            mCBMethodDesc[n].mEventNum = eventNum;
            eventNum++;
            n++;
        }
        eventNum += 2;
    }

    return NOERROR;

EExit:
    mIFCount = 0;
    mCBIFCount = 0;
    mMethodCount = 0;
    mCBMethodCount = 0;

    if (mIFList) {
        delete [] mIFList;
        mIFList = NULL;
    }

    if (mCBIFList) {
        delete [] mCBIFList;
        mCBIFList = NULL;
    }

    if (mCBMethodDesc) {
        delete [] mCBMethodDesc;
        mCBMethodDesc = NULL;
    }

    return E_OUT_OF_MEMORY;
}
