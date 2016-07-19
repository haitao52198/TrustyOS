VPATH	= $(ARCH):../$(ARCH):../../common/$(ARCH):
VPATH   += ../:../../tztest/el1/nonsecure:../../tztest/el1:../../tztest

EL1_NS_ELF		= el1_nsec.elf
EL1_NS_IMAGE 	= el1_nsec.bin
EL1_NS_LOAD		= el1_nsec.lds
EL1_NS_OBJS		= el1_init.o \
                  el1_exception.o \
                  el1.o \
                  el1_loader.o \
                  el1_nsec.o \
                  mem_util.o \
				  builtins.o \
				  tztest.o \
				  tztest_el1.o \
				  tztest_el1_nsec.o

libgcc := $(shell $(CC) $(CFLAGS) --print-libgcc-file-name)
FLATLIBS = ../../libcflat/libcflat.a $(libgcc) ../../libcflat/$(ARCH)/libeabi.a

-include .*.d

CFLAGS += -I$(ARCH) -I../$(ARCH) -I../ -I../$(ARCH)
CFLAGS += -I../../common/$(ARCH) -I../../common/
CFLAGS += -I../../tztest/el1 -I../../tztest

##################################################################

$(EL1_NS_ELF): $(EL1_NS_OBJS) $(EL1_NS_LOAD)
	$(LD) -o $@ $(EL1_NS_OBJS) $(FLATLIBS) --script=$(EL1_NS_LOAD)

$(EL1_NS_IMAGE): $(EL1_NS_ELF)
	$(OBJCOPY) -O binary $< $@

$(EL1_NS_LOAD): el1_nsec.lds.S Makefile ../../platform/$(PLAT)/
	$(CC) $(CFLAGS) -E -P -C -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $(notdir $@) $<

all: $(EL1_NS_IMAGE)

clean:
	$(RM) $(EL1_NS_OBJS) $(EL1_NS_LOAD) $(EL1_NS_ELF) $(EL1_NS_IMAGE) .*.d
