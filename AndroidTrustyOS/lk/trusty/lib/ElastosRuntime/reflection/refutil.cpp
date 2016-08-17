//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "refutil.h"
#include "CClsModule.h"

const DateTypeDesc g_cDataTypeList[] = {
   {"UnknowType",    0,                },
   {"Int16",         sizeof(Int16),    },
   {"Int32",         sizeof(Int32),    },
   {"Int64",         sizeof(Int64),    },
   {"Byte",          sizeof(Byte),     },
   {"Float",         sizeof(Float),    },
   {"Double",        sizeof(Double),   },
   {"Char32",        sizeof(Char32),   },
   {"String",        sizeof(String),   },
   {"Boolean",       sizeof(Boolean),  },
   {"EMuid",         sizeof(EMuid),    },
   {"EGuid",         sizeof(EGuid),    },
   {"ECode",         sizeof(ECode)     },
   {"LocalPtr",      sizeof(PVoid),    },
   {"LocalType",     0,                },
   {"Enum",          sizeof(Int32),    },
   {"ArrayOf",       0,                },
   {"CppVector",     sizeof(PVoid),    },
   {"Struct",        0,                },
   {"Interface",     sizeof(IInterface *),},
};

CarDataType GetCarDataType(
    /* [in] */ CARDataType type)
{
    CarDataType dataType;
    switch (type) {
        case Type_Int16:
            dataType = CarDataType_Int16;
            break;
        case Type_Int32:
            dataType = CarDataType_Int32;
            break;
        case Type_Int64:
            dataType = CarDataType_Int64;
            break;
        case Type_Byte:
            dataType = CarDataType_Byte;
            break;
        case Type_Boolean:
            dataType = CarDataType_Boolean;
            break;
        case Type_Float:
            dataType = CarDataType_Float;
            break;
        case Type_Double:
            dataType = CarDataType_Double;
            break;
        case Type_Char32:
            dataType = CarDataType_Char32;
            break;
        case Type_String:
            dataType = CarDataType_String;
            break;
        case Type_EMuid:
            dataType = CarDataType_EMuid;
            break;
        case Type_EGuid:
            dataType = CarDataType_EGuid;
            break;
        case Type_ECode:
            dataType = CarDataType_ECode;
            break;
//        case Type_const:
//            dataType = CarDataType_;
//            break;
        case Type_enum:
            dataType = CarDataType_Enum;
            break;
        case Type_Array:
            dataType = CarDataType_CppVector;
            break;
        case Type_struct:
            dataType = CarDataType_Struct;
            break;
        case Type_interface:
            dataType = CarDataType_Interface;
            break;
        case Type_ArrayOf:
            dataType = CarDataType_ArrayOf;
            break;
        case Type_Int8:
        case Type_UInt16:
        case Type_UInt32:
        case Type_UInt64:
        case Type_PVoid:
        case Type_EventHandler:
        case Type_alias:
        default:
            dataType = CarDataType_LocalType;
            break;
    }

    return dataType;
}

Boolean IsSysAlaisType(
    /* [in] */ const CClsModule* clsModule,
    /* [in] */ UInt32 index)
{
    Int32 base = clsModule->mBase;
    CLSModule* module = clsModule->mClsMod;
    AliasDirEntry* aliasDir = getAliasDirAddr(base, module->mAliasDirs, index);
    char* nameSpace = adjustNameAddr(base, aliasDir->mNameSpace);
    return (nameSpace) && !strcmp(nameSpace, "systypes");
}

void _GetOriginalType(
    /* [in] */ const CClsModule* clsModule,
    /* [in] */ const TypeDescriptor* src,
    /* [in] */ TypeDescriptor* dest)
{
    dest->mPointer = src->mPointer;
    dest->mUnsigned = src->mUnsigned;

    AliasDirEntry* aliasDir = NULL;
    while (src->mType == Type_alias) {
        aliasDir = getAliasDirAddr(clsModule->mBase,
                clsModule->mClsMod->mAliasDirs, src->mIndex);
        src = &aliasDir->mType;
        dest->mPointer += src->mPointer;
        dest->mUnsigned |= src->mUnsigned;
    }

    dest->mType = src->mType;
    dest->mIndex = src->mIndex;
    dest->mNestedType = src->mNestedType;
}

