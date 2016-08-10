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
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/../Runtime/Core/inc \
	$(LOCAL_DIR)/../Runtime/Library/inc/eltypes \
	$(LOCAL_DIR)/../Runtime/Library/inc/car \
	$(LOCAL_DIR)/../Runtime/Library/inc/elasys \
	$(LOCAL_DIR)/../Runtime/Library/inc/clsmodule \
	$(LOCAL_DIR)/../../../../lib/lib/libstdc++-trusty/include \
	$(LOCAL_DIR)/../rdk/inc \
	$(LOCAL_DIR)/../rdk/PortingLayer \

#MODULE_CPPFLAGS := -std=c++11

MODULE_SRCS += $(LOCAL_DIR)/carapi.cpp
MODULE_SRCS += $(LOCAL_DIR)/locmod.cpp
MODULE_SRCS += $(LOCAL_DIR)/todo.cpp



MODULE_DEPS += \
	app/trusty \
	lib/libc-trusty \
	lib/libstdc++-trusty \

include make/module.mk
