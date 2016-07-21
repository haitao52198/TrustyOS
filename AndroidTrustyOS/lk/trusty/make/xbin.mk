#
# Copyright (c) 2014-2015, Google, Inc. All rights reserved
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

# The following set of variables must can be passed to xbin.mk:
#
#     XBIN_NAME - an output file name (without extention)
#     XBIN_BUILDDIR - build directory
#     XBIN_TOP_MODULE - top module to compile
#     XBIN_ARCH - architecture to compile for
#     XBIN_LINKER_SCRIPT - linker script
#     XBIN_TYPE - optional executable type
#     XBIN_LDFLAGS - extra linker flags (optional)
#

# check if all required variables are set or provide default

ifeq ($(XBIN_NAME), )
$(error XBIN_NAME must be specified)
endif

ifeq ($(XBIN_BUILDDIR), )
$(error XBIN_BUILDDIR must be specified)
endif

ifeq ($(XBIN_TOP_MODULE), )
$(error XBIN_TOP_MODULE must be specified)
endif

ifeq ($(XBIN_LINKER_SCRIPT), )
$(error XBIN_LINKER_SCRIPT must be specified)
endif

ifeq ($(XBIN_ARCH), )
$(error XBIN_ARCH must be specified)
endif

ifeq ($(XBIN_TYPE), )
XBIN_TYPE := XBIN
endif

# save global variables
SAVED_ARCH := $(ARCH)
SAVED_GLOBAL_OPTFLAGS := $(GLOBAL_OPTFLAGS)
SAVED_GLOBAL_COMPILEFLAGS := $(GLOBAL_COMPILEFLAGS)
SAVED_GLOBAL_CFLAGS := $(GLOBAL_CFLAGS)
SAVED_GLOBAL_CPPFLAGS := $(GLOBAL_CPPFLAGS)
SAVED_GLOBAL_ASMFLAGS := $(GLOBAL_ASMFLAGS)
SAVED_GLOBAL_INCLUDES := $(GLOBAL_INCLUDES)
SAVED_GLOBAL_DEFINES := $(GLOBAL_DEFINES)

SAVED_BUILDDIR := $(BUILDDIR)
SAVED_ALLMODULES := $(ALLMODULES)
SAVED_ALLMODULE_OBJS := $(ALLMODULE_OBJS)

# reset lk.bin variables
ARCH := $(XBIN_ARCH)
BUILDDIR := $(XBIN_BUILDDIR)
ALLMODULES :=
ALLMODULE_OBJS :=

# Override tools
include arch/$(ARCH)/toolchain.mk

XBIN_TOOLCHAIN_PREFIX := $(ARCH_$(ARCH)_TOOLCHAIN_PREFIX)
XBIN_CC := $(CCACHE) $(XBIN_TOOLCHAIN_PREFIX)gcc
XBIN_LD := $(XBIN_TOOLCHAIN_PREFIX)ld
XBIN_OBJCOPY := $(XBIN_TOOLCHAIN_PREFIX)objcopy
XBIN_OBJDUMP := $(XBIN_TOOLCHAIN_PREFIX)objdump
XBIN_STRIP := $(XBIN_TOOLCHAIN_PREFIX)strip
XBIN_LIBGCC := $(shell $(XBIN_TOOLCHAIN_PREFIX)gcc $(GLOBAL_COMPILEFLAGS) $(THUMBCFLAGS) -print-libgcc-file-name)

$(info XBIN_TOOLCHAIN_PREFIX = $(XBIN_TOOLCHAIN_PREFIX))
$(info XBIN_LIBGCC = $(XBIN_LIBGCC))

GLOBAL_CFLAGS := $(GLOBAL_CFLAGS)
GLOBAL_INCLUDES :=
GLOBAL_DEFINES := $(XBIN_TYPE)=1

# Include XBIN top module and handle all it's dependencies
include $(addsuffix /rules.mk,$(XBIN_TOP_MODULE))
include make/recurse.mk

# Add all XBIN specific defines
GLOBAL_DEFINES += \
	$(addsuffix =1,$(addprefix $(XBIN_TYPE)_WITH_,$(ALLMODULES)))

# XBIN build rules
XBIN_BIN := $(BUILDDIR)/$(XBIN_NAME).bin
XBIN_ELF := $(BUILDDIR)/$(XBIN_NAME).elf
XBIN_SYMS_ELF := $(BUILDDIR)/$(XBIN_NAME).syms.elf
XBIN_ALL_OBJS := $(ALLMODULE_OBJS)
XBIN_CONFIGHEADER := $(BUILDDIR)/config.h

