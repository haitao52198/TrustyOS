#ifndef __CAR_ELASTOSCORE_H__
#define __CAR_ELASTOSCORE_H__

#ifndef _NO_INCLIST
#include <elastos.h>
using namespace Elastos;
#endif // !_NO_INCLIST


interface ICallbackRendezvous;
EXTERN const _ELASTOS InterfaceID EIID_ICallbackRendezvous;
interface ICallbackSink;
EXTERN const _ELASTOS InterfaceID EIID_ICallbackSink;
interface IRegime;
EXTERN const _ELASTOS InterfaceID EIID_IRegime;
interface IDataTypeInfo;
EXTERN const _ELASTOS InterfaceID EIID_IDataTypeInfo;
interface ILocalPtrInfo;
EXTERN const _ELASTOS InterfaceID EIID_ILocalPtrInfo;
interface IEnumInfo;
EXTERN const _ELASTOS InterfaceID EIID_IEnumInfo;
interface IEnumItemInfo;
EXTERN const _ELASTOS InterfaceID EIID_IEnumItemInfo;
interface ICarArrayInfo;
EXTERN const _ELASTOS InterfaceID EIID_ICarArrayInfo;
interface ICppVectorInfo;
EXTERN const _ELASTOS InterfaceID EIID_ICppVectorInfo;
interface IStructInfo;
EXTERN const _ELASTOS InterfaceID EIID_IStructInfo;
interface IFieldInfo;
EXTERN const _ELASTOS InterfaceID EIID_IFieldInfo;
interface IInterfaceInfo;
EXTERN const _ELASTOS InterfaceID EIID_IInterfaceInfo;
interface IFunctionInfo;
EXTERN const _ELASTOS InterfaceID EIID_IFunctionInfo;
interface IMethodInfo;
EXTERN const _ELASTOS InterfaceID EIID_IMethodInfo;
interface IParamInfo;
EXTERN const _ELASTOS InterfaceID EIID_IParamInfo;
interface ITypeAliasInfo;
EXTERN const _ELASTOS InterfaceID EIID_ITypeAliasInfo;
interface IArgumentList;
EXTERN const _ELASTOS InterfaceID EIID_IArgumentList;
interface IVariable;
EXTERN const _ELASTOS InterfaceID EIID_IVariable;
interface IVariableOfCarArray;
EXTERN const _ELASTOS InterfaceID EIID_IVariableOfCarArray;
interface IVariableOfStruct;
EXTERN const _ELASTOS InterfaceID EIID_IVariableOfStruct;
interface ICarArraySetter;
EXTERN const _ELASTOS InterfaceID EIID_ICarArraySetter;
interface ICarArrayGetter;
EXTERN const _ELASTOS InterfaceID EIID_ICarArrayGetter;
interface IStructSetter;
EXTERN const _ELASTOS InterfaceID EIID_IStructSetter;
interface IStructGetter;
EXTERN const _ELASTOS InterfaceID EIID_IStructGetter;
interface ICppVectorSetter;
EXTERN const _ELASTOS InterfaceID EIID_ICppVectorSetter;
interface ICppVectorGetter;
EXTERN const _ELASTOS InterfaceID EIID_ICppVectorGetter;
interface IModuleInfo;
EXTERN const _ELASTOS InterfaceID EIID_IModuleInfo;
interface IConstantInfo;
EXTERN const _ELASTOS InterfaceID EIID_IConstantInfo;
interface IClassInfo;
EXTERN const _ELASTOS InterfaceID EIID_IClassInfo;
interface IConstructorInfo;
EXTERN const _ELASTOS InterfaceID EIID_IConstructorInfo;
interface ICallbackMethodInfo;
EXTERN const _ELASTOS InterfaceID EIID_ICallbackMethodInfo;
interface IDelegateProxy;
EXTERN const _ELASTOS InterfaceID EIID_IDelegateProxy;
interface ICallbackInvocation;
EXTERN const _ELASTOS InterfaceID EIID_ICallbackInvocation;
interface ICallbackArgumentList;
EXTERN const _ELASTOS InterfaceID EIID_ICallbackArgumentList;
interface IParcel;
EXTERN const _ELASTOS InterfaceID EIID_IParcel;
interface IParcelable;
EXTERN const _ELASTOS InterfaceID EIID_IParcelable;
interface ICustomMarshal;
EXTERN const _ELASTOS InterfaceID EIID_ICustomMarshal;





