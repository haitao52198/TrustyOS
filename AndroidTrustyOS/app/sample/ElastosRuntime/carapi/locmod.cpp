//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================
#define __NO_LINKNODE_CONSTRUCTOR
#include <locmod.h>
#include <pthread.h>
#include <dlfcn.h>
#include <stdio.h>
#include <utils/Log.h>

#define ENABLE_DUMP_CLSID    0    // debug info switch
#if ENABLE_DUMP_CLSID
#define DUMP_CLSID(CLSID, info) \
    do { \
        ALOGD("> %s\n", info); \
        ALOGD("======== DUMP_CLSID ========\n"); \
        ALOGD("{%p, %p, %p, {%p, %p, %p, %p, %p, %p, %p, %p} }\n", \
                CLSID.mClsid.mData1, CLSID.mClsid.mData2, CLSID.mClsid.mData3, \
                CLSID.mClsid.mData4[0], CLSID.mClsid.mData4[1], \
                CLSID.mClsid.mData4[2], CLSID.mClsid.mData4[3], \
                CLSID.mClsid.mData4[4], CLSID.mClsid.mData4[5], \
                CLSID.mClsid.mData4[6], CLSID.mClsid.mData4[7]); \
        ALOGD("============================\n"); \
    } while(0);
#else
#define DUMP_CLSID(CLSID, info);
#endif

EXTERN_C CARAPI DllGetClassObject(REMuid, REIID, PInterface*);

_ELASTOS DLinkNode g_LocModList(&g_LocModList, &g_LocModList);
pthread_mutex_t g_LocModListLock;

CAR_INLINE _ELASTOS Boolean IsRuntimeUunm(const char* uunm)
{
    return !strcmp("Elastos.Runtime.eco", uunm);
}

ECode AcquireClassObjectFromLocalModule(
    /* [in] */ RClassID rclsid,
    /* [in] */ REIID riid,
    /* [out] */ PInterface* object)
{
    const char* uunm = rclsid.mUunm;
    PDLLGETCLASSOBJECT dllGetClassObjectFunc;

    assert(uunm);

    ECode ec = NOERROR;

    pthread_mutex_lock(&g_LocModListLock);
    LocalModule* localModule = (LocalModule *)(g_LocModList.mNext);
    while ((DLinkNode *)localModule != &g_LocModList) {
        if (IsEqualUunm(localModule->mUunm.string(), uunm)) {
            ec = (*localModule->mDllGetClassObjectFunc)(rclsid.mClsid,
                    EIID_IClassObject, object);
            localModule->mAskCount = 0;
            pthread_mutex_unlock(&g_LocModListLock);
            return ec;
        }
        localModule = (LocalModule *)localModule->mNext;
    }
    pthread_mutex_unlock(&g_LocModListLock);

    if (IsRuntimeUunm(uunm)) {
        return DllGetClassObject(rclsid.mClsid, EIID_IClassObject, object);
    }

    localModule = NULL;
    DUMP_CLSID(rclsid, uunm)
#ifdef _DEBUG
    Void* module = dlopen(uunm, RTLD_NOW);
#else
    Void* module = dlopen(uunm, RTLD_LAZY);
#endif
    if (NULL == module) {
        ec = E_FILE_NOT_FOUND;
        ALOGE("<%s, %d> dlopen '%s' failed.\n", __FILE__, __LINE__, uunm);
        ALOGE("error: %s\n", dlerror());
        goto ErrorExit;
    }

    dllGetClassObjectFunc = (PDLLGETCLASSOBJECT)dlsym(module, "DllGetClassObject");
    if (NULL == dllGetClassObjectFunc) {
        ec = E_DOES_NOT_EXIST;
        goto ErrorExit;
    }

    localModule = new LocalModule;
    if (!localModule) {
        ec = E_OUT_OF_MEMORY;
        goto ErrorExit;
    }

    localModule->mUunm = uunm;
    localModule->mIModule = module;
    localModule->mDllGetClassObjectFunc = dllGetClassObjectFunc;
    localModule->mAskCount = 0;
    localModule->mDllCanUnloadNowFunc = (PDLLCANUNLOADNOW)dlsym(module, "DllCanUnloadNow");

    ec = (*dllGetClassObjectFunc)(rclsid.mClsid, riid, object);

    if (FAILED(ec)) goto ErrorExit;

    pthread_mutex_lock(&g_LocModListLock);
    g_LocModList.InsertFirst(localModule);
    pthread_mutex_unlock(&g_LocModListLock);
    return ec;

ErrorExit:
    if (module) {
        dlclose(module);
    }
    if (localModule) {
        delete localModule;
    }
    return ec;
}

