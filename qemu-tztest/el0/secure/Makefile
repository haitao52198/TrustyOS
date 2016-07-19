VPATH	= $(ARCH):../$(ARCH):../../common/$(ARCH):
VPATH   += ../:../../tztest/el0/secure:../../tztest/el0:../../tztest

EL0_S_ELF		= el0_sec.elf
EL0_S_IMAGE 	= el0_sec.bin
EL0_S_LOAD		= el0_sec.lds
EL0_S_OBJS		= el0_sec.o \
				  el0.o \
                  builtins.o \
				  tztest.o \
				  tztest_el0.o \
				  tztest_el0_sec.o

libgcc := $(shell $(CC) $(CFLAGS) --print-libgcc-file-name)
FLATLIBS = ../../libcflat/libcflat.a $(libgcc) ../../libcflat/$(ARCH)/libeabi.a

-include .*.d

CFLAGS += -I$(ARCH) -I../$(ARCH) -I../ -I../$(ARCH)
CFLAGS += -I../../common/$(ARCH) -I../../common/
CFLAGS += -I../../tztest/el0 -I../../tztest

##################################################################

$(EL0_S_ELF): $(EL0_S_OBJS) $(EL0_S_LOAD)
	$(LD) -o $@ $(EL0_S_OBJS) $(FLATLIBS) --script=$(EL0_S_LOAD)

$(EL0_S_IMAGE): $(EL0_S_ELF)
	$(OBJCOPY) -O binary $< $@

$(EL0_S_LOAD): el0_sec.lds.S Makefile ../../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $(notdir $@) $<

all: $(EL0_S_IMAGE)

clean:
	$(RM) $(EL0_S_OBJS) $(EL0_S_LOAD) $(EL0_S_ELF) $(EL0_S_IMAGE) .*.d