# Set appropriate globals for all targets under $(BUILDDIR)
$(BUILDDIR)/%: CC := $(XBIN_CC)
$(BUILDDIR)/%: LD := $(XBIN_LD)
$(BUILDDIR)/%.o: GLOBAL_OPTFLAGS := $(GLOBAL_OPTFLAGS)
$(BUILDDIR)/%.o: GLOBAL_COMPILEFLAGS := $(GLOBAL_COMPILEFLAGS) -include $(XBIN_CONFIGHEADER)
$(BUILDDIR)/%.o: GLOBAL_CFLAGS   := $(GLOBAL_CFLAGS)
$(BUILDDIR)/%.o: GLOBAL_CPPFLAGS := $(GLOBAL_CPPFLAGS)
$(BUILDDIR)/%.o: GLOBAL_ASMFLAGS := $(GLOBAL_ASMFLAGS)
$(BUILDDIR)/%.o: GLOBAL_INCLUDES := $(addprefix -I,$(GLOBAL_INCLUDES)) $(SAVED_GLOBAL_INCLUDES)
$(BUILDDIR)/%.o: ARCH_COMPILEFLAGS := $(ARCH_$(ARCH)_COMPILEFLAGS)
$(BUILDDIR)/%.o: ARCH_CFLAGS := $(ARCH_$(ARCH)_CFLAGS)
$(BUILDDIR)/%.o: ARCH_CPPFLAGS := $(ARCH_$(ARCH)_CPPFLAGS)
$(BUILDDIR)/%.o: ARCH_ASMFLAGS := $(ARCH_$(ARCH)_ASMFLAGS)

# generate XBIN specific config.h
$(XBIN_CONFIGHEADER): GLOBAL_DEFINES := $(GLOBAL_DEFINES)
$(XBIN_CONFIGHEADER):
	@$(call MAKECONFIGHEADER,$@,GLOBAL_DEFINES)

# add it to global dependency list
GENERATED += $(XBIN_CONFIGHEADER)
GLOBAL_SRCDEPS += $(XBIN_CONFIGHEADER)

# Link XBIN elf
$(XBIN_SYMS_ELF): XBIN_LD := $(XBIN_LD)
$(XBIN_SYMS_ELF): XBIN_LIBGCC := $(XBIN_LIBGCC)
$(XBIN_SYMS_ELF): XBIN_LDFLAGS := $(XBIN_LDFLAGS)
$(XBIN_SYMS_ELF): XBIN_ALL_OBJS := $(XBIN_ALL_OBJS)
$(XBIN_SYMS_ELF): XBIN_LINKER_SCRIPT := $(XBIN_LINKER_SCRIPT)
$(XBIN_SYMS_ELF): $(XBIN_ALL_OBJS) $(XBIN_LINKER_SCRIPT) $(XBIN_CONFIGHEADER)
	@$(MKDIR)
	@echo linking $@
	$(NOECHO)$(XBIN_LD) $(XBIN_LDFLAGS) -T $(XBIN_LINKER_SCRIPT) --start-group $(XBIN_ALL_OBJS) --end-group $(XBIN_LIBGCC) -o $@

# And strip it
$(XBIN_ELF): XBIN_STRIP := $(XBIN_STRIP)
$(XBIN_ELF): $(XBIN_SYMS_ELF)
	@$(MKDIR)
	@echo stripping $<
	$(NOECHO)$(XBIN_STRIP) -s $< -o $@

# build XBIN binary
$(XBIN_BIN): XBIN_OBJCOPY := $(XBIN_OBJCOPY)
$(XBIN_BIN): $(XBIN_ELF)
	@echo generating image: $@
	$(NOECHO)$(XBIN_OBJCOPY) -O binary $< $@

# Also generate listings
all:: $(XBIN_ELF).lst $(XBIN_ELF).debug.lst

$(XBIN_ELF).lst: XBIN_OBJDUMP := $(XBIN_OBJDUMP)
$(XBIN_ELF).lst: $(XBIN_SYMS_ELF)
	@echo generating listing: $@
	$(NOECHO)$(XBIN_OBJDUMP) -d $< | $(CPPFILT) > $@

$(XBIN_ELF).debug.lst: XBIN_OBJDUMP := $(XBIN_OBJDUMP)
$(XBIN_ELF).debug.lst: $(XBIN_SYMS_ELF)
	@echo generating listing: $@
	$(NOECHO)$(XBIN_OBJDUMP) -S $< | $(CPPFILT) > $@

# restore LK variables
GLOBAL_OPTFLAGS := $(SAVED_GLOBAL_OPTFLAGS)
GLOBAL_COMPILEFLAGS := $(SAVED_GLOBAL_COMPILEFLAGS)
GLOBAL_CFLAGS   := $(SAVED_GLOBAL_CFLAGS)
GLOBAL_CPPFLAGS := $(SAVED_GLOBAL_CPPFLAGS)
GLOBAL_ASMFLAGS := $(SAVED_GLOBAL_ASMFLAGS)
GLOBAL_INCLUDES := $(SAVED_GLOBAL_INCLUDES)
GLOBAL_DEFINES  := $(SAVED_GLOBAL_DEFINES)

ARCH := $(SAVED_ARCH)
BUILDDIR := $(SAVED_BUILDDIR)
ALLMODULES := $(SAVED_ALLMODULES)
ALLMODULE_OBJS := $(SAVED_ALLMODULE_OBJS)

# Reset local variables
XBIN_NAME :=
XBIN_TYPE :=
XBIN_ARCH :=
XBIN_TOP_MODULE :=
XBIN_BUILDDIR :=

XBIN_BIN :=
XBIN_ELF :=
XBIN_SYMS_ELF :=
XBIN_ALL_OBJS :=
XBIN_CONFIGHEADER :=
XBIN_LINKER_SCRIPT :=

XBIN_TOOLCHAIN_PREFIX :=
XBIN_CC :=
XBIN_LD :=
XBIN_OBJCOPY :=
XBIN_STRIP :=

XBIN_LDFLAGS :=
