# Copyright (C) 2014-2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_INCLUDES += \
	$(LOCAL_DIR)/../../trusty \
	$(LOCAL_DIR)/../../trusty/include \
	$(LOCAL_DIR)/../Runtime/Core/inc \
	$(LOCAL_DIR)/../Runtime/Library/inc/eltypes \
	$(LOCAL_DIR)/../Runtime/Library/inc/car \
	$(LOCAL_DIR)/../Runtime/Library/inc/elasys \
	$(LOCAL_DIR)/../Runtime/Library/inc/clsmodule \
	$(LOCAL_DIR)/../Runtime/Library/syscar \
	$(LOCAL_DIR)/../../../../../lib/lib/libstdc++-trusty/include \
	$(LOCAL_DIR)/../rdk/inc \
	$(LOCAL_DIR)/../rdk/PortingLayer \
	$(LOCAL_DIR)/../ElRuntimeAPI \

#MODULE_CPPFLAGS := -std=c++11

MODULE_SRCS += $(LOCAL_DIR)/CClsModule.cpp
MODULE_SRCS += $(LOCAL_DIR)/CObjInfoList.cpp
MODULE_SRCS += $(LOCAL_DIR)/CEntryList.cpp
MODULE_SRCS += $(LOCAL_DIR)/refutil.cpp
MODULE_SRCS += $(LOCAL_DIR)/reflection.cpp
MODULE_SRCS += $(LOCAL_DIR)/CArgumentList.cpp
#MODULE_SRCS += $(LOCAL_DIR)/CCallbackArgumentList.cpp
#MODULE_SRCS += $(LOCAL_DIR)/CCallbackMethodInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CCarArrayInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CVariableOfCarArray.cpp
MODULE_SRCS += $(LOCAL_DIR)/CClassInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CConstantInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CConstructorInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CCppVectorInfo.cpp
#MODULE_SRCS += $(LOCAL_DIR)/CDelegateProxy.cpp
MODULE_SRCS += $(LOCAL_DIR)/CEnumInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CEnumItemInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CFieldInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CInterfaceInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CIntrinsicInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CLocalPtrInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CLocalTypeInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CMethodInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CModuleInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CParamInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CStructInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/CVariableOfStruct.cpp
MODULE_SRCS += $(LOCAL_DIR)/CVariableOfCppVector.cpp
MODULE_SRCS += $(LOCAL_DIR)/CTypeAliasInfo.cpp
MODULE_SRCS += $(LOCAL_DIR)/invoke_gnuc.S
MODULE_SRCS += $(LOCAL_DIR)/manifest.c
MODULE_SRCS += $(LOCAL_DIR)/ref_app.cpp
MODULE_SRCS += $(LOCAL_DIR)/pseudo-dlfcn.c
MODULE_SRCS += $(LOCAL_DIR)/pseudo-misc.cpp
MODULE_SRCS += $(LOCAL_DIR)/../Runtime/Library/elasys/sysiids.cpp
MODULE_SRCS += $(LOCAL_DIR)/../Runtime/Library/eltypes/elstring/elstring.cpp
MODULE_SRCS += $(LOCAL_DIR)/../Runtime/Library/eltypes/elstring/elsharedbuf.cpp
MODULE_SRCS += $(LOCAL_DIR)/../Runtime/Library/eltypes/elstringapi.cpp
MODULE_SRCS += $(LOCAL_DIR)/../Runtime/Library/elasys/elaatomics.cpp
MODULE_SRCS += $(LOCAL_DIR)/../Runtime/Library/eltypes/elquintet.cpp

MODULE_SRCS += $(LOCAL_DIR)/../ElRuntimeAPI/xni.cpp
MODULE_SRCS += $(LOCAL_DIR)/../ElRuntimeAPI/implClassFactory.cpp

#MODULE_SRCS += $(LOCAL_DIR)/../Runtime/Library/clsmodule/reloc.cpp
#MODULE_SRCS += $(LOCAL_DIR)/../Runtime/Library/clsmodule/clserr.cpp
#MODULE_SRCS += $(LOCAL_DIR)/../Runtime/Library/clsmodule/comprcls.cpp
#MODULE_SRCS += $(LOCAL_DIR)/../Runtime/Library/clsmodule/datatype.cpp

#MODULE_DEPS += \
#	app/trusty \
#	lib/libc-trusty \
#	lib/libstdc++-trusty \

include make/module.mk
