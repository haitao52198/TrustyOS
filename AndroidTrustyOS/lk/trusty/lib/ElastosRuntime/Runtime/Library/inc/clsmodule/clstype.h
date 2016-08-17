//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CLSTYPE_H__
#define __CLSTYPE_H__

#ifndef _WINDEF_H
typedef byte                BYTE;
typedef unsigned short      WORD;
typedef unsigned int        DWORD;
typedef int                 BOOL;
typedef unsigned short      USHORT;
typedef unsigned int        UINT;

#ifdef __ELATYPES_H__
typedef _ELASTOS EMuid GUID;
typedef GUID *PGUID;
typedef GUID IID;
typedef GUID *PIID;
typedef GUID CLSID;

#define REFGUID  const GUID &
#define RIID   const _ELASTOS InterfaceID &
#define REFCLSID const CLSID &
#define REZCLSID const _ELASTOS ClassID &
#endif // __ELATYPES_H__

#endif //_WINDEF_H

typedef enum CARAttrib
{
    CARAttrib_hasSinkObject = 0x00000001,

    CARAttrib_compress      = 0x00100000,
    CARAttrib_inheap        = 0x00200000, // set when load from .cls file

    CARAttrib_library       = 0x10000000,

    CARAttrib_AttrMask      = 0x0000ffff,
    CARAttrib_AutoMask      = 0x00ff0000,
    CARAttrib_TypeMask      = 0xff000000,
} CARAttrib;

#define CAR_ATTR(a)         ((a) & CARAttrib_AttrMask)
#define CAR_TYPE(a)         ((a) & CARAttrib_TypeMask)

typedef enum CARDataType
{
    Type_Char16 = 0x01,  Type_Char32,
    Type_Boolean,        Type_Float,        Type_Double,
    Type_Int8,           Type_Int16 ,       Type_Int32,       Type_Int64,
    Type_Byte,           Type_UInt16,       Type_UInt32,      Type_UInt64,
    Type_EMuid,          Type_ECode,        Type_EGuid,       Type_EventHandler,
    Type_Array,          Type_PVoid,         Type_const,
    Type_interface,      Type_struct,       Type_enum,        Type_alias,

    Type_String,         Type_ArrayOf

} CARDataType;

typedef enum ClassAttrib
{
    ClassAttrib_final           = 0x00000001,
    ClassAttrib_local           = 0x00000002,
    ClassAttrib_hasTrivialCtor  = 0x00000004,
    ClassAttrib_singleton       = 0x00000020,
    ClassAttrib_aggregate       = 0x00000040,
    ClassAttrib_aspect          = 0x00000080,
    ClassAttrib_freethreaded    = 0x00000400,
    ClassAttrib_naked           = 0x00000800,
    ClassAttrib_hasDefaultCtor  = 0x00001000,
    ClassAttrib_private         = 0x00002000,
    ClassAttrib_specialAspect   = 0x00004000,
    ClassAttrib_classlocal      = 0x00008000,

    ClassAttrib_hasparent       = 0x00010000,
    ClassAttrib_hascallback     = 0x00020000,
    ClassAttrib_hasvirtual      = 0x00040000,
    ClassAttrib_hasctor         = 0x00080000,
    ClassAttrib_hascoalescence  = 0x00100000,
    ClassAttrib_hasasynchronous = 0x00200000,
    ClassAttrib_hasfilter       = 0x00400000,
    ClassAttrib_defined         = 0x00800000,

    ClassAttrib_t_sink          = 0x01000000,
    ClassAttrib_t_clsobj        = 0x02000000,
    ClassAttrib_t_normalClass   = 0x08000000,
    ClassAttrib_t_aspect        = 0x10000000,
    ClassAttrib_t_generic       = 0x20000000,
    ClassAttrib_t_regime        = 0x40000000,
    ClassAttrib_t_external      = 0x80000000,

    ClassAttrib_AttrMask        = 0x0000ffff,
    ClassAttrib_TypeMask        = 0xff000000,
    ClassAttrib_AutoMask        = 0x00ff0000,
} ClassAttrib;

#define CLASS_ATTR(a)           ((a) & ClassAttrib_AttrMask)
#define CLASS_TYPE(a)           ((a & ~ClassAttrib_t_external) & ClassAttrib_TypeMask)

typedef enum ClassInterfaceAttrib
{
    ClassInterfaceAttrib_virtual        = 0x0001,
    ClassInterfaceAttrib_callback       = 0x0002,
    ClassInterfaceAttrib_hidden         = 0x0004,
    ClassInterfaceAttrib_sink           = 0x0008,
    ClassInterfaceAttrib_privileged     = 0x0010,
    ClassInterfaceAttrib_handler        = 0x0020,
    ClassInterfaceAttrib_async          = 0x0040,
    ClassInterfaceAttrib_delegate       = 0x0080,

    ClassInterfaceAttrib_inherited      = 0x0100, // inherited from other class
    ClassInterfaceAttrib_autoadded      = 0x0200, // added by carc automatically
    ClassInterfaceAttrib_filter         = 0x0400, // callback filtering interface

    ClassInterfaceAttrib_callbacksink   = 0x1000,
    ClassInterfaceAttrib_delegatesink   = 0x2000,

    ClassInterface_AttrMask             = 0x00ff,

    ClassInterfaceAttrib_outer          = ClassInterfaceAttrib_autoadded |
                                          ClassInterfaceAttrib_inherited,
} ClassInterfaceAttrib;

typedef enum InterfaceAttrib
{
    InterfaceAttrib_local       = 0x00000001,
    InterfaceAttrib_sink        = 0x00000002,
    InterfaceAttrib_parcelable  = 0x00000004,
    InterfaceAttrib_oneway      = 0x00000008,

    InterfaceAttrib_dual        = 0x00100000, // for scriptable only
    InterfaceAttrib_defined     = 0x00200000,
    InterfaceAttrib_clsobj      = 0x00400000,
    InterfaceAttrib_generic     = 0x00800000,

    InterfaceAttrib_t_normal    = 0x01000000,
    InterfaceAttrib_t_callbacks = 0x02000000,
    InterfaceAttrib_t_delegates = 0x04000000,
    InterfaceAttrib_t_external  = 0x08000000,

    InterfaceAttrib_TypeMask    = 0xff000000,
    InterfaceAttrib_AutoMask    = 0x00ff0000,
    InterfaceAttrib_AttrMask    = 0x0000ffff,
} InterfaceAttrib;

typedef enum StructAttrib
{
    StructAttrib_t_external     = 0x01000000,
} StructAttrib;

typedef enum EnumAttrib
{
    EnumAttrib_t_external     = 0x01000000,
} EnumAttrib;

typedef enum MethodAttrib
{
    MethodAttrib_TrivialCtor        = 0x00000001,
    MethodAttrib_DefaultCtor        = 0x00000002,
    MethodAttrib_NonDefaultCtor     = 0x00000004,
    MethodAttrib_WithCoalescence    = 0x00000008,
    MethodAttrib_Oneway             = 0x00000010,
} MethodAttrib;

#define INTERFACE_ATTR(a)       ((a) & InterfaceAttrib_AttrMask)
#define INTERFACE_TYPE(a)       ((a & ~InterfaceAttrib_t_external) & InterfaceAttrib_TypeMask)

typedef enum ParamAttrib
{
    ParamAttrib_in          = 0x01,
    ParamAttrib_out         = 0x02,
    ParamAttrib_callee      = 0x04,
}ParamAttrib;

#endif // __CLSTYPE_H__
