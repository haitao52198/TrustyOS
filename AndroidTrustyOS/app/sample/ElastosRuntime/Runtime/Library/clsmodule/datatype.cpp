//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include <stdio.h>
#include <assert.h>

#include "clsbase.h"

int GetOriginalType(
    /* [in] */ const CLSModule* module,
    /* [in] */ const TypeDescriptor* srcDescriptor,
    /* [in] */ TypeDescriptor* destDescriptor)
{
    destDescriptor->mPointer = srcDescriptor->mPointer;
    destDescriptor->mUnsigned = srcDescriptor->mUnsigned;

    while (srcDescriptor->mType == Type_alias) {
        srcDescriptor = &module->mAliasDirs[srcDescriptor->mIndex]->mType;
        destDescriptor->mPointer += srcDescriptor->mPointer;
        destDescriptor->mUnsigned |= srcDescriptor->mUnsigned;
    }

    destDescriptor->mType = srcDescriptor->mType;
    destDescriptor->mIndex = srcDescriptor->mIndex;
    destDescriptor->mNestedType = srcDescriptor->mNestedType;

    _ReturnOK(CLS_NoError);
}

int GetArrayBaseType(
    /* [in] */ const CLSModule* module,
    /* [in] */ const TypeDescriptor* srcDescriptor,
    /* [in] */ TypeDescriptor* destDescriptor)
{
    TypeDescriptor* type = (TypeDescriptor *)srcDescriptor;

    while (Type_Array == type->mType) {
        type = &module->mArrayDirs[type->mIndex]->mType;
    }

    memcpy(destDescriptor, type, sizeof(TypeDescriptor));

    _ReturnOK(CLS_NoError);
}

BOOL IsEqualType(
    /* [in] */ const CLSModule* module,
    /* [in] */ const TypeDescriptor* descriptor1,
    /* [in] */ const TypeDescriptor* descriptor2)
{
    TypeDescriptor src, dest;

    GetOriginalType(module, descriptor1, &src);
    GetOriginalType(module, descriptor2, &dest);

    _Return (!memcmp(&src, &dest, sizeof(src)));
}
