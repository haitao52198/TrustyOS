#include <stdint.h>
#include <stddef.h>
#include "platform.h"
#include "memory.h"
#include "exception.h"
#include "vmsa.h"
#include "cpu.h"
#include "el3.h"
#include "string.h"
#include "libcflat.h"
#include "smc.h"
#include "svc.h"
#include "el3_monitor.h"
#include "builtins.h"
#include "syscntl.h"
#include "mem_util.h"
#include "state.h"
#include "debug.h"
#include "tztest.h"

#if DEBUG
const char *smc_op_name[] = {
    [SMC_OP_NOOP] = "SMC_OP_NOOP",
    [SMC_OP_YIELD] = "SMC_OP_YIELD",
    [SMC_OP_EXIT] = "SMC_OP_EXIT",
    [SMC_OP_MAP] = "SMC_OP_MAP",
    [SMC_OP_GET_REG] = "SMC_OP_GET_REG",
    [SMC_OP_SET_REG] = "SMC_OP_SET_REG",
    [SMC_OP_TEST] = "SMC_OP_TEST",
    [SMC_OP_DISPATCH] = "SMC_OP_DISPATCH",
};
#endif

char *sec_state_str[] = {"secure", "nonsecure"};
const uint32_t exception_level = EL3;
uint32_t secure_state = SECURE;

uintptr_t EL3_TEXT_BASE = (uintptr_t)&_EL3_TEXT_BASE;
uintptr_t EL3_DATA_BASE = (uintptr_t)&_EL3_DATA_BASE;
uintptr_t EL3_TEXT_SIZE = (uintptr_t)&_EL3_TEXT_SIZE;
uintptr_t EL3_DATA_SIZE = (uintptr_t)&_EL3_DATA_SIZE;

state_buf sec_state;
state_buf nsec_state;

sys_control_t *syscntl;
smc_op_desc_t *smc_interop_buf;

uintptr_t mem_pgtbl_base = EL3_PGTBL_BASE;
uintptr_t mem_next_pa = 0;
uintptr_t mem_next_l1_page = 0;
uintptr_t mem_heap_pool = EL3_VA_HEAP_BASE;

void el3_shutdown() {
    uintptr_t *sysreg_cfgctrl = (uintptr_t *)(SYSREG_BASE + SYSREG_CFGCTRL);

    DEBUG_MSG("Test complete");

    *sysreg_cfgctrl = SYS_SHUTDOWN;

    asm volatile("wfi\n");  /* Shutdown does not work on all machines */
}

void el3_alloc_mem(op_alloc_mem_t *alloc)
{
    alloc->addr = mem_heap_allocate(alloc->len);
}

uint32_t el3_map_mem(op_map_mem_t *map)
{
    if ((map->type & OP_MAP_EL3) == OP_MAP_EL3) {
        mem_map_pa((uintptr_t)map->va, (uintptr_t)map->pa, 0, PTE_PRIV_RW);
        DEBUG_MSG("Mapped VA:0x%lx to PA:0x%lx\n", map->va, map->pa);
    }

    if ((map->type & OP_MAP_SEC_EL1) == OP_MAP_SEC_EL1) {
        map->type &= ~OP_MAP_EL3;
        DEBUG_MSG("Initiating SVC_OP_MAP\n");
        return SVC_OP_MAP;
    }

    return 0;
}

