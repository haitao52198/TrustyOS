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

KEYMASTER_ROOT := $(LOCAL_DIR)/../../system/keymaster

MODULE_SRCS += \
	$(KEYMASTER_ROOT)/aes_key.cpp \
	$(KEYMASTER_ROOT)/aes_operation.cpp \
	$(KEYMASTER_ROOT)/android_keymaster.cpp \
	$(KEYMASTER_ROOT)/android_keymaster_messages.cpp \
	$(KEYMASTER_ROOT)/android_keymaster_utils.cpp \
	$(KEYMASTER_ROOT)/asymmetric_key.cpp \
	$(KEYMASTER_ROOT)/asymmetric_key_factory.cpp \
	$(KEYMASTER_ROOT)/attestation_record.cpp \
	$(KEYMASTER_ROOT)/auth_encrypted_key_blob.cpp \
	$(KEYMASTER_ROOT)/authorization_set.cpp \
	$(KEYMASTER_ROOT)/ec_key.cpp \
	$(KEYMASTER_ROOT)/ec_key_factory.cpp \
	$(KEYMASTER_ROOT)/ecdsa_operation.cpp \
	$(KEYMASTER_ROOT)/hmac_key.cpp \
	$(KEYMASTER_ROOT)/hmac_operation.cpp \
	$(KEYMASTER_ROOT)/key.cpp \
	$(KEYMASTER_ROOT)/keymaster_enforcement.cpp \
	$(KEYMASTER_ROOT)/logger.cpp \
	$(KEYMASTER_ROOT)/ocb.c \
	$(KEYMASTER_ROOT)/ocb_utils.cpp \
	$(KEYMASTER_ROOT)/openssl_err.cpp \
	$(KEYMASTER_ROOT)/openssl_utils.cpp \
	$(KEYMASTER_ROOT)/operation.cpp \
	$(KEYMASTER_ROOT)/operation_table.cpp \
	$(KEYMASTER_ROOT)/rsa_key.cpp \
	$(KEYMASTER_ROOT)/rsa_key_factory.cpp \
	$(KEYMASTER_ROOT)/rsa_operation.cpp \
	$(KEYMASTER_ROOT)/serializable.cpp \
	$(KEYMASTER_ROOT)/symmetric_key.cpp \
	$(LOCAL_DIR)/trusty_keymaster_context.cpp \
	$(LOCAL_DIR)/trusty_keymaster_enforcement.cpp \
	$(LOCAL_DIR)/manifest.c

MODULE_INCLUDES := \
	$(KEYMASTER_ROOT)/include \
	$(KEYMASTER_ROOT) \
	$(LOCAL_DIR)

MODULE_CPPFLAGS := -std=c++11

IPC := ipc

MODULE_DEPS += \
	app/trusty \
	lib/libc-trusty \
	lib/libstdc++-trusty \
	lib/rng \
	lib/hwkey

include $(LOCAL_DIR)/$(IPC)/rules.mk

include make/module.mk
