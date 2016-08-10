//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CLSDEF_H__
#define __CLSDEF_H__

#include <string.h>
#include "clstype.h"

const char * const MAGIC_STRING = "CAR ClassInfo\r\n\x1a";
const int MAGIC_STRING_LENGTH = 16;
const unsigned short MAJOR_VERSION = 5;
const unsigned short MINOR_VERSION = 0;
const int CLSMODULE_VERSION = 1;

typedef struct FileDirEntry FileDirEntry;
typedef struct ClassDirEntry ClassDirEntry;
typedef struct InterfaceDirEntry InterfaceDirEntry;
typedef struct StructDirEntry StructDirEntry;
typedef struct EnumDirEntry EnumDirEntry;
typedef struct AliasDirEntry AliasDirEntry;
typedef struct ArrayDirEntry ArrayDirEntry;
typedef struct ConstDirEntry ConstDirEntry;

const int MAX_FILE_NUMBER = 64;
const int MAX_CLASS_NUMBER = 4096 + 1024;
const int MAX_INTERFACE_NUMBER = 8192;
const int MAX_DEFINED_INTERFACE_NUMBER = 8192;
const int MAX_STRUCT_NUMBER = 4096;
const int MAX_ENUM_NUMBER = 4096;
const int MAX_ALIAS_NUMBER = 4096;
const int MAX_LIBRARY_NUMBER = 32;
const int MAX_ARRAY_NUMBER = 4096;
const int MAX_CONST_NUMBER = 4096;

const unsigned int STRUCT_MAX_ALIGN_SIZE = 4;

typedef struct CLSModule
{
    char                mMagic[MAGIC_STRING_LENGTH];
    unsigned char       mMajorVersion; // Major version of given version(*.*)
    unsigned char       mMinorVersion; // Minor version
    int                 mCLSModuleVersion; //version of this CLSModule struct
    int                 mSize;
    unsigned int        mChecksum;
    unsigned int        mBarcode;

    char*               mUunm;
    GUID                mUuid;
    DWORD               mAttribs;
    char*               mName;
    char*               mServiceName;

    unsigned short      mFileCount;
    unsigned short      mClassCount;
    unsigned short      mInterfaceCount;
    unsigned short      mDefinedInterfaceCount;
    unsigned short      mStructCount;
    unsigned short      mEnumCount;
    unsigned short      mAliasCount;
    unsigned short      mLibraryCount;
    unsigned short      mArrayCount;
    unsigned short      mConstCount;

    FileDirEntry**      mFileDirs;
    ClassDirEntry**     mClassDirs;
    InterfaceDirEntry** mInterfaceDirs;
    int*                mDefinedInterfaceIndexes;
    StructDirEntry**    mStructDirs;
    EnumDirEntry**      mEnumDirs;
    AliasDirEntry**     mAliasDirs;
    char**              mLibraryNames;
    ArrayDirEntry**     mArrayDirs;
    ConstDirEntry**     mConstDirs;

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
} CLSModule;

struct FileDirEntry
{
    char*               mPath;
};

typedef struct TypeDescriptor
{
    CARDataType         mType;
    unsigned short      mIndex;
    unsigned char       mPointer;
    unsigned char       mUnsigned;
    int                 mSize;          //n value in the Char8Array_<n> etc
    unsigned char       mNested;
    TypeDescriptor*     mNestedType;   // just for EzArry and EzEnum
} TypeDescriptor;

typedef struct ClassDescriptor ClassDescriptor;
typedef struct ClassInterface ClassInterface;

struct ClassDirEntry
{
    char*               mName;
    char*               mNameSpace;
    unsigned short      mFileIndex;
    ClassDescriptor*    mDesc;
};

const int MAX_CLASS_INTERFACE_NUMBER = 32;
const int MAX_CLASS_ASPECT_NUMBER = 32;
const int MAX_AGGRCLASSE_OF_ASPECT_NUMBER = 32;

struct ClassDescriptor
{
    CLSID               mClsid;
    unsigned short      mParentIndex;
    unsigned short      mCtorIndex;
    unsigned short      mInterfaceCount;
    unsigned short      mAggregateCount;
    unsigned short      mClassCount;               // only for aspect
    unsigned short      mAspectCount;
    unsigned long       mAttribs;

