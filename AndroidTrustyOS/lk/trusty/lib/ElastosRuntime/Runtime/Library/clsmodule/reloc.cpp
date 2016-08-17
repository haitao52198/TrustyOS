//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include <clsbase.h>

static int sBase;

void MapFileDirEntry(
    /* [in] */ FileDirEntry* p)
{
    if (p->mPath) p->mPath += sBase;
}

void MapClassDescriptor(
    /* [in] */ ClassDescriptor* p)
{
    if (0 != p->mAggregateCount) {
        p->mAggrIndexes = (USHORT *)((int)p->mAggrIndexes + sBase);
    }
    if (0 != p->mAspectCount) {
        p->mAspectIndexes = (USHORT *)((int)p->mAspectIndexes + sBase);
    }
    if (0 != p->mClassCount) {
        p->mClassIndexes = (USHORT *)((int)p->mClassIndexes + sBase);
    }

    if (0 != p->mInterfaceCount) {
        p->mInterfaces = (ClassInterface **)((int)p->mInterfaces + sBase);

        for (int n = 0; n < p->mInterfaceCount; n++) {
            p->mInterfaces[n] = (ClassInterface *)((int)p->mInterfaces[n] + sBase);
        }
    }
}

void MapClassDirEntry(
    /* [in] */ ClassDirEntry* p)
{
    p->mName += sBase;
    if (p->mNameSpace) p->mNameSpace += sBase;

    p->mDesc = (ClassDescriptor *)((int)p->mDesc + sBase);

    MapClassDescriptor(p->mDesc);
}

void MapInterfaceConstDescriptor(
    /* [in] */ InterfaceConstDescriptor* p)
{
    p->mName += sBase;
    if (p->mType == TYPE_STRING && p->mV.mStrValue != NULL) p->mV.mStrValue += sBase;
}

void MapMethodDescriptor(
    /* [in] */ MethodDescriptor* p)
{
    p->mName += sBase;
    p->mSignature += sBase;
    if (p->mAnnotation != NULL) p->mAnnotation += sBase;

    if (0 != p->mParamCount) {
        p->mParams = (ParamDescriptor **)((int)p->mParams + sBase);

        for (int n = 0; n < p->mParamCount; n++) {
            p->mParams[n] = (ParamDescriptor *)((int)p->mParams[n] + sBase);
            p->mParams[n]->mName += sBase;

            if (p->mParams[n]->mType.mNestedType) {
                p->mParams[n]->mType.mNestedType =
                        (TypeDescriptor *)((int)p->mParams[n]->mType.mNestedType + sBase);
            }
        }
    }
}

void MapInterfaceDescriptor(
    /* [in] */ InterfaceDescriptor* p)
{
    if (0 != p->mConstCount) {
        p->mConsts = (InterfaceConstDescriptor **)((int)p->mConsts + sBase);

        for (int n = 0; n < p->mConstCount; n++) {
            p->mConsts[n] = (InterfaceConstDescriptor *)((int)p->mConsts[n] + sBase);
            MapInterfaceConstDescriptor(p->mConsts[n]);
        }
    }

    if (0 != p->mMethodCount) {
        p->mMethods = (MethodDescriptor **)((int)p->mMethods + sBase);

        for (int n = 0; n < p->mMethodCount; n++) {
            p->mMethods[n] = (MethodDescriptor *)((int)p->mMethods[n] + sBase);
            MapMethodDescriptor(p->mMethods[n]);
        }
    }
}

void MapInterfaceDirEntry(
    /* [in] */ InterfaceDirEntry* p)
{
    p->mName += sBase;
    if (p->mNameSpace) p->mNameSpace += sBase;

    p->mDesc = (InterfaceDescriptor *)((int)p->mDesc + sBase);

    MapInterfaceDescriptor(p->mDesc);
}

void MapArrayDirEntry(
    /* [in] */ ArrayDirEntry* p)
{
    if (p->mNameSpace) p->mNameSpace += sBase;

    if (p->mType.mNestedType) {
        p->mType.mNestedType = (TypeDescriptor *)((int)p->mType.mNestedType + sBase);
    }
}