int el3_handle_smc(uintptr_t op, smc_op_desc_t *desc,
                   uintptr_t __attribute((unused))far, uintptr_t elr)
{
    op_test_t *test = (op_test_t*)desc;
    uint32_t ret = 0;

    DEBUG_MSG("Took an smc(%s) - desc = %p\n", smc_op_name[op], desc);
    switch (op) {
    case SMC_OP_YIELD:
        ret = SVC_OP_YIELD;
        break;
    case SMC_OP_MAP:
        ret = el3_map_mem((op_map_mem_t *)desc);
        break;
    case SMC_OP_NOOP:
        break;
    case SMC_OP_EXIT:
        el3_shutdown();
        break;
    case SMC_OP_DISPATCH:
        if (desc->disp.el == exception_level &&
            desc->disp.state == (READ_SCR() & SCR_NS))  {
            run_test(desc->disp.fid, desc->disp.el,
                     desc->disp.state, desc->disp.arg);
        } else {
            ret = SVC_OP_DISPATCH;
        }
        break;
    case SMC_OP_TEST:
        if (test->val != test->orig >> test->count) {
            test->fail++;
        }
        test->val >>= 1;
        test->count++;
        ret = SVC_OP_TEST;
    case SMC_OP_GET_REG:
        if (desc->get.el == 3) {
            switch (desc->get.key) {
#if AARCH64
            case CURRENTEL:
                desc->get.data = READ_CURRENTEL();
                break;
            case CPTR_EL3:
                desc->get.data = READ_CPTR_EL3();
                break;
#endif
#if AARCH32
            case CPSR:
                desc->set.data = READ_CPSR();
                break;
#endif
            case CPACR:
                desc->get.data = READ_CPACR();
                break;
            case SCR:
                desc->get.data = READ_SCR();
                break;
            case SCTLR:
                desc->get.data = READ_SCTLR();
                break;
            default:
                DEBUG_MSG("Unrecognized SVC GET reg = %d\n", desc->set.key);
                break;
            }
        }
        break;
    case SMC_OP_SET_REG:
        if (desc->set.el == 3) {
            switch (desc->set.key) {
#if AARCH64
            case CURRENTEL:
                WRITE_CURRENTEL(desc->set.data);
                break;
            case CPTR_EL3:
                WRITE_CPTR_EL3(desc->set.data);
                break;
#endif
#if AARCH32
            case CPSR:
                WRITE_CPSR(desc->set.data);
                break;
#endif
            case CPACR:
                WRITE_CPACR(desc->set.data);
                break;
            case SCR:
                WRITE_SCR(desc->set.data);
                break;
            case SCTLR:
                WRITE_SCTLR(desc->set.data);
                break;
            default:
                DEBUG_MSG("Unrecognized SVC SET reg = %d\n", desc->set.key);
                break;
            }
        }
        break;
    default:
        DEBUG_MSG("Unrecognized AArch64 SMC opcode: op = %d\n", op);
        el3_shutdown();
        break;
    }

    /* EL3 can be secure or nonsecure.  Change our global secure state
     * indicator if we will be switching state.
     */
    if (ret) {
        secure_state ^= 1;
    }
    __set_exception_return(elr);
    return ret;
}

int el3_handle_exception(uintptr_t ec, uintptr_t iss, uintptr_t far,
					     uintptr_t elr)
{
    if (syscntl->excp.log) {
        syscntl->excp.taken = true;
        syscntl->excp.ec = ec;
        syscntl->excp.iss = iss;
        syscntl->excp.far = far;
        syscntl->excp.el = exception_level;
        syscntl->excp.state = 0;
    }

    switch (ec) {
    case EC_SMC64:
    case EC_SMC32:
        DEBUG_MSG("Took an SMC exception from EL3\n");
        break;
    case EC_IABORT_LOWER:
        DEBUG_MSG("Instruction abort at lower level: far = %0lx\n", far);
        el3_shutdown();
        break;
    case EC_IABORT:
        DEBUG_MSG("Instruction abort at EL3: far = %0lx\n", far);
        el3_shutdown();
        break;
    case EC_DABORT_LOWER:
        DEBUG_MSG("Data abort at lower level: far = %0lx elr = %0lx\n",
                  far, elr);
        el3_shutdown();
        break;
    case EC_DABORT:
        DEBUG_MSG("Data abort at EL3: far = %0lx elr = %0lx\n", far, elr);
        el3_shutdown();
        break;
    case EC_SYSINSN:
        DEBUG_MSG("System instruction exception far = 0x%lx  elr = 0x%lx\n",
                  far, elr);

        /* Other than system calls, synchronous exceptions return to the
         * offending instruction.  The user should have issued a SKIP.
         */
        if (syscntl->excp.action == EXCP_ACTION_SKIP) {
            elr +=4;
        }
        break;
    case EC_WFI_WFE:
        DEBUG_MSG("WFI/WFE instruction exception far = 0x%lx  elr = 0x%lx\n",
                  far, elr);

        /* Other than system calls, synchronous exceptions return to the
         * offending instruction.  The user should have issued a SKIP.
         */
        if (syscntl->excp.action == EXCP_ACTION_SKIP) {
            elr +=4;
        }
        break;
    case EC_SIMD:
        DEBUG_MSG("Adv SIMD or FP access exception - far = 0x%lx elr = 0x%lx\n",
                  far, elr);
        /* Other than system calls, synchronous exceptions return to the
         * offending instruction.  The user should have issued a SKIP.
         */
        if (syscntl->excp.action == EXCP_ACTION_SKIP) {
            elr +=4;
        }
        break;
    default:
        DEBUG_MSG("Unhandled EL3 exception: EC = 0x%lx  ISS = 0x%lx\n",
                  ec, iss);
        el3_shutdown();
        break;
    }

    /* We always restore the elr just in case there was a nested EL3 exception.
     * In some cases up above, the elr has been adjusted.
     */
    __set_exception_return(elr);

    return 0;
}

