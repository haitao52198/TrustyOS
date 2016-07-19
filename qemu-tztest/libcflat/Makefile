# libcflat paths
LIBCFLAT = libcflat.a
LIBCFLAT_OBJS = printf.o \
            	string.o \
                $(ARCH)/io.o \
            	$(ARCH)/misc.o

# libeabi paths
LIBEABI = $(ARCH)/libeabi.a
LIBEABI_OBJS = $(ARCH)/eabi_compat.o

-include .*.d $(ARCH)/.*.d

$(LIBCFLAT): $(LIBCFLAT_OBJS)
	$(AR) rcs $@ $^

$(LIBEABI): $(LIBEABI_OBJS)
	$(AR) rcs $@ $^

%.o: %.S
	$(CC) $(CFLAGS) -c -nostdlib -o $@ $<

all: $(LIBCFLAT) $(LIBEABI)

libeabi_clean:
	$(RM) $(LIBEABI) $(LIBEABI_OBJS)

libcflat_clean:
	$(RM) $(LIBCFLAT) $(LIBCFLAT_OBJS)

clean: libcflat_clean libeabi_clean
	$(RM) .*.d $(ARCH)/.*.d
