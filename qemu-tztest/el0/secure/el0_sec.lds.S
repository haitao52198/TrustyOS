OUTPUT_FORMAT(FORMAT)
OUTPUT_ARCH(ARCH)
TARGET(binary)
ENTRY(main)

#include "memory.h"

SECTIONS
{
    . = EL0_S_BASE_VA;
    .text . : AT(0x0) {
        _EL0_S_TEXT_BASE = .;
        *(.text);
        _EL0_S_TEXT_SIZE = SIZEOF(.text);
    }

    . = ALIGN(4k);
    .data . : {
        _EL0_S_DATA_BASE = .;
        *(.*data);
        *(.*bss);
        _EL0_S_DATA_SIZE = SIZEOF(.data);
    }
}
