//==========================================================================
// Copyright (c) 2000-2009,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __AJUSTADD_H__
#define __AJUSTADD_H__

_ELASTOS_NAMESPACE_USING

inline ClassDirEntry* getClassDirAddr(
    /* [in] */ Int32 base,
    /* [in] */ ClassDirEntry** dir,
    /* [in] */ Int32 index)
{
    dir = (ClassDirEntry**)((Int32)dir + base);
    return (ClassDirEntry*)((Int32)dir[index] + base);
}

inline InterfaceDirEntry* getInterfaceDirAddr(
    /* [in] */ Int32 base,
    /* [in] */ InterfaceDirEntry** dir,
    /* [in] */ Int32 index)
{
    dir = (InterfaceDirEntry**)((Int32)dir + base);
    return (InterfaceDirEntry*)((Int32)dir[index] + base);
}

inline StructDirEntry* getStructDirAddr(
    /* [in] */ Int32 base,
    /* [in] */ StructDirEntry** dir,
    /* [in] */ Int32 index)
{
    dir = (StructDirEntry**)((Int32)dir + base);
    return (StructDirEntry*)((Int32)dir[index] + base);
}

inline EnumDirEntry* getEnumDirAddr(
    /* [in] */ Int32 base,
    /* [in] */ EnumDirEntry** dir,
    /* [in] */ Int32 index)
{
    dir = (EnumDirEntry**)((Int32)dir + base);
    return (EnumDirEntry*)((Int32)dir[index] + base);
}

inline AliasDirEntry* getAliasDirAddr(
    /* [in] */ Int32 base,
    /* [in] */ AliasDirEntry** dir,
    /* [in] */ Int32 index)
{
    dir = (AliasDirEntry**)((Int32)dir + base);
    return (AliasDirEntry*)((Int32)dir[index] + base);
}

inline ArrayDirEntry* getArrayDirAddr(
    /* [in] */ Int32 base,
    /* [in] */ ArrayDirEntry** dir,
    /* [in] */ Int32 index)
{
    dir = (ArrayDirEntry**)((Int32)dir + base);
    return (ArrayDirEntry*)((Int32)dir[index] + base);
}

inline ConstDirEntry* getConstDirAddr(
    /* [in] */ Int32 base,
    /* [in] */ ConstDirEntry** dir,
    /* [in] */ Int32 index)
{
    dir = (ConstDirEntry**)((Int32)dir + base);
    return (ConstDirEntry*)((Int32)dir[index] + base);
}

inline MethodDescriptor* getMethodDescAddr(
    /* [in] */ Int32 base,
    /* [in] */ MethodDescriptor** desc,
    /* [in] */ Int32 index)
{
    desc = (MethodDescriptor**)((Int32)desc + base);
    return (MethodDescriptor*)((Int32)desc[index] + base);
}

inline ParamDescriptor* getParamDescAddr(
    /* [in] */ Int32 base,
    /* [in] */ ParamDescriptor** desc,
    /* [in] */ Int32 index)
{
    desc = (ParamDescriptor**)((Int32)desc + base);
    return (ParamDescriptor*)((Int32)desc[index] + base);
}

inline StructElement* getStructElementAddr(
    /* [in] */ Int32 base,
    /* [in] */ StructElement** element,
    /* [in] */ Int32 index)
{
    element = (StructElement**)((Int32)element + base);
    return (StructElement*)((Int32)element[index] + base);
}

inline EnumElement* getEnumElementAddr(
    /* [in] */ Int32 base,
    /* [in] */ EnumElement** element,
    /* [in] */ Int32 index)
{
    element = (EnumElement**)((Int32)element + base);
    return (EnumElement*)((Int32)element[index] + base);
}

inline ClassInterface* getCIFAddr(
    /* [in] */ Int32 base,
    /* [in] */ ClassInterface** clsInterface,
    /* [in] */ Int32 index)
{
    clsInterface = (ClassInterface**)((Int32)clsInterface + base);
    return (ClassInterface*)((Int32)clsInterface[index] + base);
}

inline char* getLibNameAddr(
    /* [in] */ Int32 base,
    /* [in] */ char** libName,
    /* [in] */ Int32 index)
{
    libName = (char**)((Int32)libName + base);
    return (char*)((Int32)libName[index] + base);
}

inline char* adjustNameAddr(
    /* [in] */ Int32 base,
    /* [in] */ char* name)
{
    return name ? name + base : name;
}

inline USHORT* adjustIndexsAddr(
    /* [in] */ Int32 base,
    /* [in] */ USHORT* indexs)
{
    return indexs ? (USHORT*)((PByte)indexs + base) : indexs;
}

inline ClassDescriptor* adjustClassDescAddr(
    /* [in] */ Int32 base,
    /* [in] */ ClassDescriptor* desc)
{
    return desc ? (ClassDescriptor*)((PByte)desc + base) : desc;
}

inline InterfaceDescriptor* adjustInterfaceDescAddr(
    /* [in] */ Int32 base,
    /* [in] */ InterfaceDescriptor* desc)
{
    return desc ? (InterfaceDescriptor*)((PByte)desc + base) : desc;
}

inline MethodDescriptor* adjustMethodDescAddr(
    /* [in] */ Int32 base,
    /* [in] */ MethodDescriptor* desc)
{
    return desc ? (MethodDescriptor*)((PByte)desc + base) : desc;
}

inline ParamDescriptor* adjustParamDescAddr(
    /* [in] */ Int32 base,
    /* [in] */ ParamDescriptor* desc)
{
    return desc ? (ParamDescriptor*)((PByte)desc + base) : desc;
}

inline StructDescriptor* adjustStructDescAddr(
    /* [in] */ Int32 base,
    /* [in] */ StructDescriptor* desc)
{
    return desc ? (StructDescriptor*)((PByte)desc + base) : desc;
}

inline EnumDescriptor* adjustEnumDescAddr(
    /* [in] */ Int32 base,
    /* [in] */ EnumDescriptor* desc)
{
    return desc ? (EnumDescriptor*)((PByte)desc + base) : desc;
}

inline TypeDescriptor* adjustTypeAddr(
    /* [in] */ Int32 base,
    /* [in] */ TypeDescriptor* desc)
{
    return desc ? (TypeDescriptor*)((PByte)desc + base) : desc;
}

inline TypeDescriptor* adjustNestedTypeAddr(
    /* [in] */ Int32 base,
    /* [in] */ TypeDescriptor* nestedType)
{
    return nestedType ?
            (TypeDescriptor*)((PByte)nestedType + base) : nestedType;
}

#endif // __AJUSTADD_H__
