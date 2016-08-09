//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CENTRYLIST_H__
#define __CENTRYLIST_H__

#include "CClsModule.h"
#include "CObjInfoList.h"

struct ObjElement
{
    UInt32  mIndex;
    void*   mDesc;
    char*   mName;
    char*   mNamespaceOrSignature;
    IInterface* mObject;
};

typedef enum IFAttrib
{
    IFAttrib_normal         = 0x01,
    IFAttrib_callback       = 0x02,
}IFAttrib;

struct IFIndexEntry
{
    UInt32                  mIndex;
    UInt32                  mBeginNo;
    char*                   mName;
    char*                   mNameSpace;
    InterfaceDescriptor*    mDesc;
    UInt32                  mAttribs;
};

typedef ArrayOf<IInterface *>*  PTypeInfos;

class CClassInfo;

class CEntryList : public ElLightRefBase
{
public:
    CEntryList(
        /* [in] */ EntryType type,
        /* [in] */ void* desc,
        /* [in] */ UInt32 totalCount,
        /* [in] */ CClsModule* clsModule,
        /* [in] */ IFIndexEntry* ifList = NULL,
        /* [in] */ UInt32 listCount = 0,
        /* [in] */ CClassInfo* clsInfo = NULL);

    ~CEntryList();

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI InitElemList();

    CARAPI AcquireObjByName(
        /* [in] */ const String& name,
        /* [out] */ IInterface** object);

    CARAPI AcquireObjByIndex(
        /* [in] */ UInt32 index,
        /* [out] */ IInterface** object);

    CARAPI GetAllObjInfos(
        /* [out] */ ArrayOf<IInterface *>* objInfos);

public:
    AutoPtr<CClsModule> mClsModule;
    UInt32              mTotalCount;

private:
    CLSModule*          mClsMod;
    void*               mDesc;
    EntryType           mType;
    CClassInfo*         mClsInfo;
    ObjElement*         mObjElement;
    Int32               mBase;

    IFIndexEntry*       mIFList;
    UInt32              mListCount;

    HashTable<UInt32, Type_String> mHTIndexs;
};

#endif // __CENTRYLIST_H__
