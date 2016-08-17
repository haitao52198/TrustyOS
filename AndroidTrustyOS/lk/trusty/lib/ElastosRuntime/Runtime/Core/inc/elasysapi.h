/** @addtogroup SystemRef
  *   @{
  */

//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __ELASYSAPI_H__
#define __ELASYSAPI_H__

_ELASTOS_NAMESPACE_BEGIN

/** @} */

/** @addtogroup CARRef
  *   @{
  */
class CReflector
{
public:
    STATIC CARAPI AcquireModuleInfo(
        /* [in] */ const String& name,
        /* [out] */ IModuleInfo** moduleInfo)
    {
        return _CReflector_AcquireModuleInfo(name, moduleInfo);
    }

    STATIC CARAPI AcquireIntrinsicTypeInfo(
        /* [in] */ CarDataType intrinsicType,
        /* [out] */ IDataTypeInfo** intrinsicTypeInfo)
    {
        return _CReflector_AcquireIntrinsicTypeInfo(intrinsicType,
                intrinsicTypeInfo);
    }

    STATIC CARAPI AcquireEnumInfo(
        /* [in] */ const String& name,
        /* [in] */ ArrayOf<String>* itemNames,
        /* [in] */ ArrayOf<Int32>* itemValues,
        /* [out] */ IEnumInfo** enumInfo)
    {
        return _CReflector_AcquireEnumInfo(name, itemNames, itemValues,
                enumInfo);
    }

    STATIC CARAPI AcquireCppVectorInfo(
        /* [in] */ IDataTypeInfo* elementTypeInfo,
        /* [in] */ Int32 length,
        /* [out] */ ICppVectorInfo** cppVectorInfo)
    {
        return _CReflector_AcquireCppVectorInfo(elementTypeInfo, length,
                cppVectorInfo);
    }

    STATIC CARAPI AcquireStructInfo(
        /* [in] */ const String& name,
        /* [in] */ ArrayOf<String>* fieldNames,
        /* [in] */ ArrayOf<IDataTypeInfo *>* fieldTypeInfos,
        /* [out] */ IStructInfo** structInfo)
    {
        return _CReflector_AcquireStructInfo(name, fieldNames,
                fieldTypeInfos, structInfo);
    }

    STATIC CARAPI AcquireCarArrayInfo(
        /* [in] */ CarDataType quintetType,
        /* [in] */ IDataTypeInfo* elementTypeInfo,
        /* [out] */ ICarArrayInfo** carArrayInfo)
    {
        return _CReflector_AcquireCarArrayInfo(quintetType, elementTypeInfo,
                carArrayInfo);
    }
};

class CObject
{
public:
    STATIC CARAPI_(Boolean) Compare(
        /* [in] */ PInterface objectA,
        /* [in] */ PInterface objectB)
    {
        return _CObject_Compare(objectA, objectB);
    }

    STATIC CARAPI EnterRegime(
        /* [in] */ PInterface object,
        /* [in] */ PRegime regime)
    {
        if (!regime) return E_NO_INTERFACE;
        return _CObject_EnterRegime(object, regime);
    }

    STATIC CARAPI LeaveRegime(
        /* [in] */ PInterface object,
        /* [in] */ PRegime regime)
    {
        if (!regime) return E_NO_INTERFACE;
        return _CObject_LeaveRegime(object, regime);
    }

    STATIC CARAPI CreateInstance(
        /* [in] */ RClassID rclsid,
        /* [in] */ PRegime regime,
        /* [in] */ REIID riid,
        /* [out] */ PInterface* object)
    {
        return _CObject_CreateInstance(rclsid, regime, riid, object);
    }

    STATIC CARAPI CreateInstanceEx(
        /* [in] */ RClassID rclsid,
        /* [in] */ PRegime regime,
        /* [in] */ UInt32 cmq,
        /* [out] */ PMULTIQI results)
    {
        return _CObject_CreateInstanceEx(rclsid, regime, cmq, results);
    }

    STATIC CARAPI AcquireClassFactory(
        /* [in] */ RClassID rclsid,
        /* [in] */ PRegime regime,
        /* [in] */ REIID iid,
        /* [out] */ PInterface* object)
    {
        return _CObject_AcquireClassFactory(rclsid, regime, iid, object);
    }

    STATIC CARAPI ReflectModuleInfo(
        /* [in] */ PInterface object,
        /* [out] */ IModuleInfo** moduleInfo)
    {
        return _CObject_ReflectModuleInfo(object, moduleInfo);
    }

    STATIC CARAPI ReflectClassInfo(
        /* [in] */ PInterface object,
        /* [out] */ IClassInfo** classInfo)
    {
        return _CObject_ReflectClassInfo(object, classInfo);
    }

    STATIC CARAPI ReflectInterfaceInfo(
        /* [in] */ PInterface object,
        /* [out] */ IInterfaceInfo** interfaceInfo)
    {
        return _CObject_ReflectInterfaceInfo(object, interfaceInfo);
    }
};

class CCallbackParcel
{
public:
    STATIC CARAPI New(
        /* [out] */ IParcel** parcel)
    {
        return _CCallbackParcel_New(parcel);
    }
};

class CParcel
{
public:
    STATIC CARAPI New(
        /* [out] */ IParcel** parcel)
    {
        return _CParcel_New(parcel);
    }
};

_ELASTOS_NAMESPACE_END

#endif // __ELASYSAPI_H__
/** @} */
