#ifndef ___ElastosCore_h__
#define ___ElastosCore_h__

#include <ElastosCore.h>

EXTERN_C ELAPI _Impl_AcquireCallbackHandler(PInterface pServerObj, _ELASTOS REIID iid, PInterface *ppHandler);
EXTERN_C ELAPI _Impl_CheckClsId(PInterface pServerObj, const _ELASTOS ClassID* pClassid, PInterface *ppServerObj);

CAR_INTERFACE("73341821-9C52-280D-629C-40D123870C1C")
ICallbackRendezvous : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICallbackRendezvous*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICallbackRendezvous*)pObj->Probe(EIID_ICallbackRendezvous);
    }

    static CARAPI_(ICallbackRendezvous*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICallbackRendezvous*)pObj->Probe(EIID_ICallbackRendezvous);
    }

    virtual CARAPI Wait(
        /* [in] */ _ELASTOS Int32 timeout,
        /* [out] */ WaitResult * result) = 0;

};

CAR_INTERFACE("7BD88031-1C52-3FD0-8CC4-E91B4781091A")
ICallbackSink : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICallbackSink*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICallbackSink*)pObj->Probe(EIID_ICallbackSink);
    }

    static CARAPI_(ICallbackSink*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICallbackSink*)pObj->Probe(EIID_ICallbackSink);
    }

    virtual CARAPI AddCallback(
        /* [in] */ _ELASTOS Int32 event,
        /* [in] */ _ELASTOS EventHandler handler) = 0;

    virtual CARAPI RemoveCallback(
        /* [in] */ _ELASTOS Int32 event,
        /* [in] */ _ELASTOS EventHandler handler) = 0;

    virtual CARAPI AcquireCallbackRendezvous(
        /* [in] */ _ELASTOS Int32 event,
        /* [out] */ ICallbackRendezvous ** rendezvous) = 0;

    virtual CARAPI RemoveAllCallbacks() = 0;

    virtual CARAPI CancelPendingCallback(
        /* [in] */ _ELASTOS Int32 event) = 0;

    virtual CARAPI CancelAllPendingCallbacks() = 0;

};

CAR_INTERFACE("F3E62A3A-DE52-461F-8519-9D68C9280000")
IRegime : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IRegime*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IRegime*)pObj->Probe(EIID_IRegime);
    }

    static CARAPI_(IRegime*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IRegime*)pObj->Probe(EIID_IRegime);
    }

    virtual CARAPI ObjectEnter(
        /* [in] */ PInterface object) = 0;

    virtual CARAPI ObjectLeave(
        /* [in] */ PInterface object) = 0;

    virtual CARAPI CreateObject(
        /* [in] */ const _ELASTOS ClassID & clsid,
        /* [in] */ const _ELASTOS InterfaceID & iid,
        /* [out] */ IInterface ** object) = 0;

};

CAR_INTERFACE("AF89272C-A612-FBE5-7E90-0B4A2E34394C")
IDataTypeInfo : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IDataTypeInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IDataTypeInfo*)pObj->Probe(EIID_IDataTypeInfo);
    }

    static CARAPI_(IDataTypeInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IDataTypeInfo*)pObj->Probe(EIID_IDataTypeInfo);
    }

    virtual CARAPI GetName(
        /* [out] */ _ELASTOS String * name) = 0;

    virtual CARAPI GetSize(
        /* [out] */ _ELASTOS MemorySize * size) = 0;

    virtual CARAPI GetDataType(
        /* [out] */ CarDataType * dataType) = 0;

};

CAR_INTERFACE("DA265139-C792-370C-6AEC-FB41AE03891A")
ILocalPtrInfo : public IDataTypeInfo
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ILocalPtrInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ILocalPtrInfo*)pObj->Probe(EIID_ILocalPtrInfo);
    }

    static CARAPI_(ILocalPtrInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ILocalPtrInfo*)pObj->Probe(EIID_ILocalPtrInfo);
    }

    virtual CARAPI GetTargetTypeInfo(
        /* [out] */ IDataTypeInfo ** dateTypeInfo) = 0;

    virtual CARAPI GetPtrLevel(
        /* [out] */ _ELASTOS Int32 * level) = 0;

};

CAR_INTERFACE("C248702C-B192-3F7D-C805-B53183840914")
IEnumInfo : public IDataTypeInfo
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IEnumInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IEnumInfo*)pObj->Probe(EIID_IEnumInfo);
    }

    static CARAPI_(IEnumInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IEnumInfo*)pObj->Probe(EIID_IEnumInfo);
    }

    virtual CARAPI GetNamespace(
        /* [out] */ _ELASTOS String * ns) = 0;

    virtual CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo ** moduleInfo) = 0;

    virtual CARAPI GetItemCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllItemInfos(
        /* [out] */ _ELASTOS ArrayOf<IEnumItemInfo *> * itemInfos) = 0;

    virtual CARAPI GetItemInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IEnumItemInfo ** enumItemInfo) = 0;

};

CAR_INTERFACE("93362835-B192-FB84-7E90-0B4ACED83E72")
IEnumItemInfo : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IEnumItemInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IEnumItemInfo*)pObj->Probe(EIID_IEnumItemInfo);
    }

    static CARAPI_(IEnumItemInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IEnumItemInfo*)pObj->Probe(EIID_IEnumItemInfo);
    }

    virtual CARAPI GetName(
        /* [out] */ _ELASTOS String * name) = 0;

    virtual CARAPI GetEnumInfo(
        /* [out] */ IEnumInfo ** enumInfo) = 0;

    virtual CARAPI GetValue(
        /* [out] */ _ELASTOS Int32 * value) = 0;

};

CAR_INTERFACE("0EC4662D-0312-3512-B0EC-FB41CEC10B14")
ICarArrayInfo : public IDataTypeInfo
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICarArrayInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICarArrayInfo*)pObj->Probe(EIID_ICarArrayInfo);
    }

    static CARAPI_(ICarArrayInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICarArrayInfo*)pObj->Probe(EIID_ICarArrayInfo);
    }

    virtual CARAPI GetElementTypeInfo(
        /* [out] */ IDataTypeInfo ** elementTypeInfo) = 0;

    virtual CARAPI CreateVariable(
        /* [in] */ _ELASTOS Int32 capacity,
        /* [out] */ IVariableOfCarArray ** variableBox) = 0;

    virtual CARAPI CreateVariableBox(
        /* [in] */ _ELASTOS PCarQuintet variableDescriptor,
        /* [out] */ IVariableOfCarArray ** variableBox) = 0;

};

