ifeq ($(wildcard config.mk),)
$(error run ./configure first. See ./configure -h)
endif

# Pick up configuration definitions
include config.mk
# Pick up architecture specific build rules
include $(ARCH).mk


.PHONY: clean distclean

# Usage: OP_CFLAGS+=$(call cc-option, -falign-functions=0, -malign-functions=0)

cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
              > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

autodepend-flags = -MMD -MF $(dir $*).$(notdir $*).d

CFLAGS += $(autodepend-flags)
CFLAGS += -std=gnu99 -DASM
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -Wextra -Werror -Wall
CFLAGS += -g -O0
CFLAGS += -I. -I/include -I$(ARCH)/include
CFLAGS += -I../libcflat/include
CFLAGS += -I../platform/$(PLAT)
CFLAGS += -I/usr/$(CROSS)/include
CFLAGS += -I../../platform/$(PLAT) -I../../libcflat/include
CFLAGS += -I$(ARCH) -I../common/$(ARCH) -I../common
CFLAGS += $(call cc-option, -fomit-frame-pointer, "")
CFLAGS += $(call cc-option, -fno-stack-protector, "")
CFLAGS += $(call cc-option, -fno-stack-protector-all, "")

export CFLAGS

BIOS_IMAGE 		= tztest.img
LIBCFLAT		= libcflat.a
EL3_IMAGE		= el3/el3.bin
EL1_S_IMAGE		= el1/secure/el1_sec.bin
EL1_NS_IMAGE	= el1/nonsecure/el1_nsec.bin
EL0_S_IMAGE 	= el0/secure/el0_sec.elf
EL0_NS_IMAGE 	= el0/nonsecure/el0_nsec.elf

-include .*.d

$(BIOS_IMAGE): $(LIBCFLAT) $(EL3_IMAGE) $(EL1_S_IMAGE) $(EL1_NS_IMAGE) \
			   $(EL0_S_IMAGE) $(EL0_NS_IMAGE)
	dd if=$(EL3_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=65536 if=$(EL1_S_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=131072 if=$(EL1_NS_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=196608 if=$(EL0_NS_IMAGE) of=$@ 2> /dev/null
	dd oflag=seek_bytes seek=327680 if=$(EL0_S_IMAGE) of=$@ 2> /dev/null
	chmod +x $(BIOS_IMAGE)

$(LIBCFLAT):
	$(MAKE) -C libcflat all

$(EL3_IMAGE):
	$(MAKE) -C el3 all

$(EL1_S_IMAGE):
	$(MAKE) -C el1/secure all

$(EL1_NS_IMAGE):
	$(MAKE) -C el1/nonsecure all

$(EL0_S_IMAGE):
	$(MAKE) -C el0/secure all

$(EL0_NS_IMAGE):
	$(MAKE) -C el0/nonsecure all

clean:
	$(MAKE) -C libcflat clean
	$(MAKE) -C el3 clean
	$(MAKE) -C el1/secure clean
	$(MAKE) -C el1/nonsecure clean
	$(MAKE) -C el0/secure clean
	$(MAKE) -C el0/nonsecure clean
	$(RM) $(BIOS_IMAGE) .*.d

distclean: clean
	$(RM) config.mak cscope.*
