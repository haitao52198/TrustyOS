//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CMETHODINFO_H__
#define __CMETHODINFO_H__

#include "CClsModule.h"
#include "CEntryList.h"

class CMethodInfo
    : public ElLightRefBase
    , public IMethodInfo
{
public:
    CMethodInfo(
        /* [in] */ CClsModule* clsModule,
        /* [in] */ MethodDescriptor* methodDescriptor,
        /* [in] */ UInt32 index);

    virtual ~CMethodInfo();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI Init();

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

    CARAPI Invoke(
        /* [in] */ PInterface target,
        /* [in] */ IArgumentList* argumentList);

    CARAPI SetParamElem(
        /* [in] */ ParamDescriptor* paramDescriptor,
        /* [in] */ ParmElement* parmElement);

    CARAPI InitParmElement();

    CARAPI InitParamInfos();

    CARAPI CreateFunctionArgumentList(
        /* [in] */ IFunctionInfo* functionInfo,
        /* [in] */ Boolean isMethodInfo,
        /* [out] */ IArgumentList** argumentList);

    CARAPI CreateCBArgumentList(
        /* [in] */ ICallbackMethodInfo* callbackMethodInfo,
        /* [out] */ ICallbackArgumentList** cbArgumentList);

public:
    MethodDescriptor*   mMethodDescriptor;
    UInt32              mIndex;
    CLSModule*          mClsMod;
    AutoPtr<CClsModule> mClsModule;

private:
    ArrayOf<IParamInfo *>*  mParameterInfos;
    ParmElement*            mParamElem;
    UInt32                  mParamBufSize;
    Int32                   mBase;
};

#endif // __CMETHODINFO_H__