CAR_INTERFACE("688C514F-0312-1FB4-885D-D5F2A38A8394")
ICppVectorInfo : public IDataTypeInfo
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICppVectorInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICppVectorInfo*)pObj->Probe(EIID_ICppVectorInfo);
    }

    static CARAPI_(ICppVectorInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICppVectorInfo*)pObj->Probe(EIID_ICppVectorInfo);
    }

    virtual CARAPI GetElementTypeInfo(
        /* [out] */ IDataTypeInfo ** elementTypeInfo) = 0;

    virtual CARAPI GetLength(
        /* [out] */ _ELASTOS Int32 * length) = 0;

};

CAR_INTERFACE("C3DE8833-EC52-3F7D-C8E5-DC47EE010B14")
IStructInfo : public IDataTypeInfo
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IStructInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IStructInfo*)pObj->Probe(EIID_IStructInfo);
    }

    static CARAPI_(IStructInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IStructInfo*)pObj->Probe(EIID_IStructInfo);
    }

    virtual CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo ** moduleInfo) = 0;

    virtual CARAPI GetFieldCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllFieldInfos(
        /* [out] */ _ELASTOS ArrayOf<IFieldInfo *> * fieldInfos) = 0;

    virtual CARAPI GetFieldInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IFieldInfo ** fieldInfo) = 0;

    virtual CARAPI CreateVariable(
        /* [out] */ IVariableOfStruct ** variableBox) = 0;

    virtual CARAPI CreateVariableBox(
        /* [in] */ _ELASTOS PVoid variableDescriptor,
        /* [out] */ IVariableOfStruct ** variableBox) = 0;

};

CAR_INTERFACE("46D71D54-03D2-2783-B5FC-A8D265671539")
IFieldInfo : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IFieldInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IFieldInfo*)pObj->Probe(EIID_IFieldInfo);
    }

    static CARAPI_(IFieldInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IFieldInfo*)pObj->Probe(EIID_IFieldInfo);
    }

    virtual CARAPI GetName(
        /* [out] */ _ELASTOS String * name) = 0;

    virtual CARAPI GetTypeInfo(
        /* [out] */ IDataTypeInfo ** typeInfo) = 0;

};

CAR_INTERFACE("A1B59930-C092-3F7D-C805-B53183840914")
IInterfaceInfo : public IDataTypeInfo
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IInterfaceInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IInterfaceInfo*)pObj->Probe(EIID_IInterfaceInfo);
    }

    static CARAPI_(IInterfaceInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IInterfaceInfo*)pObj->Probe(EIID_IInterfaceInfo);
    }

    virtual CARAPI GetNamespace(
        /* [out] */ _ELASTOS String * ns) = 0;

    virtual CARAPI GetId(
        /* [out] */ _ELASTOS InterfaceID * iid) = 0;

    virtual CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo ** moduleInfo) = 0;

    virtual CARAPI IsLocal(
        /* [out] */ _ELASTOS Boolean * isLocal) = 0;

    virtual CARAPI HasBase(
        /* [out] */ _ELASTOS Boolean * hasBase) = 0;

    virtual CARAPI GetBaseInfo(
        /* [out] */ IInterfaceInfo ** baseInfo) = 0;

    virtual CARAPI GetMethodCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllMethodInfos(
        /* [out] */ _ELASTOS ArrayOf<IMethodInfo *> * methodInfos) = 0;

    virtual CARAPI GetMethodInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ const _ELASTOS String& signature,
        /* [out] */ IMethodInfo ** methodInfo) = 0;

};

CAR_INTERFACE("70175724-B852-3F7D-C805-2587B512B925")
IFunctionInfo : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IFunctionInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IFunctionInfo*)pObj->Probe(EIID_IFunctionInfo);
    }

    static CARAPI_(IFunctionInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IFunctionInfo*)pObj->Probe(EIID_IFunctionInfo);
    }

    virtual CARAPI GetName(
        /* [out] */ _ELASTOS String * name) = 0;

    virtual CARAPI GetParamCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllParamInfos(
        /* [out] */ _ELASTOS ArrayOf<IParamInfo *> * paramInfos) = 0;

    virtual CARAPI GetParamInfoByIndex(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ IParamInfo ** paramInfo) = 0;

    virtual CARAPI GetParamInfoByName(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IParamInfo ** paramInfo) = 0;

};

CAR_INTERFACE("A78D8834-CD52-3F7D-C829-881143C60D12")
IMethodInfo : public IFunctionInfo
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IMethodInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IMethodInfo*)pObj->Probe(EIID_IMethodInfo);
    }

    static CARAPI_(IMethodInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IMethodInfo*)pObj->Probe(EIID_IMethodInfo);
    }

    virtual CARAPI GetAnnotation(
        /* [out] */ _ELASTOS String * annotation) = 0;

    virtual CARAPI CreateArgumentList(
        /* [out] */ IArgumentList ** argumentList) = 0;

    virtual CARAPI Invoke(
        /* [in] */ PInterface target,
        /* [in] */ IArgumentList * argumentList) = 0;

};

CAR_INTERFACE("32C78335-D612-3F7D-C8D5-DC472E28B97C")
IParamInfo : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IParamInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IParamInfo*)pObj->Probe(EIID_IParamInfo);
    }

    static CARAPI_(IParamInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IParamInfo*)pObj->Probe(EIID_IParamInfo);
    }

    virtual CARAPI GetMethodInfo(
        /* [out] */ IMethodInfo ** methodInfo) = 0;

    virtual CARAPI GetName(
        /* [out] */ _ELASTOS String * name) = 0;

    virtual CARAPI GetIndex(
        /* [out] */ _ELASTOS Int32 * index) = 0;

    virtual CARAPI GetIOAttribute(
        /* [out] */ ParamIOAttribute * ioAttrib) = 0;

    virtual CARAPI IsReturnValue(
        /* [out] */ _ELASTOS Boolean * returnValue) = 0;

    virtual CARAPI GetTypeInfo(
        /* [out] */ IDataTypeInfo ** typeInfo) = 0;

    virtual CARAPI GetAdvisedCapacity(
        /* [out] */ _ELASTOS Int32 * advisedCapacity) = 0;

    virtual CARAPI IsUsingTypeAlias(
        /* [out] */ _ELASTOS Boolean * usingTypeAlias) = 0;

    virtual CARAPI GetUsedTypeAliasInfo(
        /* [out] */ ITypeAliasInfo ** usedTypeAliasInfo) = 0;

};