void MapStructDescriptor(
    /* [in] */ StructDescriptor* p)
{
    if (0 != p->mElementCount) {
        p->mElements = (StructElement **)((int)p->mElements + sBase);

        for (int n = 0; n < p->mElementCount; n++) {
            p->mElements[n] = (StructElement *)((int)p->mElements[n] + sBase);
            p->mElements[n]->mName += sBase;

            if (p->mElements[n]->mType.mNestedType) {
                p->mElements[n]->mType.mNestedType =
                        (TypeDescriptor *)((int)p->mElements[n]->mType.mNestedType + sBase);
            }
        }
    }
}

void MapStructDirEntry(
    /* [in] */ StructDirEntry* p)
{
    p->mName += sBase;
    if (p->mNameSpace) p->mNameSpace += sBase;

    p->mDesc = (StructDescriptor *)((int)p->mDesc + sBase);

    MapStructDescriptor(p->mDesc);
}

void MapEnumDescriptor(
    /* [in] */ EnumDescriptor* p)
{
    if (0 != p->mElementCount) {
        p->mElements = (EnumElement **)((int)p->mElements + sBase);

        for (int n = 0; n < p->mElementCount; n++) {
            p->mElements[n] = (EnumElement *)((int)p->mElements[n] + sBase);
            p->mElements[n]->mName += sBase;
        }
    }
}

void MapEnumDirEntry(
    /* [in] */ EnumDirEntry* p)
{
    p->mName += sBase;
    if (p->mNameSpace) p->mNameSpace += sBase;

    p->mDesc = (EnumDescriptor *)((int)p->mDesc + sBase);

    MapEnumDescriptor(p->mDesc);
}

void MapConstDirEntry(
    /* [in] */ ConstDirEntry* p)
{
    p->mName += sBase;
    if (p->mNameSpace) p->mNameSpace += sBase;
    if (p->mType == TYPE_STRING && p->mV.mStrValue.mValue != NULL) p->mV.mStrValue.mValue += sBase;
}

void MapAliasDirEntry(
    /* [in] */ AliasDirEntry* p)
{
    p->mName += sBase;
    if (p->mNameSpace) p->mNameSpace += sBase;

    if (p->mType.mNestedType) {
        p->mType.mNestedType = (TypeDescriptor *)((int)p->mType.mNestedType + sBase);
    }
}