void el3_monitor_init()
{
    uintptr_t syscntl_pa = (uintptr_t)mem_lookup_pa(syscntl);

    /* Clear out our secure and non-secure state buffers */
    memset(&sec_state, 0, sizeof(sec_state));
    memset(&nsec_state, 0, sizeof(nsec_state));

    /* Below we setup the initial register states for when we start the secure
     * and non-secure images.  In both cases we set up register 4 (index 0) to
     * contain the system control block physical address.
     * In the case of AArch64, we also have to set SPSEL to 1 so that the stack
     * used before and after the switch is the same.
     * In the case of the non-secure state, we also have to set-up the initial
     * exception LR and SPSR as we do an SMC from secure to non-secure.  For
     * the secure side, we just perform an exception return with the target LR
     * and SPSR.
     */
#ifdef AARCH64
    sec_state.spsel = 0x1;
#endif
    sec_state.reg[0] = syscntl_pa;

    nsec_state.elr_el3 = EL1_NS_FLASH_BASE;
    nsec_state.spsr_el3 = SPSR_EL1;
#ifdef AARCH64
    nsec_state.spsel = 0x1;
#endif
    nsec_state.reg[0] = syscntl_pa;
}

void el3_start(uintptr_t base, uintptr_t size)
{
    uintptr_t addr = base;
    size_t len;

    printf("EL%d started...\n", exception_level);

    /* Unmap the init segement so we don't accidentally use it */
    for (len = 0; len < ((size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1));
         len += PAGE_SIZE, addr += PAGE_SIZE) {
        mem_unmap_va(addr);
    }

    syscntl = mem_heap_allocate(PAGE_SIZE);

    smc_interop_buf = mem_heap_allocate(PAGE_SIZE);
    syscntl->smc_interop.buf_va = smc_interop_buf;
    syscntl->smc_interop.buf_pa = mem_lookup_pa(smc_interop_buf);

    el3_monitor_init();

    /* Initialize SCR. In particular, we must ensure that SCR_EL3.RW is 1
     * so that we define EL1 as AArch64; otherwise the exception return
     * will fail; we also want SCR_EL3.NS to be 0.
     * Setting SCR_ST and SCR_HCE isn't necessary but this configuration
     * is essentially "turn off all the trap-to-EL3 possibilities".
     */
#ifdef AARCH64
    WRITE_SCR(SCR_ST | SCR_RW | SCR_HCE);
#else
    WRITE_SCR(SCR_HCE);
#endif

    /* Set-up our state to return to secure EL1 to start its init on exception
     * return.
     */
    monitor_restore_state(&sec_state);
    __exception_return(EL1_S_FLASH_BASE, SPSR_EL1);
}
