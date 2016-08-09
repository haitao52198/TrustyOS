//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CCONSTRUCTORINFO_H__
#define __CCONSTRUCTORINFO_H__

#include "CMethodInfo.h"

class CConstructorInfo
    : public ElLightRefBase
    , public IConstructorInfo
{
public:
    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI Init(
        /* [in] */ CClsModule* clsModule,
        /* [in] */ MethodDescriptor* methodDescriptor,
        /* [in] */ UInt32 index,
        /* [in] */ ClassID* clsId);

    CARAPI GetName(
        /* [out] */ String* name);

    CARAPI GetAnnotation(
        /* [out] */ String* annotation);

    CARAPI GetParamCount(
        /* [out] */ Int32* count);

    CARAPI GetAllParamInfos(
        /* [out] */ ArrayOf<IParamInfo *>* paramInfos);

    CARAPI GetParamInfoByIndex(
        /* [in] */ Int32 index,
        /* [out] */ IParamInfo** paramInfo);

    CARAPI GetParamInfoByName(
        /* [in] */ const String& name,
        /* [out] */ IParamInfo** paramInfo);

    CARAPI CreateArgumentList(
        /* [out] */ IArgumentList** argumentList);

    CARAPI CreateObject(
        /* [in] */ IArgumentList* argumentList,
        /* [out] */ PInterface* object);

    CARAPI CreateObjectInRegime(
        /* [in] */ PRegime rgm,
        /* [in] */ IArgumentList* argumentList,
        /* [out] */ PInterface* object);

    CARAPI AcquireParamList();

    CARAPI CreateObjInRgm(
        /* [in] */ PRegime rgm,
        /* [in] */ IArgumentList* argumentList,
        /* [out] */ PInterface* object);

public:
    AutoPtr<CMethodInfo>    mMethodInfo;

    char                    mUrn2[_MAX_PATH];
    ClassID                 mInstClsId;

private:
    Int32                   mOutParamIndex;

    char                    mUrn[_MAX_PATH];
    ClassID                 mClsId;
};

#endif // __CCONSTRUCTORINFO_H__