CAR_INTERFACE("49F64054-1C92-2C05-1769-F951A5CBCE2A")
ITypeAliasInfo : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ITypeAliasInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ITypeAliasInfo*)pObj->Probe(EIID_ITypeAliasInfo);
    }

    static CARAPI_(ITypeAliasInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ITypeAliasInfo*)pObj->Probe(EIID_ITypeAliasInfo);
    }

    virtual CARAPI GetName(
        /* [out] */ _ELASTOS String * name) = 0;

    virtual CARAPI GetTypeInfo(
        /* [out] */ IDataTypeInfo ** typeInfo) = 0;

    virtual CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo ** moduleInfo) = 0;

    virtual CARAPI IsDummy(
        /* [out] */ _ELASTOS Boolean * isDummy) = 0;

    virtual CARAPI GetPtrLevel(
        /* [out] */ _ELASTOS Int32 * level) = 0;

};

CAR_INTERFACE("F43FEE3D-1552-3F8D-C885-DBA7F95F95A5")
IArgumentList : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IArgumentList*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IArgumentList*)pObj->Probe(EIID_IArgumentList);
    }

    static CARAPI_(IArgumentList*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IArgumentList*)pObj->Probe(EIID_IArgumentList);
    }

    virtual CARAPI GetFunctionInfo(
        /* [out] */ IFunctionInfo ** functionInfo) = 0;

    virtual CARAPI SetInputArgumentOfInt16(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int16 value) = 0;

    virtual CARAPI SetInputArgumentOfInt32(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int32 value) = 0;

    virtual CARAPI SetInputArgumentOfInt64(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int64 value) = 0;

    virtual CARAPI SetInputArgumentOfByte(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Byte value) = 0;

    virtual CARAPI SetInputArgumentOfFloat(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Float value) = 0;

    virtual CARAPI SetInputArgumentOfDouble(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Double value) = 0;

    virtual CARAPI SetInputArgumentOfChar(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Char32 value) = 0;

    virtual CARAPI SetInputArgumentOfString(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ const _ELASTOS String& value) = 0;

    virtual CARAPI SetInputArgumentOfBoolean(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Boolean value) = 0;

    virtual CARAPI SetInputArgumentOfEMuid(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS EMuid * value) = 0;

    virtual CARAPI SetInputArgumentOfEGuid(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS EGuid * value) = 0;

    virtual CARAPI SetInputArgumentOfECode(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS ECode value) = 0;

    virtual CARAPI SetInputArgumentOfLocalPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ LocalPtr value) = 0;

    virtual CARAPI SetInputArgumentOfLocalType(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS PVoid value) = 0;

    virtual CARAPI SetInputArgumentOfEnum(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int32 value) = 0;

    virtual CARAPI SetInputArgumentOfCarArray(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS PCarQuintet value) = 0;

    virtual CARAPI SetInputArgumentOfStructPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS PVoid value) = 0;

    virtual CARAPI SetInputArgumentOfObjectPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ PInterface value) = 0;

    virtual CARAPI SetOutputArgumentOfInt16Ptr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int16 * value) = 0;

    virtual CARAPI SetOutputArgumentOfInt32Ptr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI SetOutputArgumentOfInt64Ptr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int64 * value) = 0;

    virtual CARAPI SetOutputArgumentOfBytePtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Byte * value) = 0;

    virtual CARAPI SetOutputArgumentOfFloatPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Float * value) = 0;

    virtual CARAPI SetOutputArgumentOfDoublePtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Double * value) = 0;

    virtual CARAPI SetOutputArgumentOfCharPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Char32 * value) = 0;

    virtual CARAPI SetOutputArgumentOfStringPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS String * value) = 0;

    virtual CARAPI SetOutputArgumentOfBooleanPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Boolean * value) = 0;

    virtual CARAPI SetOutputArgumentOfEMuidPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS EMuid * value) = 0;

    virtual CARAPI SetOutputArgumentOfEGuidPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS EGuid * value) = 0;

    virtual CARAPI SetOutputArgumentOfECodePtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS ECode * value) = 0;

    virtual CARAPI SetOutputArgumentOfLocalPtrPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ LocalPtr * value) = 0;

    virtual CARAPI SetOutputArgumentOfLocalTypePtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS PVoid value) = 0;

    virtual CARAPI SetOutputArgumentOfEnumPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI SetOutputArgumentOfCarArrayPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS PCarQuintet value) = 0;

    virtual CARAPI SetOutputArgumentOfCarArrayPtrPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS PCarQuintet * value) = 0;

    virtual CARAPI SetOutputArgumentOfStructPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS PVoid value) = 0;

    virtual CARAPI SetOutputArgumentOfStructPtrPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS PVoid * value) = 0;

    virtual CARAPI SetOutputArgumentOfObjectPtrPtr(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ PInterface * value) = 0;

};

CAR_INTERFACE("47D4242C-F712-E41F-96EF-239781048F17")
IVariable : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IVariable*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IVariable*)pObj->Probe(EIID_IVariable);
    }

    static CARAPI_(IVariable*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IVariable*)pObj->Probe(EIID_IVariable);
    }

    virtual CARAPI GetTypeInfo(
        /* [out] */ IDataTypeInfo ** typeInfo) = 0;

    virtual CARAPI GetPayload(
        /* [out] */ _ELASTOS PVoid * payload) = 0;

    virtual CARAPI Rebox(
        /* [in] */ _ELASTOS PVoid localVariablePtr) = 0;

};

CAR_INTERFACE("0E754A32-F712-0CA5-48D4-C0F2839CE61B")
IVariableOfCarArray : public IVariable
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IVariableOfCarArray*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IVariableOfCarArray*)pObj->Probe(EIID_IVariableOfCarArray);
    }

    static CARAPI_(IVariableOfCarArray*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IVariableOfCarArray*)pObj->Probe(EIID_IVariableOfCarArray);
    }

    virtual CARAPI GetSetter(
        /* [out] */ ICarArraySetter ** setter) = 0;

    virtual CARAPI GetGetter(
        /* [out] */ ICarArrayGetter ** getter) = 0;

};