UInt32 GetDataTypeSize(
    /* [in] */ const CClsModule* clsModule,
    /* [in] */ TypeDescriptor* typeDesc)
{
    UInt32 size = 0;

    CLSModule* module = clsModule->mClsMod;
    if (typeDesc->mType == Type_alias) {
        TypeDescriptor orgTypeDesc;
        _GetOriginalType(clsModule, typeDesc, &orgTypeDesc);
        typeDesc = &orgTypeDesc;
    }

    if (typeDesc->mPointer) {
        return sizeof(void *);
    }

    ArrayDirEntry* arrayDir = NULL;
    StructDirEntry* structDir = NULL;
    Int32 base = clsModule->mBase;
    switch (typeDesc->mType) {
        case Type_Char16:
            size = sizeof(Char16);
            break;
        case Type_Char32:
            size = sizeof(Char32);
            break;
        case Type_Int8:
            size = sizeof(Int8);
            break;
        case Type_Int16:
            size = sizeof(Int16);
            break;
        case Type_Int32:
            size = sizeof(Int32);
            break;
        case Type_Int64:
            size = sizeof(Int64);
            break;
        case Type_UInt16:
            size = sizeof(UInt16);
            break;
        case Type_UInt32:
            size = sizeof(UInt32);
            break;
        case Type_UInt64:
            size = sizeof(UInt64);
            break;
        case Type_Byte:
            size = sizeof(Byte);
            break;
        case Type_Boolean:
            size = sizeof(Boolean);
            break;
        case Type_EMuid:
            size = sizeof(EMuid);
            break;
        case Type_Float:
            size = sizeof(float);
            break;
        case Type_Double:
            size = sizeof(double);
            break;
        case Type_PVoid:
            size = 4;
            break;
        case Type_ECode:
            size = sizeof(ECode);
            break;
        case Type_EGuid:
            size = sizeof(ClassID);
            break;
        case Type_EventHandler:
            size = sizeof(EventHandler);
            break;
        case Type_String:
            // [in] String (in car) --> /* [in] */ const String& (in c++), so should be sizeof(String*)
            size = sizeof(String*);
            break;
        case Type_interface:
            size = sizeof(PInterface);
            break;
        case Type_struct:
            structDir = getStructDirAddr(base,
                    module->mStructDirs, typeDesc->mIndex);
            size = adjustStructDescAddr(base, structDir->mDesc)->mAlignSize;
            break;
        case Type_Array:
            arrayDir = getArrayDirAddr(base,
                    module->mArrayDirs, typeDesc->mIndex);
            size = GetDataTypeSize(clsModule, &arrayDir->mType) * arrayDir->mElementCount;
            break;
        case Type_enum:
            size = sizeof(int);
            break;
        case Type_ArrayOf:
            size = GetDataTypeSize(clsModule,
                    adjustNestedTypeAddr(base, typeDesc->mNestedType));
            break;
//        case Type_EzEnum:
//            size = sizeof(EzEnum);
//            break;
        case Type_alias:
        case Type_const:
        default:
            size = 0;
    }

    return size;
}

//CARDataType GetBasicType(TypeDescriptor *typeDesc)
//{
//    CARDataType type;
//    switch (typeDesc->type) {
//        case Type_Int16Array_:
//            type = Type_Int16;
//            break;
//        case Type_Int32Array_:
//            type = Type_Int32;
//            break;
//        case Type_Int64Array_:
//            type = Type_Int64;
//            break;
//        case Type_ByteArray_:
//            type = Type_Byte;
//            break;
//        case Type_FloatArray_:
//            type = Type_Float;
//            break;
//        case Type_DoubleArray_:
//            type = Type_Double;
//            break;
//        case Type_Char16Array_:
//            type = Type_Char16;
//            break;
//        case Type_StringArray_:
//            type = Type_String;
//            break;
//        case Type_ArrayOf_:
//            type = Type_struct;
//            break;
//        case Type_ObjectArray_:
//            type = Type_interface;
//            break;
//        case Type_BooleanArray_:
//            type = Type_Boolean;
//            break;
//        case Type_EMuidArray_:
//            type = Type_EMuid;
//            break;
//        case Type_EGuidArray_:
//            type = Type_EGuid;
//            break;
//        case Type_ECodeArray_:
//            type = Type_ECode;
//            break;
//        case Type_ArrayOf:
//            type = GetBasicType(typeDesc->mNestedType);
//            break;
//        default:
//            type = typeDesc->type;
//            break;
//   }
//
//   return type;
//}

CarQuintetFlag DataTypeToFlag(
    /* [in] */ CarDataType type)
{
    CarQuintetFlag flag;
    switch (type) {
        case CarDataType_Int16:
            flag = CarQuintetFlag_Type_Int16;
            break;
        case CarDataType_Int32:
            flag = CarQuintetFlag_Type_Int32;
            break;
        case CarDataType_Int64:
            flag = CarQuintetFlag_Type_Int64;
            break;
        case CarDataType_Byte:
            flag = CarQuintetFlag_Type_Byte;
            break;
        case CarDataType_Boolean:
            flag = CarQuintetFlag_Type_Boolean;
            break;
        case CarDataType_Float:
            flag = CarQuintetFlag_Type_Float;
            break;
        case CarDataType_Double:
            flag = CarQuintetFlag_Type_Double;
            break;
        case CarDataType_Char32:
            flag = CarQuintetFlag_Type_Char32;
            break;
        case CarDataType_String:
            flag = CarQuintetFlag_Type_String;
            break;
        case CarDataType_EMuid:
            flag = CarQuintetFlag_Type_EMuid;
            break;
        case CarDataType_EGuid:
            flag = CarQuintetFlag_Type_EGuid;
            break;
        case CarDataType_ECode:
            flag = CarQuintetFlag_Type_ECode;
            break;
        case CarDataType_Enum:
            flag = CarQuintetFlag_Type_Enum;
            break;
        case CarDataType_Struct:
            flag = CarQuintetFlag_Type_Struct;
            break;
        case CarDataType_Interface:
            flag = CarQuintetFlag_Type_IObject;
            break;
        default:
            flag = CarQuintetFlag_Type_Unknown;
            break;
    }

    return flag;
}
