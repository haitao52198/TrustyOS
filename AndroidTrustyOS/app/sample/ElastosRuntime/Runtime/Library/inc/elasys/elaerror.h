//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __ELAERROR_H__
#define __ELAERROR_H__

//  ECODEs' layout:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-+-+-+-+-+-+-+-+---------------+-------------------------------+
//  |S| family code |   info code   |         debug code            |
//  +-+-+-+-+-+-+-+-+---------------+-------------------------------+
//
//

// Success codes
//
#ifndef NOERROR
#define NOERROR         ((_ELASTOS ECode)0x00000000L)
#endif

#define S_FALSE         ((_ELASTOS ECode)0x00000001L)

//
// Severity values
//
#define SEVERITY_SUCCESS    0
#define SEVERITY_ERROR      1

//
// Create an ECode value from component pieces
//

#define MAKE_ECODE(sev, family, code) \
        (_ELASTOS ECode)(((family << 24) | (code << 16 )) | (sev << 31))

#define MAKE_SUCCESS(family, code) \
        (_ELASTOS ECode)(((family << 24) | (code << 16 )) & 0x7FFFFFFF)

#define ERROR_DETAIL(c) \
        (_ELASTOS ECode)((c) & 0x0000FFFF)

#define ERROR(c) \
        (_ELASTOS ECode)((c) & 0xFFFF0000)

#define SUCCEEDED(x)    ((_ELASTOS ECode)(x) >= 0)
#define FAILED(x)       ((_ELASTOS ECode)(x) < 0)
//
// Define the family codes
//
#define FAMILY_RUNTIME      0x00    // CAR runtime error codes

//==========================================================================
// Macros and constants for FAMILY_RUNTIME error codes
//==========================================================================
#define RUNTIME_ERROR(c)                 MAKE_ECODE(SEVERITY_ERROR, FAMILY_RUNTIME, c)

#define E_DOES_NOT_EXIST                 RUNTIME_ERROR(0x01) // 0x80010000
#define E_INVALID_OPERATION              RUNTIME_ERROR(0x02) // 0x80020000
#define E_TIMED_OUT                      RUNTIME_ERROR(0x03) // 0x80030000
#define E_INTERRUPTED                    RUNTIME_ERROR(0x04) // 0x80040000
#define E_FILE_NOT_FOUND                 RUNTIME_ERROR(0x05) // 0x80050000
#define E_NOT_SUPPORTED                  RUNTIME_ERROR(0x06) // 0x80060000
#define E_OUT_OF_MEMORY                  RUNTIME_ERROR(0x07) // 0x80070000
#define E_INVALID_ARGUMENT               RUNTIME_ERROR(0x08) // 0x80080000
// Not implemented
#define E_NOT_IMPLEMENTED                RUNTIME_ERROR(0x09) // 0x80090000
// No such interface supported
#define E_NO_INTERFACE                   RUNTIME_ERROR(0x0A) // 0x800A0000
// Operation aborted
#define E_ABORTED                        RUNTIME_ERROR(0x0B) // 0x800B0000
// Unspecified error
#define E_FAIL                           RUNTIME_ERROR(0x0C) // 0x800C0000
// No default constructor in class
#define E_NO_DEFAULT_CTOR                RUNTIME_ERROR(0x0D) // 0x800D0000
// Class does not support aggregation (or class object is remote,
// SEVERITY_ERROR)
#define E_CLASS_NO_AGGREGATION           RUNTIME_ERROR(0x0E) // 0x800E0000
// ClassFactory cannot supply requested class
#define E_CLASS_NOT_AVAILABLE            RUNTIME_ERROR(0x0F) // 0x800F0000
//aspect cannot Aggregate aspect
#define E_ASPECT_CANNOT_AGGREGATE_ASPECT RUNTIME_ERROR(0x10) // 0x80100000
//Unaggregate failed
#define E_UNAGGREGATE_FAILED             RUNTIME_ERROR(0x11) // 0x80110000
// No generic defined
#define E_NO_GENERIC                     RUNTIME_ERROR(0x12) // 0x80120000
// Wrong generic defined
#define E_WRONG_GENERIC                  RUNTIME_ERROR(0x13) // 0x80130000
//Aspect refuse to attach
#define E_ASPECT_REFUSE_TO_ATTACH        RUNTIME_ERROR(0x14) // 0x80140000
//Aspect object has been unaggregated
#define E_ZOMBIE_ASPECT                  RUNTIME_ERROR(0x15) // 0x80150000
//Aspect has been aggregated
#define E_DUPLICATE_ASPECT               RUNTIME_ERROR(0x16) // 0x80160000
//A class with local constructor or interface cannot be created in diff process
#define E_CONFLICT_WITH_LOCAL_KEYWORD    RUNTIME_ERROR(0x17) // 0x80170000
#define E_NOT_IN_PROTECTED_ZONE          RUNTIME_ERROR(0x18) // 0x80180000
// Reference count of component is not zero
#define E_COMPONENT_CANNOT_UNLOAD_NOW    RUNTIME_ERROR(0x19) // 0x80190000
// It is not standard car component
#define E_NOT_CAR_COMPONENT              RUNTIME_ERROR(0x1A) // 0x801A0000
// Dll is not in active component list, maybe has been unloaded or hasn't been loaded.
#define E_COMPONENT_NOT_LOADED           RUNTIME_ERROR(0x1B) // 0x801B0000
#define E_REMOTE_FAIL                    RUNTIME_ERROR(0x1C) // 0x801C0000
#define E_OUT_OF_NUMBER                  RUNTIME_ERROR(0x1D) // 0x801D0000
#define E_DATAINFO_EXIST                 RUNTIME_ERROR(0x1E) // 0x801E0000
// No class information
#define E_NO_CLASS_INFO                  RUNTIME_ERROR(0x1F) // 0x801F0000
// not a export server
#define E_NO_EXPORT_OBJECT               RUNTIME_ERROR(0x20) // 0x80200000
// not import server
#define E_NO_IMPORT_OBJECT               RUNTIME_ERROR(0x21) // 0x80210000
// marshaling: data transport error
#define E_MARSHAL_DATA_TRANSPORT_ERROR   RUNTIME_ERROR(0x22) // 0x80220000
#define E_NOT_CALLBACK_THREAD            RUNTIME_ERROR(0x23) // 0x80230000
#define E_CANCLE_BOTH_EVENTS             RUNTIME_ERROR(0x24) // 0x80240000
#define E_NO_DELEGATE_REGISTERED         RUNTIME_ERROR(0x25) // 0x80250000
#define E_DELEGATE_ALREADY_REGISTERED    RUNTIME_ERROR(0x26) // 0x80260000
#define E_CALLBACK_CANCELED              RUNTIME_ERROR(0x27) // 0x80270000
#define E_CALLBACK_IS_BUSY               RUNTIME_ERROR(0x28) // 0x80280000
// there is no connection for this connection id
#define E_CONNECT_NOCONNECTION           RUNTIME_ERROR(0x29) // 0x80290000
// this implementation's limit for advisory connections has been reached
#define E_CONNECT_ADVISELIMIT            RUNTIME_ERROR(0x2A) // 0x802A0000
// connection attempt failed
#define E_CONNECT_CANNOTCONNECT          RUNTIME_ERROR(0x2B) // 0x802B0000
//// must use a derived interface to connect
#define E_CONNECT_OVERRIDDEN             RUNTIME_ERROR(0x2C) // 0x802C0000

#endif // __ELAERROR_H__