CAR_INTERFACE("6EA54827-F712-B1A5-7F90-D37CE33E3972")
IVariableOfStruct : public IVariable
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IVariableOfStruct*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IVariableOfStruct*)pObj->Probe(EIID_IVariableOfStruct);
    }

    static CARAPI_(IVariableOfStruct*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IVariableOfStruct*)pObj->Probe(EIID_IVariableOfStruct);
    }

    virtual CARAPI GetSetter(
        /* [out] */ IStructSetter ** setter) = 0;

    virtual CARAPI GetGetter(
        /* [out] */ IStructGetter ** getter) = 0;

};

CAR_INTERFACE("9702EE24-0312-3512-B068-BE71FF87E6EC")
ICarArraySetter : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICarArraySetter*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICarArraySetter*)pObj->Probe(EIID_ICarArraySetter);
    }

    static CARAPI_(ICarArraySetter*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICarArraySetter*)pObj->Probe(EIID_ICarArraySetter);
    }

    virtual CARAPI SetUsed(
        /* [in] */ _ELASTOS Int32 used) = 0;

    virtual CARAPI SetInt16Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int16 value) = 0;

    virtual CARAPI SetInt32Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int32 value) = 0;

    virtual CARAPI SetInt64Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int64 value) = 0;

    virtual CARAPI SetByteElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Byte value) = 0;

    virtual CARAPI SetFloatElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Float value) = 0;

    virtual CARAPI SetDoubleElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Double value) = 0;

    virtual CARAPI SetEnumElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int32 value) = 0;

    virtual CARAPI SetCharElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Char32 value) = 0;

    virtual CARAPI SetStringElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ const _ELASTOS String& value) = 0;

    virtual CARAPI SetBooleanElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Boolean value) = 0;

    virtual CARAPI SetEMuidElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS EMuid * value) = 0;

    virtual CARAPI SetEGuidElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS EGuid * value) = 0;

    virtual CARAPI SetECodeElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS ECode value) = 0;

    virtual CARAPI SetLocalTypeElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS PVoid value) = 0;

    virtual CARAPI SetObjectPtrElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ PInterface value) = 0;

    virtual CARAPI GetStructElementSetter(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ IStructSetter ** setter) = 0;

};

CAR_INTERFACE("2FD8F922-0312-3512-B090-BB71FF07393A")
ICarArrayGetter : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICarArrayGetter*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICarArrayGetter*)pObj->Probe(EIID_ICarArrayGetter);
    }

    static CARAPI_(ICarArrayGetter*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICarArrayGetter*)pObj->Probe(EIID_ICarArrayGetter);
    }

    virtual CARAPI GetCapacity(
        /* [out] */ _ELASTOS Int32 * capacity) = 0;

    virtual CARAPI GetUsed(
        /* [out] */ _ELASTOS Int32 * used) = 0;

    virtual CARAPI GetInt16Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int16 * value) = 0;

    virtual CARAPI GetInt32Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI GetInt64Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int64 * value) = 0;

    virtual CARAPI GetByteElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Byte * value) = 0;

    virtual CARAPI GetFloatElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Float * value) = 0;

    virtual CARAPI GetDoubleElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Double * value) = 0;

    virtual CARAPI GetEnumElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI GetCharElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Char32 * value) = 0;

    virtual CARAPI GetStringElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS String * value) = 0;

    virtual CARAPI GetBooleanElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Boolean * value) = 0;

    virtual CARAPI GetEMuidElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS EMuid * value) = 0;

    virtual CARAPI GetEGuidElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS EGuid * value) = 0;

    virtual CARAPI GetECodeElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS ECode * value) = 0;

    virtual CARAPI GetLocalTypeElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS PVoid value) = 0;

    virtual CARAPI GetObjectPtrElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ PInterface * value) = 0;

    virtual CARAPI GetStructElementGetter(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ IStructGetter ** getter) = 0;

};

CAR_INTERFACE("239FEC35-6C52-37CD-EEFF-58BFE4010B14")
IStructSetter : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IStructSetter*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IStructSetter*)pObj->Probe(EIID_IStructSetter);
    }

    static CARAPI_(IStructSetter*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IStructSetter*)pObj->Probe(EIID_IStructSetter);
    }

    virtual CARAPI ZeroAllFields() = 0;

    virtual CARAPI SetInt16Field(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS Int16 value) = 0;

    virtual CARAPI SetInt32Field(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS Int32 value) = 0;

    virtual CARAPI SetInt64Field(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS Int64 value) = 0;

    virtual CARAPI SetByteField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS Byte value) = 0;

    virtual CARAPI SetFloatField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS Float value) = 0;

    virtual CARAPI SetDoubleField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS Double value) = 0;

    virtual CARAPI SetCharField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS Char32 value) = 0;

    virtual CARAPI SetBooleanField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS Boolean value) = 0;

    virtual CARAPI SetEMuidField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS EMuid * value) = 0;

    virtual CARAPI SetEGuidField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS EGuid * value) = 0;

    virtual CARAPI SetECodeField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS ECode value) = 0;

    virtual CARAPI SetLocalPtrField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ LocalPtr value) = 0;

    virtual CARAPI SetLocalTypeField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS PVoid value) = 0;

    virtual CARAPI SetEnumField(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ _ELASTOS Int32 value) = 0;

    virtual CARAPI GetStructFieldSetter(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IStructSetter ** setter) = 0;

    virtual CARAPI GetCppVectorFieldSetter(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ ICppVectorSetter ** setter) = 0;

};

CAR_INTERFACE("7F0BF622-6C52-3772-EEFF-205730808107")
IStructGetter : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IStructGetter*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IStructGetter*)pObj->Probe(EIID_IStructGetter);
    }

    static CARAPI_(IStructGetter*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IStructGetter*)pObj->Probe(EIID_IStructGetter);
    }

    virtual CARAPI GetInt16Field(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS Int16 * value) = 0;

    virtual CARAPI GetInt32Field(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI GetInt64Field(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS Int64 * value) = 0;

    virtual CARAPI GetByteField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS Byte * value) = 0;

    virtual CARAPI GetFloatField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS Float * value) = 0;

    virtual CARAPI GetDoubleField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS Double * value) = 0;

    virtual CARAPI GetCharField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS Char32 * value) = 0;

    virtual CARAPI GetBooleanField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS Boolean * value) = 0;

    virtual CARAPI GetEMuidField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS EMuid * value) = 0;

    virtual CARAPI GetEGuidField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS EGuid * value) = 0;

    virtual CARAPI GetECodeField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS ECode * value) = 0;

    virtual CARAPI GetLocalPtrField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ LocalPtr * value) = 0;

    virtual CARAPI GetLocalTypeField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS PVoid value) = 0;

    virtual CARAPI GetEnumField(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI GetStructFieldGetter(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IStructGetter ** getter) = 0;

    virtual CARAPI GetCppVectorFieldGetter(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ ICppVectorGetter ** getter) = 0;

};

