VPATH	= $(ARCH):../$(ARCH):../../common/$(ARCH):
VPATH   += ../:../../tztest/el1/secure:../../tztest/el1:../../tztest

EL1_S_ELF		= el1_sec.elf
EL1_S_IMAGE 	= el1_sec.bin
EL1_S_LOAD		= el1_sec.lds
EL1_S_OBJS		= el1_init.o \
                  el1_exception.o \
                  el1.o \
                  el1_loader.o \
                  el1_sec.o \
                  mem_util.o \
				  builtins.o \
				  tztest.o \
				  tztest_el1.o \
				  tztest_el1_sec.o

libgcc := $(shell $(CC) $(CFLAGS) --print-libgcc-file-name)
FLATLIBS = ../../libcflat/libcflat.a $(libgcc) ../../libcflat/$(ARCH)/libeabi.a

-include .*.d

CFLAGS += -I$(ARCH) -I../$(ARCH) -I../ -I../$(ARCH)
CFLAGS += -I../../common/$(ARCH) -I../../common/
CFLAGS += -I../../tztest/el1 -I../../tztest

##################################################################

$(EL1_S_ELF): $(EL1_S_OBJS) $(EL1_S_LOAD)
	$(LD) -o $@ $(EL1_S_OBJS) $(FLATLIBS) --script=$(EL1_S_LOAD)

$(EL1_S_IMAGE): $(EL1_S_ELF)
	$(OBJCOPY) -O binary $< $@

$(EL1_S_LOAD): el1_sec.lds.S Makefile ../../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $(notdir $@) $<

all: $(EL1_S_IMAGE)

clean:
	$(RM) $(EL1_S_OBJS) $(EL1_S_LOAD) $(EL1_S_ELF) $(EL1_S_IMAGE) .*.d
