OUTPUT_FORMAT(FORMAT)
OUTPUT_ARCH(ARCH)
TARGET(binary)
ENTRY(main)

#include "memory.h"

SECTIONS
{
    . = EL0_NS_BASE_VA;
    .text . : {
        _EL0_NS_TEXT_BASE = .;
        *(.text);
        _EL0_NS_TEXT_SIZE = SIZEOF(.text);
    }

    . = ALIGN(4k);
    .data . : {
        _EL0_NS_DATA_BASE = .;
        *(.*data);
        *(.*bss);
        _EL0_NS_DATA_SIZE = SIZEOF(.data);
    }
}