CAR_INTERFACE("49F0F950-0312-1FB4-885D-5593F9C78867")
ICppVectorSetter : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICppVectorSetter*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICppVectorSetter*)pObj->Probe(EIID_ICppVectorSetter);
    }

    static CARAPI_(ICppVectorSetter*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICppVectorSetter*)pObj->Probe(EIID_ICppVectorSetter);
    }

    virtual CARAPI ZeroAllElements() = 0;

    virtual CARAPI SetAllElements(
        /* [in] */ _ELASTOS PVoid value,
        /* [in] */ _ELASTOS MemorySize size) = 0;

    virtual CARAPI SetInt16Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int16 value) = 0;

    virtual CARAPI SetInt32Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int32 value) = 0;

    virtual CARAPI SetInt64Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int64 value) = 0;

    virtual CARAPI SetByteElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Byte value) = 0;

    virtual CARAPI SetFloatElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Float value) = 0;

    virtual CARAPI SetDoubleElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Double value) = 0;

    virtual CARAPI SetCharElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Char32 value) = 0;

    virtual CARAPI SetBooleanElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Boolean value) = 0;

    virtual CARAPI SetEMuidElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS EMuid * value) = 0;

    virtual CARAPI SetEGuidElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS EGuid * value) = 0;

    virtual CARAPI SetECodeElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS ECode value) = 0;

    virtual CARAPI SetLocalPtrElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ LocalPtr value) = 0;

    virtual CARAPI SetLocalTypeElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS PVoid value) = 0;

    virtual CARAPI SetEnumElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [in] */ _ELASTOS Int32 value) = 0;

    virtual CARAPI GetStructElementSetter(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ IStructSetter ** setter) = 0;

    virtual CARAPI GetCppVectorElementSetter(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ ICppVectorSetter ** setter) = 0;

};

CAR_INTERFACE("931AFA4C-0312-1FB4-885D-AD92F947950A")
ICppVectorGetter : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICppVectorGetter*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICppVectorGetter*)pObj->Probe(EIID_ICppVectorGetter);
    }

    static CARAPI_(ICppVectorGetter*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICppVectorGetter*)pObj->Probe(EIID_ICppVectorGetter);
    }

    virtual CARAPI GetLength(
        /* [out] */ _ELASTOS Int32 * length) = 0;

    virtual CARAPI GetRank(
        /* [out] */ _ELASTOS Int32 * rank) = 0;

    virtual CARAPI GetInt16Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int16 * value) = 0;

    virtual CARAPI GetInt32Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI GetInt64Element(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int64 * value) = 0;

    virtual CARAPI GetByteElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Byte * value) = 0;

    virtual CARAPI GetFloatElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Float * value) = 0;

    virtual CARAPI GetDoubleElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Double * value) = 0;

    virtual CARAPI GetCharElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Char32 * value) = 0;

    virtual CARAPI GetBooleanElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Boolean * value) = 0;

    virtual CARAPI GetEMuidElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS EMuid * value) = 0;

    virtual CARAPI GetEGuidElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS EGuid * value) = 0;

    virtual CARAPI GetECodeElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS ECode * value) = 0;

    virtual CARAPI GetLocalPtrElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ LocalPtr * value) = 0;

    virtual CARAPI GetLocalTypeElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS PVoid value) = 0;

    virtual CARAPI GetEnumElement(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI GetStructElementGetter(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ IStructGetter ** getter) = 0;

    virtual CARAPI GetCppVectorElementGetter(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ ICppVectorGetter ** getter) = 0;

};

CAR_INTERFACE("EB08F228-CE52-3F7D-C865-25079E9C9E4A")
IModuleInfo : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IModuleInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IModuleInfo*)pObj->Probe(EIID_IModuleInfo);
    }

    static CARAPI_(IModuleInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IModuleInfo*)pObj->Probe(EIID_IModuleInfo);
    }

    virtual CARAPI GetPath(
        /* [out] */ _ELASTOS String * path) = 0;

    virtual CARAPI GetVersion(
        /* [out] */ _ELASTOS Int32 * major,
        /* [out] */ _ELASTOS Int32 * minor,
        /* [out] */ _ELASTOS Int32 * build,
        /* [out] */ _ELASTOS Int32 * revision) = 0;

    virtual CARAPI GetClassCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllClassInfos(
        /* [out] */ _ELASTOS ArrayOf<IClassInfo *> * classInfos) = 0;

    virtual CARAPI GetClassInfo(
        /* [in] */ const _ELASTOS String& fullName,
        /* [out] */ IClassInfo ** classInfo) = 0;

    virtual CARAPI GetInterfaceCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllInterfaceInfos(
        /* [out] */ _ELASTOS ArrayOf<IInterfaceInfo *> * interfaceInfos) = 0;

    virtual CARAPI GetInterfaceInfo(
        /* [in] */ const _ELASTOS String& fullName,
        /* [out] */ IInterfaceInfo ** interfaceInfo) = 0;

    virtual CARAPI GetStructCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllStructInfos(
        /* [out] */ _ELASTOS ArrayOf<IStructInfo *> * structInfos) = 0;

    virtual CARAPI GetStructInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IStructInfo ** structInfo) = 0;

    virtual CARAPI GetEnumCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllEnumInfos(
        /* [out] */ _ELASTOS ArrayOf<IEnumInfo *> * enumInfos) = 0;

    virtual CARAPI GetEnumInfo(
        /* [in] */ const _ELASTOS String& fullName,
        /* [out] */ IEnumInfo ** enumInfo) = 0;

    virtual CARAPI GetTypeAliasCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllTypeAliasInfos(
        /* [out] */ _ELASTOS ArrayOf<ITypeAliasInfo *> * typeAliasInfos) = 0;

    virtual CARAPI GetTypeAliasInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ ITypeAliasInfo ** typeAliasInfo) = 0;

    virtual CARAPI GetConstantCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllConstantInfos(
        /* [out] */ _ELASTOS ArrayOf<IConstantInfo *> * constantInfos) = 0;

    virtual CARAPI GetConstantInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IConstantInfo ** constantInfo) = 0;

    virtual CARAPI GetImportModuleInfoCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllImportModuleInfos(
        /* [out] */ _ELASTOS ArrayOf<IModuleInfo *> * moduleInfos) = 0;

};

