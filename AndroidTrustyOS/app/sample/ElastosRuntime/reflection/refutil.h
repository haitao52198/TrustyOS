//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __REFUTIL_H__
#define __REFUTIL_H__

#include <elastos.h>
#include <clsdef.h>
#include <stdlib.h>
#include <stdio.h>
#include "hashtable.h"
#include <clsbase.h>
#include "adjustaddr.h"

// #include <_pubcrt.h>

_ELASTOS_NAMESPACE_USING

//For CarDataType_ArrayOf
#define CarDataType_CarArray 100

#define MK_METHOD_INDEX(i, m)       (((i) + 1) << 16 | (m))
#define INTERFACE_INDEX(n)          (((n) >> 16) - 1)
#define METHOD_INDEX(n)             ((n) & 0x0000FFFF)

#define ROUND4(n)       (((n)+3)&~3)   // round up to multiple of 4 bytes
#define ROUND8(n)       (((n)+7)&~7)   // round up to multiple of 8 bytes

#define METHOD_START_NO              4

struct DateTypeDesc
{
    const char* mName;
    size_t mSize;
};

struct ParmElement
{
    UInt32         mPos;
    UInt32         mSize;
    CarDataType    mType;
    ParamIOAttribute mAttrib;
    Int32          mPointer;
};

struct StructFieldDesc
{
    char*       mName;
    UInt32      mPos;
    UInt32      mSize;
    CarDataType mType;
};

struct TypeAliasDesc
{
    TypeDescriptor* mTypeDesc;
    TypeDescriptor* mOrgTypeDesc;
};

typedef enum EntryType {
    EntryType_Module,
    EntryType_Class,
    EntryType_Aspect,
    EntryType_Aggregatee,
    EntryType_ClassInterface,
    EntryType_Interface,
    EntryType_Struct,
    EntryType_Enum,
    EntryType_TypeAliase,
    EntryType_Constant,
    EntryType_Constructor,
    EntryType_Method,
    EntryType_CBMethod,
    EntryType_Field,
    EntryType_EnumItem,
    EntryType_Parameter,
    EntryType_DataType,
    EntryType_Local,
    EntryType_ClsModule,
} EntryType;

struct InfoLinkNode
{
    PVoid mInfo;
    InfoLinkNode* mNext;
};

inline int StringAlignSize(const char *s)
{
    return ROUND4(strlen(s) + 1);
}

class CClsModule;

extern Boolean IsSysAlaisType(
    /* [in] */ const CClsModule* clsModule,
    /* [in] */ UInt32 index);

extern void _GetOriginalType(
    /* [in] */ const CClsModule* clsModule,
    /* [in] */ const TypeDescriptor* src,
    /* [in] */ TypeDescriptor* dest);

extern const DateTypeDesc g_cDataTypeList[];

extern CarDataType GetCarDataType(
    /* [in] */ CARDataType type);

extern UInt32 GetDataTypeSize(
    /* [in] */ const CClsModule* clsModule,
    /* [in] */ TypeDescriptor* typeDesc);

extern CarQuintetFlag DataTypeToFlag(
    /* [in] */ CarDataType type);

//extern CARDataType GetBasicType(TypeDescriptor *pTypeDesc);
//extern UInt32 GetStructSize(const CLSModule *pModule,
//    StructDescriptor *pStructDesc);

#endif // __REFUTIL_H__