    ClassInterface**    mInterfaces;
    unsigned short*     mAggrIndexes;
    unsigned short*     mAspectIndexes;
    unsigned short*     mClassIndexes;          // only for aspect

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

struct ClassInterface
{
    unsigned short      mIndex;
    unsigned short      mAttribs;

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

typedef struct InterfaceDescriptor InterfaceDescriptor;
typedef struct InterfaceConstDescriptor InterfaceConstDescriptor;
typedef struct MethodDescriptor MethodDescriptor;
typedef struct ParamDescriptor ParamDescriptor;

const int MAX_INTERFACE_CONST_NUMBER = 512;
const int MAX_METHOD_NUMBER = 512;

struct InterfaceDirEntry
{
    char*                   mName;
    char*                   mNameSpace;
    unsigned short          mFileIndex;
    InterfaceDescriptor*    mDesc;
};

struct InterfaceDescriptor
{
    IID                 mIID;
    unsigned short      mConstCount;
    unsigned short      mMethodCount;
    unsigned short      mParentIndex;
    DWORD               mAttribs;

    InterfaceConstDescriptor**  mConsts;
    MethodDescriptor**  mMethods;

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

const int MAX_PARAM_NUMBER = 32;

struct MethodDescriptor
{
    char*               mName;
    char*               mSignature;
    char*               mAnnotation;
    TypeDescriptor      mType;
    unsigned short      mParamCount;
    ParamDescriptor**   mParams;
    DWORD               mAttribs;

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

struct ParamDescriptor
{
    char*               mName;
    TypeDescriptor      mType;
    DWORD               mAttribs;

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

typedef struct StructDescriptor StructDescriptor;
typedef struct StructElement StructElement;

const int MAX_STRUCT_ELEMENT_NUMBER = 128;

struct StructDirEntry
{
    char*               mName;
    char*               mNameSpace;
    unsigned short      mFileIndex;
    StructDescriptor*   mDesc;
};

struct StructDescriptor
{
    unsigned short      mElementCount;
    StructElement**     mElements;
    unsigned int        mAlignSize;
    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

struct StructElement
{
    char*               mName;
    TypeDescriptor      mType;

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

typedef struct EnumDescriptor EnumDescriptor;
typedef struct EnumElement EnumElement;

const int MAX_ENUM_ELEMENT_NUMBER = 512;

struct EnumDirEntry
{
    char*               mName;
    char*               mNameSpace;
    unsigned short      mFileIndex;
    EnumDescriptor*     mDesc;
};

struct EnumDescriptor
{
    unsigned short      mElementCount;
    EnumElement**       mElements;

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

struct EnumElement
{
    char*               mName;
    int                 mValue;
    BOOL                mIsHexFormat;

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

struct AliasDirEntry
{
    char*               mName;
    char*               mNameSpace;

    TypeDescriptor      mType;
    BOOL                mIsDummyType;

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

struct ArrayDirEntry
{
    char*               mNameSpace;
    unsigned short      mElementCount;
    TypeDescriptor      mType;
};

#define TYPE_BOOLEAN 0
#define TYPE_CHAR32 1
#define TYPE_BYTE 2
#define TYPE_INTEGER16 3
#define TYPE_INTEGER32 4
#define TYPE_INTEGER64 5
#define TYPE_FLOAT 6
#define TYPE_DOUBLE 7
#define TYPE_STRING 8

#define FORMAT_DECIMAL 1
#define FORMAT_HEX 2

struct ConstDirEntry
{
    char*               mName;
    char*               mNameSpace;
    unsigned char       mType;     // type == 0 integer; type == 1 string;
    union
    {
        struct {
            int         mValue;
            BOOL        mIsHexFormat;
        }               mInt32Value;
        struct {
            char*       mValue;
        }               mStrValue;
    }                   mV;