CAR_INTERFACE("F4DC2A4D-1112-82C8-6E68-F951A5CBCE2A")
IConstantInfo : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IConstantInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IConstantInfo*)pObj->Probe(EIID_IConstantInfo);
    }

    static CARAPI_(IConstantInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IConstantInfo*)pObj->Probe(EIID_IConstantInfo);
    }

    virtual CARAPI GetName(
        /* [out] */ _ELASTOS String * name) = 0;

    virtual CARAPI GetValue(
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo ** moduleInfo) = 0;

};

CAR_INTERFACE("7166FA24-9E92-3F7D-C805-2517D4C60C12")
IClassInfo : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IClassInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IClassInfo*)pObj->Probe(EIID_IClassInfo);
    }

    static CARAPI_(IClassInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IClassInfo*)pObj->Probe(EIID_IClassInfo);
    }

    virtual CARAPI GetName(
        /* [out] */ _ELASTOS String * name) = 0;

    virtual CARAPI GetNamespace(
        /* [out] */ _ELASTOS String * ns) = 0;

    virtual CARAPI GetId(
        /* [out] */ _ELASTOS ClassID * clsid) = 0;

    virtual CARAPI GetModuleInfo(
        /* [out] */ IModuleInfo ** moduleInfo) = 0;

    virtual CARAPI GetClassLoader(
        /* [out] */ IInterface ** loader) = 0;

    virtual CARAPI SetClassLoader(
        /* [in] */ IInterface * loader) = 0;

    virtual CARAPI IsSingleton(
        /* [out] */ _ELASTOS Boolean * isSingleton) = 0;

    virtual CARAPI GetThreadingModel(
        /* [out] */ ThreadingModel * threadingModel) = 0;

    virtual CARAPI IsPrivate(
        /* [out] */ _ELASTOS Boolean * isPrivate) = 0;

    virtual CARAPI IsReturnValue(
        /* [out] */ _ELASTOS Boolean * returnValue) = 0;

    virtual CARAPI IsBaseClass(
        /* [out] */ _ELASTOS Boolean * isBaseClass) = 0;

    virtual CARAPI HasBaseClass(
        /* [out] */ _ELASTOS Boolean * hasBaseClass) = 0;

    virtual CARAPI GetBaseClassInfo(
        /* [out] */ IClassInfo ** baseClassInfo) = 0;

    virtual CARAPI IsGeneric(
        /* [out] */ _ELASTOS Boolean * isGeneric) = 0;

    virtual CARAPI HasGeneric(
        /* [out] */ _ELASTOS Boolean * hasGeneric) = 0;

    virtual CARAPI GetGenericInfo(
        /* [out] */ IClassInfo ** genericInfo) = 0;

    virtual CARAPI IsRegime(
        /* [out] */ _ELASTOS Boolean * isRegime) = 0;

    virtual CARAPI GetAspectCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllAspectInfos(
        /* [out] */ _ELASTOS ArrayOf<IClassInfo *> * aspectInfos) = 0;

    virtual CARAPI GetAspectInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IClassInfo ** aspectInfo) = 0;

    virtual CARAPI IsAspect(
        /* [out] */ _ELASTOS Boolean * isAspect) = 0;

    virtual CARAPI GetAggregateeCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllAggregateeInfos(
        /* [out] */ _ELASTOS ArrayOf<IClassInfo *> * aggregateeInfos) = 0;

    virtual CARAPI GetAggregateeInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IClassInfo ** aggregateeInfo) = 0;

    virtual CARAPI GetConstructorCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllConstructorInfos(
        /* [out] */ _ELASTOS ArrayOf<IConstructorInfo *> * constructorInfos) = 0;

    virtual CARAPI GetConstructorInfoByParamNames(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IConstructorInfo ** constructorInfo) = 0;

    virtual CARAPI GetConstructorInfoByParamCount(
        /* [in] */ _ELASTOS Int32 count,
        /* [out] */ IConstructorInfo ** constructorInfo) = 0;

    virtual CARAPI GetInterfaceCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllInterfaceInfos(
        /* [out] */ _ELASTOS ArrayOf<IInterfaceInfo *> * interfaceInfos) = 0;

    virtual CARAPI GetInterfaceInfo(
        /* [in] */ const _ELASTOS String& fullName,
        /* [out] */ IInterfaceInfo ** interfaceInfo) = 0;

    virtual CARAPI HasInterfaceInfo(
        /* [in] */ IInterfaceInfo * interfaceInfo,
        /* [out] */ _ELASTOS Boolean * result) = 0;

    virtual CARAPI GetCallbackInterfaceCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllCallbackInterfaceInfos(
        /* [out] */ _ELASTOS ArrayOf<IInterfaceInfo *> * callbackInterfaceInfos) = 0;

    virtual CARAPI GetCallbackInterfaceInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ IInterfaceInfo ** callbackInterfaceInfo) = 0;

    virtual CARAPI GetMethodCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllMethodInfos(
        /* [out] */ _ELASTOS ArrayOf<IMethodInfo *> * methodInfos) = 0;

    virtual CARAPI GetMethodInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [in] */ const _ELASTOS String& signature,
        /* [out] */ IMethodInfo ** methodInfo) = 0;

    virtual CARAPI GetCallbackMethodCount(
        /* [out] */ _ELASTOS Int32 * count) = 0;

    virtual CARAPI GetAllCallbackMethodInfos(
        /* [out] */ _ELASTOS ArrayOf<ICallbackMethodInfo *> * callbackMethodInfos) = 0;

    virtual CARAPI GetCallbackMethodInfo(
        /* [in] */ const _ELASTOS String& name,
        /* [out] */ ICallbackMethodInfo ** callbackMethodInfo) = 0;

    virtual CARAPI RemoveAllCallbackHandlers(
        /* [in] */ PInterface server) = 0;

    virtual CARAPI CreateObject(
        /* [out] */ PInterface * object) = 0;

    virtual CARAPI CreateObjectInRegime(
        /* [in] */ PRegime rgm,
        /* [out] */ PInterface * object) = 0;

};