void DoRelocCLS(
    /* [in] */ CLSModule* p)
{
    sBase = (int)p;

    p->mName += sBase;
    if (p->mUunm) p->mUunm += sBase;
    if (p->mServiceName) p->mServiceName += sBase;
    if (p->mDefinedInterfaceCount > 0) {
        p->mDefinedInterfaceIndexes = (int *)((int)(p->mDefinedInterfaceIndexes) + sBase);
    }

    if (0 != p->mFileCount) {
        p->mFileDirs = (FileDirEntry **)((int)p->mFileDirs + sBase);

        for (int n = 0; n < p->mFileCount; n++) {
            p->mFileDirs[n] = (FileDirEntry *)((int)p->mFileDirs[n] + sBase);
            MapFileDirEntry(p->mFileDirs[n]);
        }
    }
    else {
        p->mFileDirs = NULL;
    }

    if (0 != p->mClassCount) {
        p->mClassDirs = (ClassDirEntry **)((int)p->mClassDirs + sBase);

        for (int n = 0; n < p->mClassCount; n++) {
            p->mClassDirs[n] = (ClassDirEntry *)((int)p->mClassDirs[n] + sBase);
            MapClassDirEntry(p->mClassDirs[n]);
        }
    }
    else {
        p->mClassDirs = NULL;
    }

    if (0 != p->mInterfaceCount) {
        p->mInterfaceDirs = (InterfaceDirEntry **)((int)p->mInterfaceDirs + sBase);

        for (int n = 0; n < p->mInterfaceCount; n++) {
            p->mInterfaceDirs[n] = (InterfaceDirEntry *)((int)p->mInterfaceDirs[n] + sBase);
            MapInterfaceDirEntry(p->mInterfaceDirs[n]);
        }
    }
    else {
        p->mInterfaceDirs = NULL;
    }

    if (0 != p->mArrayCount) {
        p->mArrayDirs = (ArrayDirEntry **)((int)p->mArrayDirs + sBase);

        for (int n = 0; n < p->mArrayCount; n++) {
            p->mArrayDirs[n] = (ArrayDirEntry *)((int)p->mArrayDirs[n] + sBase);
            MapArrayDirEntry(p->mArrayDirs[n]);
        }
    }
    else {
        p->mArrayDirs = NULL;
    }

    if (0 != p->mStructCount) {
        p->mStructDirs = (StructDirEntry **)((int)p->mStructDirs + sBase);

        for (int n = 0; n < p->mStructCount; n++) {
            p->mStructDirs[n] = (StructDirEntry *)((int)p->mStructDirs[n] + sBase);
            MapStructDirEntry(p->mStructDirs[n]);
        }
    }
    else {
        p->mStructDirs = NULL;
    }

    if (0 != p->mEnumCount) {
        p->mEnumDirs = (EnumDirEntry **)((int)p->mEnumDirs + sBase);

        for (int n = 0; n < p->mEnumCount; n++) {
            p->mEnumDirs[n] = (EnumDirEntry *)((int)p->mEnumDirs[n] + sBase);
            MapEnumDirEntry(p->mEnumDirs[n]);
        }
    }
    else {
        p->mEnumDirs = NULL;
    }

    if (0 != p->mConstCount) {
        p->mConstDirs = (ConstDirEntry **)((int)p->mConstDirs + sBase);

        for (int n = 0; n < p->mConstCount; n++) {
            p->mConstDirs[n] = (ConstDirEntry *)((int)p->mConstDirs[n] + sBase);
            MapConstDirEntry(p->mConstDirs[n]);
        }
    }
    else {
        p->mConstDirs = NULL;
    }

    if (0 != p->mAliasCount) {
        p->mAliasDirs = (AliasDirEntry **)((int)p->mAliasDirs + sBase);

        for (int n = 0; n < p->mAliasCount; n++) {
            p->mAliasDirs[n] = (AliasDirEntry *)((int)p->mAliasDirs[n] + sBase);
            MapAliasDirEntry(p->mAliasDirs[n]);
        }
    }
    else {
        p->mAliasDirs = NULL;
    }

    if (0 != p->mLibraryCount) {
        p->mLibraryNames = (char **)((int)p->mLibraryNames + sBase);
        for (int n = 0; n < p->mLibraryCount; n++) {
            p->mLibraryNames[n] += sBase;
        }
    }
    else {
        p->mLibraryNames = NULL;
    }
}

int RelocFlattedCLS(
    /* [in] */ const void* src,
    /* [in] */ int size,
    /* [out] */ CLSModule** outDest)
{
    CLSModule* srcModule = (CLSModule *)src;

    CLSModule* destModule = (CLSModule *)new char[srcModule->mSize];
    if (!destModule) _ReturnError(CLSError_OutOfMemory);

    if (srcModule->mAttribs & CARAttrib_compress) {
        int n = UncompressCLS(src, size, destModule);
        if (n != srcModule->mSize) {
            delete [] (char *)destModule;
            _ReturnError(CLSError_FormatSize);
        }
    }
    else {
        memcpy(destModule, srcModule, size);
    }

    DoRelocCLS(destModule);
    destModule->mAttribs |= CARAttrib_inheap;

    *outDest = destModule;
    _ReturnOK(CLS_NoError);
}

int DisposeFlattedCLS(
    /* [in] */ void* dest)
{
    if (((CLSModule *)dest)->mAttribs & CARAttrib_inheap) {
        delete [] (char *)dest;
    }
    _ReturnOK(CLS_NoError);
}
