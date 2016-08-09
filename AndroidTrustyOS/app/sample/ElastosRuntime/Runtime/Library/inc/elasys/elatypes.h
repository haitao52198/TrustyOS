//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __ELATYPES_H__
#define __ELATYPES_H__

#include <stddef.h>
#include <elans.h>

#ifndef _win32
#include <stdint.h>
#endif

extern "C" {

#if defined(_MSC_VER) || defined(_EVC)
#define DECLSPEC_SELECTANY      __declspec(selectany)
#define DECLSPEC_NOVTABLE       __declspec(novtable)
#define DECLSPEC_UUID(x)        __declspec(uuid(x))
#define DECL_NAKED              __declspec(naked)
#define DECL_PACKED
#define DECL_SECTION(s)
#elif defined(_GNUC)
#define DECL_REGPARM(n)         __attribute__ ((regparm(n)))
#define DECL_SECTION(s)         __attribute__ ((section(s)))
#define DECL_PACKED             __attribute__ ((packed))
#define DECL_ALIGN(n)           __attribute__ ((aligned(n)))
#define DECL_NORETURN           __attribute__ ((noreturn))
#define DECL_NAKED
#endif // defined(_MSC_VER)

//typedef __int64 hyper;

#define INFINITE -1     // Infinite timeout

typedef unsigned char   byte;
typedef int             bool_t;
#ifndef _UINT_T_DEFINED_
#define _UINT_T_DEFINED_
typedef unsigned int    uint_t;
#endif
typedef void *          virtaddr_t;
typedef unsigned long   ulong_t;
typedef ulong_t         oid_t;
typedef unsigned char   uchar_t;
typedef uchar_t         byte_t;

_ELASTOS_NAMESPACE_BEGIN

/** @addtogroup BaseTypesRef
  *   @{
  */
typedef void Void;
/** @} */

/** @addtogroup CARTypesRef
  *   @{
  */
typedef void * PVoid;

typedef signed char Int8;
typedef unsigned char UInt8;
typedef UInt8 Byte;
typedef unsigned short Char16;
typedef unsigned int Char32;
typedef signed short Int16;
typedef unsigned short UInt16;
typedef int Int32;
typedef unsigned int UInt32;
#ifdef __GNUC__
typedef long long Int64;
typedef unsigned long long UInt64;
#else
typedef __int64 Int64;
typedef unsigned __int64 UInt64;
#endif
typedef float Float;
typedef double Double;
typedef unsigned char Boolean;
typedef Int32 ECode;
/** @} */

typedef Char16 *PChar16;
typedef Char32 *PChar32;
typedef Int8 *PInt8;
typedef Byte *PByte;
typedef UInt8 *PUInt8;
typedef Int16 *PInt16;
typedef UInt16 *PUInt16;
typedef Int32 *PInt32;
typedef UInt32 *PUInt32;
typedef Int64 *PInt64;
typedef UInt64 *PUInt64;
typedef Float *PFloat;
typedef Double *PDouble;
typedef Boolean *PBoolean;

/** @addtogroup CARTypesRef
  *   @{
  */
typedef UInt32 Handle32;
typedef UInt64 Handle64;
/** @} */

typedef Handle32 *PHandle32;
typedef Handle64 *PHandle64;

/** @addtogroup CARTypesRef
  *   @{
  */
//System Type
typedef Int32 MemorySize;
typedef UInt32 Address;

/** @} */

/** @addtogroup CARTypesRef
  *   @{
  */
typedef struct _EMuid
{
    UInt32  mData1;
    UInt16  mData2;
    UInt16  mData3;
    UInt8   mData4[8];
} EMuid, *PEMuid;

typedef struct _EGuid
{
    EMuid   mClsid;
    char*   mUunm;
    UInt32  mCarcode;
} EGuid, *PEGuid;

typedef EGuid ClassID;
typedef EMuid InterfaceID;
typedef InterfaceID EIID;
/** @} */

typedef ClassID *PClassID;
typedef InterfaceID *PInterfaceID;
typedef EIID* PEIID;

typedef const EMuid&        REMuid;
typedef const ClassID&      RClassID;
typedef const InterfaceID&  REIID;
typedef const InterfaceID&  RInterfaceID;

_ELASTOS_NAMESPACE_END

#if defined(_linux) || defined(_mips) && defined(_GNUC)
#define __cdecl
#define __stdcall
#endif

#define CDECL                   __cdecl
#define STDCALL                 __stdcall

#define EXTERN      extern
#define STATIC      static
#define CONST       const
#define VOLATILE    volatile

//---- EXTERN_C ----
#define EXTERN_C            extern "C"
#define EXTERN_C_BEGIN      EXTERN_C {
#define EXTERN_C_END        }

//---- ECO_PUBLIC & ECO_LOCAL ----
#if __GNUC__ >= 4
#define ECO_PUBLIC __attribute__ ((visibility ("default")))
#define ECO_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define ECO_PUBLIC
#define ECO_LOCAL
#endif

//---- INIT_PROI ----
#define INIT_PROI_1 __attribute__ ((init_priority (500)))
#define INIT_PROI_2 __attribute__ ((init_priority (1000)))
#define INIT_PROI_3 __attribute__ ((init_priority (1500)))
#define INIT_PROI_4 __attribute__ ((init_priority (2000)))
#define INIT_PROI_5 __attribute__ ((init_priority (2500)))
#define INIT_PROI_6 __attribute__ ((init_priority (3000)))
#define INIT_PROI_7 __attribute__ ((init_priority (3500)))

//---- CAR_INLINE ----
#ifdef _GNUC
#define CAR_INLINE inline
#elif defined(_MSC_VER)
#define CAR_INLINE __inline
#elif defined(DIAB_COMPILER)
#define CAR_INLINE /* only pragmas supported, don't bother */
#endif

// ---- Boolean Value ----
#ifdef _GNUC
  #ifndef TRUE
  #define TRUE    ((_ELASTOS Boolean)1)
  #endif
  #ifndef FALSE
  #define FALSE   ((_ELASTOS Boolean)0)
  #endif
#else
  #ifndef TRUE
  #define TRUE    ((_ELASTOS Boolean)1)
  #endif
  #ifndef FALSE
  #define FALSE   ((_ELASTOS Boolean)0)
  #endif
#endif

//---- ASM ----
#if defined(_EVC) && defined(_mips)
EXTERN_C void __asm(char*, ...);
#endif

#ifdef _GNUC
#define __asm           __asm__
#define _asm            __asm
#define ASM_VOLATILE    __asm__ __volatile__
#define ASM             ASM_VOLATILE
#elif defined(_EVC) || defined(_MSVC)
#define __asm           __asm
#define _asm            __asm
#define ASM_VOLATILE    __asm
#define ASM             ASM_VOLATILE
#else
#error unknown compiler
#endif

#define interface           struct
#ifdef _GNUC
#define CAR_INTERFACE(x)    interface
#else
#define CAR_INTERFACE(x)    interface DECLSPEC_UUID(x) DECLSPEC_NOVTABLE
#endif

#ifdef _CALLBACK_KEYWORD_CHECK
#define CarClass(name)      class name : public _##name name##_CallbackKeyword_Checking
#else
#define CarClass(name)      class name : public _##name
#endif

// ---- API Call ----
#define ELAPICALLTYPE       CDECL
#define CARAPICALLTYPE      STDCALL

#define ELAPI               EXTERN_C _ELASTOS ECode ELAPICALLTYPE
#define ELAPI_(type)        EXTERN_C type ELAPICALLTYPE

#define CARAPI              _ELASTOS ECode CARAPICALLTYPE
#define CARAPI_(type)       type CARAPICALLTYPE

#define ELFUNCCALLTYPE      CDECL
#define ELFUNC              _ELASTOS ECode ELFUNCCALLTYPE
#define ELFUNC_(type)       type ELFUNCCALLTYPE

}

#endif // __ELATYPES_H__
