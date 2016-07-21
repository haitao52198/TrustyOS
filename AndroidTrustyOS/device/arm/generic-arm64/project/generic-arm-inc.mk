# Copyright (C) 2015 The Android Open Source Project
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

DEBUG ?= 2
SMP_MAX_CPUS ?= 8
SMP_CPU_CLUSTER_SHIFT ?= 2

TARGET := generic-arm64

ifeq (false,$(call TOBOOL,$(KERNEL_32BIT)))

# Arm64 address space configuration
KERNEL_ASPACE_BASE := 0xffffffffe0000000
KERNEL_ASPACE_SIZE := 0x0000000020000000
KERNEL_BASE        := 0xffffffffe0000000

GLOBAL_DEFINES += MMU_USER_SIZE_SHIFT=25 # 32 MB user-space address space

else

KERNEL_BASE        := 0xe0000000

endif

# select timer
ifeq (true,$(call TOBOOL,$(KERNEL_32BIT)))
# 32 bit Secure EL1 with a 64 bit EL3 gets the non-secure physical timer
GLOBAL_DEFINES += TIMER_ARM_GENERIC_SELECTED=CNTP
else
GLOBAL_DEFINES += TIMER_ARM_GENERIC_SELECTED=CNTPS
endif

#
# GLOBAL definitions
#

# requires linker GC
WITH_LINKER_GC := 1

# Need support for Non-secure memory mapping
WITH_NS_MAPPING := true

# do not relocate kernel in physical memory
GLOBAL_DEFINES += WITH_NO_PHYS_RELOCATION=1

# limit heap grows
GLOBAL_DEFINES += HEAP_GROW_SIZE=8192

# limit physical memory to 38 bit to prevert tt_trampiline from getting larger than arm64_kernel_translation_table
GLOBAL_DEFINES += MMU_IDENT_SIZE_SHIFT=38

#
# Modules to be compiled into lk.bin
#
MODULES += \
	lib/sm \
	lib/trusty \
	lib/memlog \

TRUSTY_USER_ARCH := arm

#
# user tasks to be compiled into lk.bin
#

# prebuilt
TRUSTY_PREBUILT_USER_TASKS :=

# compiled from source
TRUSTY_ALL_USER_TASKS := \
	keymaster \
	gatekeeper \
	storage \

# This project requires trusty IPC
WITH_TRUSTY_IPC := true

EXTRA_BUILDRULES += app/trusty/user-tasks.mk
