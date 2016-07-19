OUTPUT_FORMAT(FORMAT)
OUTPUT_ARCH(ARCH)
TARGET(binary)

#include "memory.h"

SECTIONS
{
    . = EL3_FLASH_BASE;
    .init . : {
        _EL3_INIT_BASE = .;
        el3_init.o(.init);
        *(.init);
    }
    _EL3_INIT_SIZE = SIZEOF(.init);

    . = ALIGN(4K);
    _EL3_FLASH_TEXT = .;
    . = EL3_BASE_VA;
    .text . : AT(_EL3_FLASH_TEXT) {
        _EL3_TEXT_BASE = .;
        *(.text);
        *(.vectors);
    }
    _EL3_TEXT_SIZE = SIZEOF(.text);

    . = ALIGN(4k);
    .data . : {
        _EL3_DATA_BASE = .;
        _EL3_FLASH_DATA = LOADADDR(.data);
        *(.*data);
        *(.*bss);
    }
    _EL3_DATA_SIZE = SIZEOF(.data);
}