CAR_INTERFACE("D811A75D-1112-7FC8-5F5D-D5F2A38A8218")
IConstructorInfo : public IFunctionInfo
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IConstructorInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IConstructorInfo*)pObj->Probe(EIID_IConstructorInfo);
    }

    static CARAPI_(IConstructorInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IConstructorInfo*)pObj->Probe(EIID_IConstructorInfo);
    }

    virtual CARAPI GetAnnotation(
        /* [out] */ _ELASTOS String * annotation) = 0;

    virtual CARAPI CreateArgumentList(
        /* [out] */ IArgumentList ** argumentList) = 0;

    virtual CARAPI CreateObject(
        /* [in] */ IArgumentList * argumentList,
        /* [out] */ PInterface * object) = 0;

    virtual CARAPI CreateObjectInRegime(
        /* [in] */ PRegime rgm,
        /* [in] */ IArgumentList * argumentList,
        /* [out] */ PInterface * object) = 0;

};

CAR_INTERFACE("81599734-9C52-FB9A-7E18-89D3378E2554")
ICallbackMethodInfo : public IFunctionInfo
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICallbackMethodInfo*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICallbackMethodInfo*)pObj->Probe(EIID_ICallbackMethodInfo);
    }

    static CARAPI_(ICallbackMethodInfo*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICallbackMethodInfo*)pObj->Probe(EIID_ICallbackMethodInfo);
    }

    virtual CARAPI AddCallback(
        /* [in] */ PInterface server,
        /* [in] */ _ELASTOS EventHandler handler) = 0;

    virtual CARAPI RemoveCallback(
        /* [in] */ PInterface server,
        /* [in] */ _ELASTOS EventHandler handler) = 0;

    virtual CARAPI CreateDelegateProxy(
        /* [in] */ _ELASTOS PVoid targetObject,
        /* [in] */ _ELASTOS PVoid targetMethod,
        /* [in] */ ICallbackInvocation * callbackInvocation,
        /* [out] */ IDelegateProxy ** delegateProxy) = 0;

};

CAR_INTERFACE("CAF76221-AA12-350C-64EC-E0F1835C9C9A")
IDelegateProxy : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IDelegateProxy*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IDelegateProxy*)pObj->Probe(EIID_IDelegateProxy);
    }

    static CARAPI_(IDelegateProxy*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IDelegateProxy*)pObj->Probe(EIID_IDelegateProxy);
    }

    virtual CARAPI GetCallbackMethodInfo(
        /* [out] */ ICallbackMethodInfo ** callbackMethodInfo) = 0;

    virtual CARAPI GetTargetObject(
        /* [out] */ _ELASTOS PVoid * targetObject) = 0;

    virtual CARAPI GetTargetMethod(
        /* [out] */ _ELASTOS PVoid * targetMethod) = 0;

    virtual CARAPI GetCallbackInvocation(
        /* [out] */ ICallbackInvocation ** callbackInvocation) = 0;

    virtual CARAPI GetDelegate(
        /* [out] */ _ELASTOS EventHandler * handler) = 0;

};

CAR_INTERFACE("10DA1A2E-9C52-3983-6498-2001FFC7C11C")
ICallbackInvocation : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICallbackInvocation*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICallbackInvocation*)pObj->Probe(EIID_ICallbackInvocation);
    }

    static CARAPI_(ICallbackInvocation*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICallbackInvocation*)pObj->Probe(EIID_ICallbackInvocation);
    }

    virtual CARAPI Invoke(
        /* [in] */ _ELASTOS PVoid targetObject,
        /* [in] */ _ELASTOS PVoid targetMethod,
        /* [in] */ ICallbackArgumentList * callbackArgumentList) = 0;

};

CAR_INTERFACE("A23DF130-9C52-1A2A-7F90-8B53731F39CC")
ICallbackArgumentList : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICallbackArgumentList*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICallbackArgumentList*)pObj->Probe(EIID_ICallbackArgumentList);
    }

    static CARAPI_(ICallbackArgumentList*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICallbackArgumentList*)pObj->Probe(EIID_ICallbackArgumentList);
    }

    virtual CARAPI GetCallbackMethodInfo(
        /* [out] */ ICallbackMethodInfo ** callbackMethodInfo) = 0;

    virtual CARAPI GetServerPtrArgument(
        /* [out] */ PInterface * server) = 0;

    virtual CARAPI GetInt16Argument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int16 * value) = 0;

    virtual CARAPI GetInt32Argument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI GetInt64Argument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int64 * value) = 0;

    virtual CARAPI GetByteArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Byte * value) = 0;

    virtual CARAPI GetFloatArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Float * value) = 0;

    virtual CARAPI GetDoubleArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Double * value) = 0;

    virtual CARAPI GetCharArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Char32 * value) = 0;

    virtual CARAPI GetStringArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS String * value) = 0;

    virtual CARAPI GetBooleanArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Boolean * value) = 0;

    virtual CARAPI GetEMuidArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS EMuid ** value) = 0;

    virtual CARAPI GetEGuidArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS EGuid ** value) = 0;

    virtual CARAPI GetECodeArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS ECode * value) = 0;

    virtual CARAPI GetLocalPtrArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ LocalPtr * value) = 0;

    virtual CARAPI GetEnumArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI GetCarArrayArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS PCarQuintet * value) = 0;

    virtual CARAPI GetStructPtrArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ _ELASTOS PVoid * value) = 0;

    virtual CARAPI GetObjectPtrArgument(
        /* [in] */ _ELASTOS Int32 index,
        /* [out] */ PInterface * value) = 0;

};

