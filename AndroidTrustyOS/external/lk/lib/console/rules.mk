LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/console.c

EXTRA_LINKER_SCRIPTS += $(LOCAL_DIR)/console.ld

include make/module.mk