#ifndef __ENUM_WaitResult__
#define __ENUM_WaitResult__
enum {
    WaitResult_OK = 0x00000000,
    WaitResult_Interrupted = 0x00000001,
    WaitResult_TimedOut = 0x00000002,
};
typedef _ELASTOS Int32 WaitResult;

#endif //__ENUM_WaitResult__


#ifndef __ENUM_CallbackPriority__
#define __ENUM_CallbackPriority__
enum {
    CallbackPriority_Highest = 0x00000000,
    CallbackPriority_AboveNormal = 0x00000007,
    CallbackPriority_Normal = 0x00000010,
    CallbackPriority_BelowNormal = 0x00000017,
    CallbackPriority_Lowest = 0x0000001f,
};
typedef _ELASTOS Int32 CallbackPriority;

#endif //__ENUM_CallbackPriority__


#ifndef __ENUM_CallbackContextStatus__
#define __ENUM_CallbackContextStatus__
enum {
    CallbackContextStatus_Created = 0x00000001,
    CallbackContextStatus_Idling = 2,
    CallbackContextStatus_Working = 3,
    CallbackContextStatus_Finishing = 4,
};
typedef _ELASTOS Int32 CallbackContextStatus;

#endif //__ENUM_CallbackContextStatus__


#ifndef __ENUM_CallbackContextFinish__
#define __ENUM_CallbackContextFinish__
enum {
    CallbackContextFinish_Nice = 0x00000001,
    CallbackContextFinish_ASAP = 2,
};
typedef _ELASTOS Int32 CallbackContextFinish;

#endif //__ENUM_CallbackContextFinish__


#ifndef __ENUM_CarDataType__
#define __ENUM_CarDataType__
enum {
    CarDataType_Int16 = 1,
    CarDataType_Int32 = 2,
    CarDataType_Int64 = 3,
    CarDataType_Byte = 4,
    CarDataType_Float = 5,
    CarDataType_Double = 6,
    CarDataType_Char32 = 7,
    CarDataType_String = 8,
    CarDataType_Boolean = 9,
    CarDataType_EMuid = 10,
    CarDataType_EGuid = 11,
    CarDataType_ECode = 12,
    CarDataType_LocalPtr = 13,
    CarDataType_LocalType = 14,
    CarDataType_Enum = 15,
    CarDataType_ArrayOf = 16,
    CarDataType_CppVector = 17,
    CarDataType_Struct = 18,
    CarDataType_Interface = 19,
};
typedef _ELASTOS Int32 CarDataType;

#endif //__ENUM_CarDataType__


#ifndef __ENUM_ParamIOAttribute__
#define __ENUM_ParamIOAttribute__
enum {
    ParamIOAttribute_In = 0,
    ParamIOAttribute_CalleeAllocOut = 1,
    ParamIOAttribute_CallerAllocOut = 2,
};
typedef _ELASTOS Int32 ParamIOAttribute;

#endif //__ENUM_ParamIOAttribute__


#ifndef __ENUM_ThreadingModel__
#define __ENUM_ThreadingModel__
enum {
    ThreadingModel_Sequenced = 1,
    ThreadingModel_Synchronized = 2,
    ThreadingModel_ThreadSafe = 3,
    ThreadingModel_Naked = 4,
};
typedef _ELASTOS Int32 ThreadingModel;

#endif //__ENUM_ThreadingModel__


#ifndef __ENUM_MarshalType__
#define __ENUM_MarshalType__
enum {
    MarshalType_IPC = 0,
    MarshalType_RPC = 1,
};
typedef _ELASTOS Int32 MarshalType;

#endif //__ENUM_MarshalType__


#ifndef __ENUM_UnmarshalFlag__
#define __ENUM_UnmarshalFlag__
enum {
    UnmarshalFlag_Noncoexisting = 0x00000010,
    UnmarshalFlag_Coexisting = 0x00000020,
};
typedef _ELASTOS Int32 UnmarshalFlag;

#endif //__ENUM_UnmarshalFlag__

typedef _ELASTOS PVoid LocalPtr;

#ifdef __ELASTOSCORE_USER_TYPE_H__
#include "ElastosCore_user_type.h"
#endif

#endif // __CAR_ELASTOSCORE_H__