CAR_INTERFACE("AACEFA4F-8652-A044-5EFC-B09088704C3A")
IParcel : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IParcel*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IParcel*)pObj->Probe(EIID_IParcel);
    }

    static CARAPI_(IParcel*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IParcel*)pObj->Probe(EIID_IParcel);
    }

    virtual CARAPI Marshall(
        /* [out, callee] */ _ELASTOS ArrayOf<_ELASTOS Byte> ** bytes) = 0;

    virtual CARAPI Unmarshall(
        /* [in] */ _ELASTOS ArrayOf<_ELASTOS Byte> * data,
        /* [in] */ _ELASTOS Int32 offest,
        /* [in] */ _ELASTOS Int32 length) = 0;

    virtual CARAPI AppendFrom(
        /* [in] */ IParcel * parcel,
        /* [in] */ _ELASTOS Int32 offset,
        /* [in] */ _ELASTOS Int32 length) = 0;

    virtual CARAPI HasFileDescriptors(
        /* [out] */ _ELASTOS Boolean * result) = 0;

    virtual CARAPI ReadByte(
        /* [out] */ _ELASTOS Byte * value) = 0;

    virtual CARAPI WriteByte(
        /* [in] */ _ELASTOS Byte value) = 0;

    virtual CARAPI ReadBoolean(
        /* [out] */ _ELASTOS Boolean * value) = 0;

    virtual CARAPI WriteBoolean(
        /* [in] */ _ELASTOS Boolean value) = 0;

    virtual CARAPI ReadChar(
        /* [out] */ _ELASTOS Char32 * value) = 0;

    virtual CARAPI WriteChar(
        /* [in] */ _ELASTOS Char32 value) = 0;

    virtual CARAPI ReadInt16(
        /* [out] */ _ELASTOS Int16 * value) = 0;

    virtual CARAPI WriteInt16(
        /* [in] */ _ELASTOS Int16 value) = 0;

    virtual CARAPI ReadInt32(
        /* [out] */ _ELASTOS Int32 * value) = 0;

    virtual CARAPI WriteInt32(
        /* [in] */ _ELASTOS Int32 value) = 0;

    virtual CARAPI ReadInt64(
        /* [out] */ _ELASTOS Int64 * value) = 0;

    virtual CARAPI WriteInt64(
        /* [in] */ _ELASTOS Int64 value) = 0;

    virtual CARAPI ReadFloat(
        /* [out] */ _ELASTOS Float * value) = 0;

    virtual CARAPI WriteFloat(
        /* [in] */ _ELASTOS Float value) = 0;

    virtual CARAPI ReadDouble(
        /* [out] */ _ELASTOS Double * value) = 0;

    virtual CARAPI WriteDouble(
        /* [in] */ _ELASTOS Double value) = 0;

    virtual CARAPI ReadString(
        /* [out] */ _ELASTOS String * str) = 0;

    virtual CARAPI WriteString(
        /* [in] */ const _ELASTOS String& str) = 0;

    virtual CARAPI ReadStruct(
        /* [out] */ _ELASTOS Handle32 * address) = 0;

    virtual CARAPI WriteStruct(
        /* [in] */ _ELASTOS Handle32 value,
        /* [in] */ _ELASTOS Int32 size) = 0;

    virtual CARAPI ReadEMuid(
        /* [out] */ _ELASTOS EMuid * id) = 0;

    virtual CARAPI WriteEMuid(
        /* [in] */ const _ELASTOS EMuid & id) = 0;

    virtual CARAPI ReadEGuid(
        /* [out] */ _ELASTOS EGuid * id) = 0;

    virtual CARAPI WriteEGuid(
        /* [in] */ const _ELASTOS EGuid & id) = 0;

    virtual CARAPI ReadArrayOf(
        /* [out] */ _ELASTOS Handle32 * array) = 0;

    virtual CARAPI WriteArrayOf(
        /* [in] */ _ELASTOS Handle32 array) = 0;

    virtual CARAPI ReadArrayOfString(
        /* [out, callee] */ _ELASTOS ArrayOf<_ELASTOS String> ** array) = 0;

    virtual CARAPI WriteArrayOfString(
        /* [in] */ _ELASTOS ArrayOf<_ELASTOS String> * array) = 0;

    virtual CARAPI ReadInterfacePtr(
        /* [out] */ _ELASTOS Handle32 * itfpp) = 0;

    virtual CARAPI WriteInterfacePtr(
        /* [in] */ IInterface * value) = 0;

    virtual CARAPI WriteFileDescriptor(
        /* [in] */ _ELASTOS Int32 fd) = 0;

    virtual CARAPI WriteDupFileDescriptor(
        /* [in] */ _ELASTOS Int32 fd) = 0;

    virtual CARAPI ReadFileDescriptor(
        /* [out] */ _ELASTOS Int32 * fd) = 0;

    virtual CARAPI Clone(
        /* [in] */ IParcel * srcParcel) = 0;

    virtual CARAPI GetDataPosition(
        /* [out] */ _ELASTOS Int32 * position) = 0;

    virtual CARAPI SetDataPosition(
        /* [in] */ _ELASTOS Int32 position) = 0;

    virtual CARAPI GetElementPayload(
        /* [out] */ _ELASTOS Handle32 * buffer) = 0;

    virtual CARAPI GetElementSize(
        /* [out] */ _ELASTOS Int32 * size) = 0;

};

CAR_INTERFACE("30D12755-8652-A044-4C5A-FA71664A8644")
IParcelable : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(IParcelable*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (IParcelable*)pObj->Probe(EIID_IParcelable);
    }

    static CARAPI_(IParcelable*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (IParcelable*)pObj->Probe(EIID_IParcelable);
    }

    virtual CARAPI ReadFromParcel(
        /* [in] */ IParcel * source) = 0;

    virtual CARAPI WriteToParcel(
        /* [in] */ IParcel * dest) = 0;

};

CAR_INTERFACE("05782314-8312-B7DB-0C5B-646DAFE4FB43")
ICustomMarshal : public IInterface
{
    virtual CARAPI_(PInterface) Probe(
        /* [in] */ _ELASTOS REIID riid) = 0;

    static CARAPI_(ICustomMarshal*) Probe(PInterface pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICustomMarshal*)pObj->Probe(EIID_ICustomMarshal);
    }

    static CARAPI_(ICustomMarshal*) Probe(IObject* pObj)
    {
        if (pObj == NULL) return NULL;
        return (ICustomMarshal*)pObj->Probe(EIID_ICustomMarshal);
    }

    virtual CARAPI GetClsid(
        /* [out] */ _ELASTOS ClassID * clsid) = 0;

    virtual CARAPI CreateObject(
        /* [in] */ ICustomMarshal * originProxy,
        /* [out] */ IInterface ** newProxy) = 0;

};

#endif // ___ElastosCore_h__
