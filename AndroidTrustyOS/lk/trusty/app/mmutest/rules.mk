LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/mmutest.c \
	$(LOCAL_DIR)/mmutest_$(ARCH).S \

include make/module.mk