ELAPI_(_ELASTOS Boolean) _CModule_CanUnloadAllModules()
{
    LocalModule* localModule;

    pthread_mutex_lock(&g_LocModListLock);
    localModule = (LocalModule *)(g_LocModList.mNext);

    while ((DLinkNode *)localModule != &g_LocModList) {
        if (NULL != localModule->mDllCanUnloadNowFunc) {
            if (NOERROR != (*localModule->mDllCanUnloadNowFunc)()) {
                pthread_mutex_unlock(&g_LocModListLock);
                return FALSE;
            }
        }
        localModule = (LocalModule *)localModule->mNext;
    }

    pthread_mutex_unlock(&g_LocModListLock);
    return TRUE;
}

ELAPI _CProcess_FreeUnusedModule(
    /* [in] */ const String& moduleName)
{
    ECode ec = NOERROR;

    if (moduleName.IsNullOrEmpty()) return E_INVALID_ARGUMENT;

    pthread_mutex_lock(&g_LocModListLock);
    LocalModule* localModule = (LocalModule *)(g_LocModList.mNext);

    while (localModule != (LocalModule *)&g_LocModList) {
        LocalModule* next = (LocalModule *)(localModule->Next());
        assert(!localModule->mUunm.IsNullOrEmpty());

        Int32 index = localModule->mUunm.LastIndexOf(moduleName);
        if (index < 0 ||
                (index > 0 && localModule->mUunm.GetChar(index - 1) != '/')) {
            localModule = next;
            continue;
        }

        if (NULL == localModule->mDllCanUnloadNowFunc) {
            ec = E_NOT_CAR_COMPONENT;
        }
        else if (NOERROR == (*localModule->mDllCanUnloadNowFunc)()) {
            localModule->Detach();
            dlclose(localModule->mIModule);
            delete localModule;
            localModule = NULL;
            ec = NOERROR;
        }
        else {
            ec = E_COMPONENT_CANNOT_UNLOAD_NOW;
        }
        pthread_mutex_unlock(&g_LocModListLock);
        return ec;
    }

    pthread_mutex_unlock(&g_LocModListLock);

    return E_COMPONENT_NOT_LOADED;
}

ELAPI_(void) _CProcess_FreeUnusedModules(
    /* [in] */ Boolean immediate)
{
    pthread_mutex_lock(&g_LocModListLock);
    LocalModule* localModule = (LocalModule *)(g_LocModList.mNext);

    while (localModule != (LocalModule *)&g_LocModList) {
        LocalModule* next = (LocalModule *)(localModule->Next());
        if (NULL != localModule->mDllCanUnloadNowFunc) {
            if (NOERROR == (*localModule->mDllCanUnloadNowFunc)()) {
                ++localModule->mAskCount;
                /*
                * If DllCanUnloadNow() return TRUE more than or
                * equal to 3 successive times, then release the DLL.
                * If caller specify 'immediate' is TRUE, then release
                * the DLL immediately.
                */
                if (immediate || localModule->mAskCount > 2) {
                    localModule->Detach();
                    dlclose(localModule->mIModule);
                    delete localModule;
                    localModule = NULL;
                }
            }
            else
                localModule->mAskCount = 0;
        }
        localModule = next;
    }

    pthread_mutex_unlock(&g_LocModListLock);
}

void FreeCurProcComponents()
{
    pthread_mutex_lock(&g_LocModListLock);
    LocalModule* curModule = (LocalModule *)(g_LocModList.mNext);

    while (curModule != (LocalModule *)&g_LocModList) {
        LocalModule* next = (LocalModule *)(curModule->Next());
        ECode ecCur = S_FALSE;
        if (NULL != curModule->mDllCanUnloadNowFunc) {
            ecCur = (*curModule->mDllCanUnloadNowFunc)();
            if (NOERROR == ecCur) {
                curModule->Detach();
                dlclose(curModule->mIModule);
                delete curModule;
                curModule = NULL;
            }
        }

        if (S_FALSE != ecCur) {
            curModule = (LocalModule *)(g_LocModList.mNext);
        }
        else {
            curModule = next;
        }
    }

    pthread_mutex_unlock(&g_LocModListLock);
}
