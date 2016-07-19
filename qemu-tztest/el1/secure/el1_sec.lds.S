OUTPUT_FORMAT(FORMAT)
OUTPUT_ARCH(ARCH)
TARGET(binary)

#include "memory.h"

SECTIONS
{
    . = EL1_S_FLASH_BASE;
    .init . : {
        _EL1_S_INIT_BASE = .;
        el1_init.o(.init);
        *(.init);
    }
    _EL1_S_INIT_SIZE = SIZEOF(.init);

    . = ALIGN(4K);
    _EL1_S_FLASH_TEXT = .;
    . = EL1_S_BASE_VA;
    .text . : AT(_EL1_S_FLASH_TEXT) {
        _EL1_S_TEXT_BASE = .;
        *(.text);
        *(.vectors);
    }
    _EL1_S_TEXT_SIZE = SIZEOF(.text);

    . = ALIGN(4k);
    .data . : {
        _EL1_S_DATA_BASE = .;
        _EL1_S_FLASH_DATA = LOADADDR(.data);
        *(.*data);
        *(.*bss);
    }
    _EL1_S_DATA_SIZE = SIZEOF(.data);
}
