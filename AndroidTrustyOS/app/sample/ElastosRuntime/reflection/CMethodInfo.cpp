//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CMethodInfo.h"
#include "CParamInfo.h"
#include "CArgumentList.h"
#include "CCallbackArgumentList.h"

#define INVALID_PARAM_COUNT 0xFFFFFFFF

EXTERN_C int invoke(void* func, int* param, int size);

struct VTable
{
    void* mMethods[1];
};

struct VObject
{
    VTable* mVtab;
};

CMethodInfo::CMethodInfo(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ MethodDescriptor* methodDescriptor,
    /* [in] */ UInt32 index)
{
    mMethodDescriptor = methodDescriptor;
    mIndex = index;
    mClsModule = clsModule;
    mClsMod = mClsModule->mClsMod;
    mParamElem = NULL;
    mParameterInfos = NULL;
    mBase = mClsModule->mBase;
}

CMethodInfo::~CMethodInfo()
{
    Int32 count = mMethodDescriptor->mParamCount;
    if (mParameterInfos) {
        for (Int32 i = 0; i < count; i++) {
            if ((*mParameterInfos)[i]) {
                delete (CParamInfo*)(*mParameterInfos)[i];
                (*mParameterInfos)[i] = NULL;
            }
        }
        ArrayOf<IParamInfo *>::Free(mParameterInfos);
    }

    if (mParamElem) {
        delete [] mParamElem;
    }
}

UInt32 CMethodInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CMethodInfo::Release()
{
    g_objInfoList.LockHashTable(EntryType_Method);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        g_objInfoList.RemoveMethodInfo(mMethodDescriptor, mIndex);
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_Method);
    assert(ref >= 0);
    return ref;
}

