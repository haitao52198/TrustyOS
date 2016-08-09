//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CEntryList.h"
#include "CModuleInfo.h"

CObjInfoList g_objInfoList;

ELAPI _CReflector_AcquireModuleInfo(
    /* [in] */ const String& name,
    /* [out] */ IModuleInfo** moduleInfo)
{
    return g_objInfoList.AcquireModuleInfo(name, moduleInfo);
}

ELAPI _CReflector_AcquireIntrinsicTypeInfo(
    /* [in] */ CarDataType intrinsicType,
    /* [out] */ IDataTypeInfo** intrinsicTypeInfo)
{
    return g_objInfoList.AcquireIntrinsicInfo(intrinsicType, intrinsicTypeInfo);
}

ELAPI _CReflector_AcquireEnumInfo(
    /* [in] */ const String& name,
    /* [in] */ ArrayOf<String>* itemNames,
    /* [in] */ ArrayOf<Int32>* itemValues,
    /* [out] */ IEnumInfo** enumInfo)
{
    return g_objInfoList.AcquireDynamicEnumInfo(name, itemNames,
            itemValues, enumInfo);
}

ELAPI _CReflector_AcquireCppVectorInfo(
    /* [in] */ IDataTypeInfo* elementTypeInfo,
    /* [in] */ Int32 length,
    /* [out] */ ICppVectorInfo** cppVectorInfo)
{
    return g_objInfoList.AcquireCppVectorInfo(elementTypeInfo,
            length, cppVectorInfo);
}

ELAPI _CReflector_AcquireStructInfo(
    /* [in] */ const String& name,
    /* [in] */ ArrayOf<String>* fieldNames,
    /* [in] */ ArrayOf<IDataTypeInfo *>* fieldTypeInfos,
    /* [out] */ IStructInfo** structInfo)
{
    return g_objInfoList.AcquireDynamicStructInfo(name, fieldNames,
            fieldTypeInfos, structInfo);
}

ELAPI _CReflector_AcquireCarArrayInfo(
    /* [in] */ CarDataType quintetType,
    /* [in] */ IDataTypeInfo* elementTypeInfo,
    /* [out] */ ICarArrayInfo** carArrayInfo)
{
    return g_objInfoList.AcquireCarArrayInfo(quintetType, elementTypeInfo,
            carArrayInfo);
}

ELAPI _CObject_ReflectModuleInfo(
    /* [in] */ PInterface object,
    /* [out] */ IModuleInfo** moduleInfo)
{
    if (!object || !moduleInfo) {
        return E_INVALID_ARGUMENT;
    }

    IObject* iObject = (IObject*)object->Probe(EIID_IObject);
    if (iObject == NULL) return E_NO_INTERFACE;

    ClassID clsid;
    ECode ec = iObject->GetClassID(&clsid);
    if (FAILED(ec)) {
        return E_NO_INTERFACE;
    }

    return _CReflector_AcquireModuleInfo(String(clsid.mUunm), moduleInfo);
}

ELAPI _CObject_ReflectClassInfo(
    /* [in] */ PInterface object,
    /* [out] */ IClassInfo** classInfo)
{
    if (!object || !classInfo) {
        return E_INVALID_ARGUMENT;
    }

    IObject* iObject = (IObject*)object->Probe(EIID_IObject);
    if (iObject == NULL) return E_NO_INTERFACE;

    ClassID clsid;
    ECode ec = iObject->GetClassID(&clsid);
    if (FAILED(ec)) {
        return E_NO_INTERFACE;
    }

    AutoPtr<IModuleInfo> iModuleInfo;
    ec = g_objInfoList.AcquireModuleInfo(String(clsid.mUunm),
            (IModuleInfo**)&iModuleInfo);
    if (FAILED(ec)) {
        return ec;
    }

    CModuleInfo* moduleInfo = (CModuleInfo*)iModuleInfo.Get();

    ClassDirEntry* classDir = NULL;
    ClassDescriptor* clsDesc = NULL;
    Int32 base = moduleInfo->mClsModule->mBase;

    *classInfo = NULL;
    for (Int32 i = 0; i < moduleInfo->mClsMod->mClassCount; i++) {
        classDir = getClassDirAddr(base, moduleInfo->mClsMod->mClassDirs, i);
        clsDesc = adjustClassDescAddr(base, classDir->mDesc);
        if (clsDesc->mClsid == clsid.mClsid) {
            ec = g_objInfoList.AcquireClassInfo(moduleInfo->mClsModule,
                    classDir, (IInterface **)classInfo);
            return ec;
        }
    }

    return E_DOES_NOT_EXIST;
}

ELAPI _CObject_ReflectInterfaceInfo(
    /* [in] */ PInterface object,
    /* [out] */ IInterfaceInfo** interfaceInfo)
{
    if (!object || !interfaceInfo) {
        return E_INVALID_ARGUMENT;
    }

    EIID iid;
    ECode ec = object->GetInterfaceID(object, &iid);
    if (FAILED(ec)) return E_INVALID_ARGUMENT;

    IObject* iObject = (IObject*)object->Probe(EIID_IObject);
    if (iObject == NULL) return E_NO_INTERFACE;

    ClassID clsid;
    ec = iObject->GetClassID(&clsid);
    if (FAILED(ec)) return E_INVALID_ARGUMENT;

    AutoPtr<IModuleInfo> obj;
    ec = _CReflector_AcquireModuleInfo(String(clsid.mUunm), (IModuleInfo**)&obj);
    if (FAILED(ec)) {
        return ec;
    }
    CModuleInfo* moduleInfo = (CModuleInfo*)obj.Get();

    ClassDirEntry* classDir = NULL;
    ClassDescriptor* clsDesc = NULL;
    ClassInterface* cifDir = NULL;
    InterfaceDirEntry* ifDir = NULL;
    InterfaceDescriptor* ifDesc = NULL;
    UInt16 index = 0;
    Int32 base = moduleInfo->mClsModule->mBase;

    *interfaceInfo = NULL;
    for (Int32 i = 0; i < moduleInfo->mClsMod->mClassCount; i++) {
        classDir = getClassDirAddr(base, moduleInfo->mClsMod->mClassDirs, i);
        clsDesc = adjustClassDescAddr(base, classDir->mDesc);
        //find the class
        if (clsDesc->mClsid == clsid.mClsid) {
            for (Int32 j = 0; j < clsDesc->mInterfaceCount; j++) {
                cifDir = getCIFAddr(base, clsDesc->mInterfaces, j);
                index = cifDir->mIndex;
                ifDir = getInterfaceDirAddr(base,
                        moduleInfo->mClsMod->mInterfaceDirs, index);
                ifDesc = adjustInterfaceDescAddr(base, ifDir->mDesc);
                //find the interface
                if (ifDesc->mIID == iid) {
                    ec = g_objInfoList.AcquireInterfaceInfo(
                            moduleInfo->mClsModule, index, (IInterface **)interfaceInfo);
                    return ec;
                }
            }
        }
    }

    return E_DOES_NOT_EXIST;
}