    union
    {
        int             mReserved;
        void*           mBindData;
    } mR;
};

struct InterfaceConstDescriptor
{
    char*               mName;
    unsigned char       mType;
    union
    {
        struct
        {
            int         mValue;
            unsigned char mFormat;
        }               mInt32Value;
        struct
        {
            long long int mValue;
            unsigned char mFormat;
        }               mInt64Value;
        BOOL            mBoolValue;
        double          mDoubleValue;
        char*           mStrValue;
    }                   mV;
};

inline BOOL IsValidUUID(
    /* [in] */ const GUID* guid)
{
    return memcmp(guid,
        "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0", sizeof(GUID));
}

#define IsNullUUID(uuid)    !IsValidUUID(uuid)

inline BOOL IsEqualUUID(
    /* [in] */ const GUID* src,
    /* [in] */ const GUID* dest)
{
    return !memcmp(src, dest, sizeof(GUID));
}

typedef enum CLSError
{
    CLS_NoError                 = 0,
    CLSError_DupEntry           = -1,
    CLSError_FullEntry          = -2,
    CLSError_OutOfMemory        = -3,
    CLSError_NameConflict       = -4,
    CLSError_NotFound           = -5,
    CLSError_FormatSize         = -6,
    CLSError_FormatMagic        = -7,
    CLSError_LoadResource       = -8,
    CLSError_OpenFile           = -9,
    CLSError_UnknownFileType    = -10,
    CLSError_StringTooLong      = -11,
    CLSError_Compress           = -12,
    CLSError_Uncompress         = -13,
    CLSError_CLSModuleVersion   = -14,
}   CLSError;

extern int CLSLastError();
extern const char *CLSLastErrorMessage();

extern CLSModule *CreateCLS();
extern void DestroyCLS(CLSModule *);

extern int IsValidCLS(CLSModule *, int, const char *);

extern int CreateFileDirEntry(const char *, CLSModule *);
extern int CreateClassDirEntry(const char *, CLSModule *, unsigned long);
extern int CreateInterfaceDirEntry(const char *, CLSModule *, unsigned long);
extern int CreateStructDirEntry(const char *, CLSModule *);
extern int CreateEnumDirEntry(const char *, CLSModule *);
extern int CreateAliasDirEntry(const char *, CLSModule *, TypeDescriptor *);
extern int CreateArrayDirEntry(CLSModule *);
extern int CreateConstDirEntry(const char *, CLSModule *);

extern int CreateClassInterface(unsigned short, ClassDescriptor *);
extern int CreateInterfaceConstDirEntry(const char *, InterfaceDescriptor *);
extern int CreateInterfaceMethod(const char *, InterfaceDescriptor *);
extern int CreateMethodParam(const char *, MethodDescriptor *);
extern int CreateStructElement(const char *, StructDescriptor *);
extern int CreateEnumElement(const char *, CLSModule *, EnumDescriptor *);

extern int SelectFileDirEntry(const char *, const CLSModule *);
extern int SelectClassDirEntry(const char *, const char *, const CLSModule *);
extern int SelectInterfaceDirEntry(const char *, const char *, const CLSModule *);
extern int SelectStructDirEntry(const char *, const CLSModule *);
extern int SelectEnumDirEntry(const char *, const char *, const CLSModule *);
extern int SelectAliasDirEntry(const char *, const CLSModule *);
extern int SelectArrayDirEntry(unsigned short, const TypeDescriptor&, const CLSModule *);
extern int SelectConstDirEntry(const char *, const CLSModule *);

extern int SelectClassInterface(unsigned short, const ClassDescriptor *);
extern int SelectInterfaceConstDirEntry(const char *, const InterfaceDescriptor *);
extern int SelectInterfaceMethod(const char *, const InterfaceDescriptor *);
extern int SelectMethodParam(const char *, const MethodDescriptor *);
extern int SelectStructElement(const char *, const StructDescriptor *);
extern int SelectEnumElement(const char *, const EnumDescriptor *);

typedef enum GlobalSymbolType
{
    GType_None              = 0,
    GType_Class             = Type_String,
    GType_Interface         = Type_interface,
    GType_Struct            = Type_struct,
    GType_Enum              = Type_enum,
    GType_Alias             = Type_alias,
    GType_Const             = Type_const,
}   GlobalObjectType;

extern int GlobalSelectSymbol(const char *, const char *, const CLSModule *, GlobalSymbolType, GlobalSymbolType *);
extern EnumElement *GlobalSelectEnumElement(const char *, const CLSModule *);
extern int SelectInterfaceMemberSymbol(const char *, InterfaceDescriptor *);

extern int GetIdentifyType(const char *, const char*, const CLSModule *, TypeDescriptor *);
int GetArrayBaseType(const CLSModule *, const TypeDescriptor *, TypeDescriptor *);
extern int GetOriginalType(const CLSModule *, const TypeDescriptor *, TypeDescriptor *);
extern BOOL IsEqualType(const CLSModule *, const TypeDescriptor *, const TypeDescriptor *);
EXTERN_C BOOL IsLocalCarQuintet(const CLSModule *, const TypeDescriptor *, DWORD);
EXTERN_C BOOL IsLocalStruct(const CLSModule *, StructDescriptor *);

extern int TypeCopy(const TypeDescriptor *, TypeDescriptor *);
EXTERN_C int TypeCopy(const CLSModule *, const TypeDescriptor *, CLSModule *, TypeDescriptor *, BOOL);
EXTERN_C int ArrayCopy(const CLSModule *, int, CLSModule *, BOOL);
EXTERN_C int AliasCopy(const CLSModule *, int, CLSModule *, BOOL);
EXTERN_C int EnumCopy(const CLSModule *, int, CLSModule *, BOOL);
EXTERN_C int StructCopy(const CLSModule *, int, CLSModule *, BOOL);
EXTERN_C int InterfaceCopy(const CLSModule *, int, CLSModule *, BOOL);
EXTERN_C int ClassCopy(const CLSModule *, int, CLSModule *, BOOL);
EXTERN_C int ConstCopy(const CLSModule *, int, CLSModule *, BOOL);

EXTERN_C int TypeXCopy(const CLSModule *, const TypeDescriptor *, CLSModule *, TypeDescriptor *, BOOL);
EXTERN_C int ArrayXCopy(const CLSModule *, int, CLSModule *, BOOL);
EXTERN_C int AliasXCopy(const CLSModule *, int, CLSModule *, BOOL);
EXTERN_C int StructXCopy(const CLSModule *, int, CLSModule *, BOOL);
EXTERN_C int InterfaceXCopy(const CLSModule *, int, CLSModule *, BOOL);
EXTERN_C int ClassXCopy(const CLSModule *, int, CLSModule *, BOOL);

extern int InterfaceMethodsAppend(const CLSModule *, const InterfaceDescriptor *, InterfaceDescriptor *);
extern int MethodAppend(const MethodDescriptor *, InterfaceDescriptor *);

const int MAX_SEED_SIZE = 255;
EXTERN_C int GuidFromSeedString(const char *, GUID *);
EXTERN_C int SeedStringFromGuid(REFGUID, char *);

extern int FlatCLS(const CLSModule *, void **);
extern int DisposeFlattedCLS(void *);
extern int RelocFlattedCLS(const void *, int, CLSModule **);

extern int CompressCLS(void *);
extern int UncompressCLS(const void *, int, CLSModule *);

extern void SetLibraryPath(const char *);
extern int CopyCLS(const CLSModule *, CLSModule *);
extern int LoadCLS(const char *, CLSModule **);
extern int LoadCLSFromDll(const char *, CLSModule **);
extern int LoadCLSFromFile(const char *, CLSModule **);
extern int SaveCLS(const char *, const CLSModule *);

extern int AbridgeCLS(const CLSModule *, void **);
extern void DisposeAbridgedCLS(void *);
extern void RelocAbridgedCLS(void *);

extern void GetNakedFileName(const char *, char *);
extern void GetNakedFileType(const char *, char *);
extern int SearchFileFromPath(const char *, const char *, char *);

#ifdef _DEBUG
extern void GuidPrint(REFGUID);
#endif // _DEBUG

//path separator
#define IS_PATH_SEPARATOR(x) ((x) == '/' || (x) == '\\')

#endif // __CLSDEF_H__