PInterface CMethodInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IMethodInfo) {
        return (IMethodInfo *)this;
    }
    else if (riid == EIID_IFunctionInfo) {
        return (IFunctionInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CMethodInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CMethodInfo::Init()
{
    ECode ec = InitParmElement();
    if (FAILED(ec)) return ec;

    return InitParamInfos();
}

ECode CMethodInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = adjustNameAddr(mBase, mMethodDescriptor->mName);
    return NOERROR;
}

ECode CMethodInfo::GetAnnotation(
    /* [out] */ String* annotation)
{
    if (annotation == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *annotation = adjustNameAddr(mBase, mMethodDescriptor->mAnnotation);

    return NOERROR;
}

ECode CMethodInfo::GetParamCount(
    /* [out] */ Int32* count)
{
    if (!count) {
        return E_INVALID_ARGUMENT;
    }

    *count = mMethodDescriptor->mParamCount;
    return NOERROR;
}

ECode CMethodInfo::GetAllParamInfos(
    /* [out] */ ArrayOf<IParamInfo *>* paramInfos)
{
    if (!paramInfos || !paramInfos->GetLength()) {
        return E_INVALID_ARGUMENT;
    }

    paramInfos->Copy(mParameterInfos);

    return NOERROR;
}

ECode CMethodInfo::GetParamInfoByIndex(
    /* [in] */ Int32 index,
    /* [out] */ IParamInfo** paramInfo)
{
    if (!paramInfo || index < 0) {
        return E_INVALID_ARGUMENT;
    }

    if (index >= mParameterInfos->GetLength()) {
        return E_DOES_NOT_EXIST;
    }

    *paramInfo = (*mParameterInfos)[index];
    (*paramInfo)->AddRef();

    return NOERROR;
}

ECode CMethodInfo::GetParamInfoByName(
    /* [in] */ const String& name,
    /* [out] */ IParamInfo** paramInfo)
{
    if (name.IsNull() || !paramInfo) {
        return E_INVALID_ARGUMENT;
    }

    if (!mMethodDescriptor->mParamCount) {
        return E_DOES_NOT_EXIST;
    }

    ParamDescriptor* param = NULL;
    for (Int32 i = 0; i < mMethodDescriptor->mParamCount; i++) {
        param = getParamDescAddr(mBase, mMethodDescriptor->mParams, i);
        if (name.Equals(adjustNameAddr(mBase, param->mName))) {
            return GetParamInfoByIndex(i, paramInfo);
        }
    }

    return E_DOES_NOT_EXIST;
}

ECode CMethodInfo::SetParamElem(
    /* [in] */ ParamDescriptor* paramDescriptor,
    /* [in] */ ParmElement* parmElement)
{
    TypeDescriptor* typeDesc = &paramDescriptor->mType;
    parmElement->mPointer = typeDesc->mPointer;

    ECode ec = NOERROR;
    if (typeDesc->mType == Type_alias) {
        ec = mClsModule->AliasToOriginal(typeDesc, &typeDesc);
        if (FAILED(ec)) {
            return ec;
        }
        parmElement->mPointer += typeDesc->mPointer;
    }

    CarDataType type = GetCarDataType(typeDesc->mType);
    //Set type and IO attrib
    if (paramDescriptor->mAttribs & ParamAttrib_in) {
        if (type == CarDataType_Interface) {
            if (parmElement->mPointer > 1) {
                type = CarDataType_LocalPtr;
            }
        }
        else if (parmElement->mPointer > 0) {
            type = CarDataType_LocalPtr;
        }
        parmElement->mAttrib = ParamIOAttribute_In;
    }
    else {
        parmElement->mAttrib = ParamIOAttribute_CallerAllocOut;

        if (parmElement->mPointer > 2) {
            type = CarDataType_LocalPtr;
            parmElement->mPointer -= 1;
        }
        else if (parmElement->mPointer == 2 && type != CarDataType_Interface) {
            if (type == CarDataType_String) {
                parmElement->mAttrib = ParamIOAttribute_CalleeAllocOut;
            }
            else {
                type = CarDataType_LocalPtr;
                parmElement->mPointer = 1;
            }
        }
        else if (paramDescriptor->mAttribs & ParamAttrib_callee) {
            parmElement->mAttrib = ParamIOAttribute_CalleeAllocOut;
            parmElement->mPointer = 2;
        }
    }
    parmElement->mType = type;

    //Set size
    if (typeDesc->mType == Type_struct
        || typeDesc->mType == Type_EMuid
        || typeDesc->mType == Type_EGuid
        || typeDesc->mType == Type_ArrayOf
        || typeDesc->mType == Type_Array) {
        parmElement->mSize = sizeof(PVoid);
    }
    else {
        parmElement->mSize = GetDataTypeSize(mClsModule, typeDesc);
    }

#if defined(_arm) && defined(__GNUC__) && (__GNUC__ >= 4)
    if ((typeDesc->mType == Type_Double || typeDesc->mType == Type_Int64)
        && parmElement->mAttrib != ParamIOAttribute_CallerAllocOut) {
        mParamBufSize = ROUND8(mParamBufSize);
    }
#endif

    parmElement->mPos = mParamBufSize;
    mParamBufSize += ROUND4(parmElement->mSize);

    return NOERROR;
}

ECode CMethodInfo::InitParmElement()
{
    Int32 count = mMethodDescriptor->mParamCount;
    mParamElem = new ParmElement[count];
    if (!mParamElem) {
        return E_OUT_OF_MEMORY;
    }
    memset(mParamElem, 0, sizeof(mParamElem) * count);

    mParamBufSize = 4; //For this pointer
    ECode ec = NOERROR;
    for (Int32 i = 0; i < count; i++) {
        ec = SetParamElem(
                getParamDescAddr(mBase, mMethodDescriptor->mParams, i),
                &mParamElem[i]);
        if (FAILED(ec)) goto EExit;
    }

    return NOERROR;

EExit:
    delete [] mParamElem;
    mParamElem = NULL;

    return ec;
}

ECode CMethodInfo::InitParamInfos()
{
    Int32 i = 0, count = mMethodDescriptor->mParamCount;
    mParameterInfos = ArrayOf<IParamInfo *>::Alloc(count);
    if (mParameterInfos == NULL) {
        return E_OUT_OF_MEMORY;
    }

    for (i = 0; i < count; i++) {
        // do not use mParameterInfos->Set(i, xxx), because it will
        // call xxx->AddRef()
        (*mParameterInfos)[i] = new CParamInfo(mClsModule, this,
                &mParamElem[i],
                getParamDescAddr(mBase, mMethodDescriptor->mParams, i),
                i);
        if (!(*mParameterInfos)[i]) goto EExit;
    }

    return NOERROR;

EExit:
    for (i = 0; i < count; i++) {
        if ((*mParameterInfos)[i]) {
            delete (CParamInfo*)(*mParameterInfos)[i];
            (*mParameterInfos)[i] = NULL;
        }
    }
    ArrayOf<IParamInfo *>::Free(mParameterInfos);
    mParameterInfos = NULL;

    return NOERROR;
}

ECode CMethodInfo::CreateFunctionArgumentList(
    /* [in] */ IFunctionInfo* functionInfo,
    /* [in] */ Boolean isMethodInfo,
    /* [out] */ IArgumentList** argumentList)
{
    if (!argumentList) {
        return E_INVALID_ARGUMENT;
    }

    if (!mMethodDescriptor->mParamCount) {
        return E_INVALID_OPERATION;
    }

    AutoPtr<CArgumentList> argumentListObj = new CArgumentList();
    if (argumentListObj == NULL) {
        return E_OUT_OF_MEMORY;
    }

#if defined(_arm) && defined(__GNUC__) && (__GNUC__ >= 4)
    mParamBufSize = ROUND8(mParamBufSize);
#endif

    ECode ec = argumentListObj->Init(functionInfo, mParamElem,
            mMethodDescriptor->mParamCount, mParamBufSize, isMethodInfo);
    if FAILED(ec) {
        return ec;
    }

    *argumentList = argumentListObj;
    (*argumentList)->AddRef();

    return NOERROR;
}

ECode CMethodInfo::CreateCBArgumentList(
    /* [in] */ ICallbackMethodInfo* callbackMethodInfo,
    /* [out] */ ICallbackArgumentList** cbArgumentList)
{
    AutoPtr<CCallbackArgumentList> cbArgumentListObj = new CCallbackArgumentList();
    if (cbArgumentListObj == NULL) {
        return E_OUT_OF_MEMORY;
    }

/*
The GCC4 is 64-bit types (like long long) alignment and m_pHandlerThis(4-Byte)
 is picked out when creating ParamBuf, so we must recompute the position.
*/
#if defined(_arm) && defined(__GNUC__) && (__GNUC__ >= 4)
    Int32 position = 4;
    for (Int32 i = 0; i < mMethodDescriptor->mParamCount; i++) {
        if (mParamElem[i].mType == CarDataType_Double ||
                mParamElem[i].mType == CarDataType_Int64) {
            mParamElem[i].mPos = ROUND8(position + 4) - 4;
        }
        else {
            position = ROUND4(position);
            mParamElem[i].mPos = position;
        }

        position += mParamElem[i].mSize;
    }
#endif

    ECode ec = cbArgumentListObj->Init(callbackMethodInfo, mParamElem,
            mMethodDescriptor->mParamCount, mParamBufSize);
    if FAILED(ec) {
        return ec;
    }

    *cbArgumentList = cbArgumentListObj;
    (*cbArgumentList)->AddRef();

    return NOERROR;
}

ECode CMethodInfo::CreateArgumentList(
    /* [out] */ IArgumentList** argumentList)
{
    return CreateFunctionArgumentList(this, TRUE, argumentList);
}

ECode CMethodInfo::Invoke(
    /* [in] */ PInterface target,
    /* [in] */ IArgumentList* argumentList)
{
    if (!target) {
        return E_INVALID_ARGUMENT;
    }

    Int32* paramBuf = NULL;
    if (!argumentList) {
        if (mMethodDescriptor->mParamCount) return E_INVALID_ARGUMENT;
        paramBuf = (Int32 *)alloca(4);
        if (!paramBuf) return E_OUT_OF_MEMORY;
        mParamBufSize = 4;
    }
    else {
        paramBuf = (Int32 *)((CArgumentList *)argumentList)->mParamBuf;
    }

    InterfaceDirEntry* ifDir = getInterfaceDirAddr(mBase,
            mClsMod->mInterfaceDirs, INTERFACE_INDEX(mIndex));
    EIID iid = adjustInterfaceDescAddr(mBase, ifDir->mDesc)->mIID;
    PInterface object = target->Probe(iid);
    if (!object) {
        return E_NO_INTERFACE;
    }

    //push this pointer to buf
    *(PInterface *)paramBuf = object;

    VObject* vobj = reinterpret_cast<VObject*>(object);
    void* methodAddr = vobj->mVtab->mMethods[METHOD_INDEX(mIndex)];
    return (ECode)invoke(methodAddr, paramBuf,  mParamBufSize);
}
