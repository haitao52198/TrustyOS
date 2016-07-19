#ifndef _PLATFORM_H
#define _PLATFORM_H

/* NORFLASH0 on Vexpress aseries (a15) is mapped from 0x08000000 to 0x0A000000
 * which is 32MB.  It is also aliased to 0x0 (to 0x2000000).
 * It is acceptable to have the text here as it is RO.
 */
#define FLASH_BASE 0x00000000
#define FLASH_SIZE (64 * 1024 * 1024)

#define RAM_BASE 0x80000000
#define RAM_SIZE (1*1024*1024*1024)

#define UART0_BASE 0x1c090000

/* Taken from Linux vexpress support */
#define SYSREG_BASE 0x1c010000
#define SYSREG_CFGCTRL 0xa4
#define SYS_CFGCTRL_FUNC(n) (((n) & 0x3f) << 20)
#define SYS_CFGFUNC_RESET 5
#define SYS_CFGFUNC_SHUTDOWN 8
#define SYS_CFGCTRL_START       (1 << 31)
#define SYS_CFGCTRL_WRITE       (1 << 30)
#define SYS_SHUTDOWN    (SYS_CFGCTRL_FUNC(SYS_CFGFUNC_SHUTDOWN) | \
                         SYS_CFGCTRL_START | SYS_CFGCTRL_WRITE)
#define SYS_RESET       (SYS_CFGCTRL_FUNC(SYS_CFGFUNC_SHUTDOWN) | \
                         SYS_CFGCTRL_START | SYS_CFGCTRL_WRITE)

#endif
