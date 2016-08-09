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
	$(LOCAL_DIR)/../include \
	$(LOCAL_DIR)/../Runtime/Core/inc \
	$(LOCAL_DIR)/../Runtime/Library/inc/eltypes \
	$(LOCAL_DIR)/../Runtime/Library/inc/car \
	$(LOCAL_DIR)/../Runtime/Library/inc/elasys \


#MODULE_CPPFLAGS := -std=c++11

MODULE_SRCS += $(LOCAL_DIR)/CClsModule.cpp
MODULE_SRCS += CObjInfoList.cpp
MODULE_SRCS += CEntryList.cpp
MODULE_SRCS += refutil.cpp
MODULE_SRCS += reflection.cpp
MODULE_SRCS += CArgumentList.cpp
MODULE_SRCS += CCallbackArgumentList.cpp
MODULE_SRCS += CCallbackMethodInfo.cpp
MODULE_SRCS += CCarArrayInfo.cpp
MODULE_SRCS += CVariableOfCarArray.cpp
MODULE_SRCS += CClassInfo.cpp
MODULE_SRCS += CConstantInfo.cpp
MODULE_SRCS += CConstructorInfo.cpp
MODULE_SRCS += CCppVectorInfo.cpp
MODULE_SRCS += CDelegateProxy.cpp
MODULE_SRCS += CEnumInfo.cpp
MODULE_SRCS += CEnumItemInfo.cpp
MODULE_SRCS += CFieldInfo.cpp
MODULE_SRCS += CInterfaceInfo.cpp
MODULE_SRCS += CIntrinsicInfo.cpp
MODULE_SRCS += CLocalPtrInfo.cpp
MODULE_SRCS += CLocalTypeInfo.cpp
MODULE_SRCS += CMethodInfo.cpp
MODULE_SRCS += CModuleInfo.cpp
MODULE_SRCS += CParamInfo.cpp
MODULE_SRCS += CStructInfo.cpp
MODULE_SRCS += CVariableOfStruct.cpp
MODULE_SRCS += CVariableOfCppVector.cpp
MODULE_SRCS += CTypeAliasInfo.cpp


MODULE_DEPS += \
	app/trusty \
	lib/libc-trusty \

include make/module.mk
