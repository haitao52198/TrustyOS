//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================
#ifndef __LOCMOD_H__
#define __LOCMOD_H__

#include <clsinfo.h>
#include <elcontainer.h>

using namespace Elastos;

typedef ECode (__stdcall *PDLLGETCLASSOBJECT)(REMuid, REIID, PInterface*);
typedef ECode (__stdcall *PDLLCANUNLOADNOW)();

typedef struct LocalModule : DLinkNode {
    String              mUunm;
    Void*               mIModule;
    PDLLGETCLASSOBJECT  mDllGetClassObjectFunc;
    PDLLCANUNLOADNOW    mDllCanUnloadNowFunc;
    CIClassInfo*        mClassInfo;
    Int32               mAskCount;
} LocalModule;

ECode  AcquireClassObjectFromLocalModule(
    /* [in] */ RClassID rclsid,
    /* [in] */ REIID riid,
    /* [out] */ PInterface* object);

#endif // __LOCMOD_H__
