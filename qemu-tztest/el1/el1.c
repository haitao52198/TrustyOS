#include "el1_common.h"
#include "exception.h"
#include "vmsa.h"
#include "mem_util.h"
#include "tztest.h"

uintptr_t mem_pgtbl_base = EL1_PGTBL_BASE;
smc_op_desc_t *smc_interop_buf;
sys_control_t *syscntl;
uintptr_t mem_next_pa = 0;
uintptr_t mem_next_l1_page = 0;
uintptr_t mem_heap_pool = EL1_VA_HEAP_BASE;

const char *svc_op_name[] = {
    [SVC_OP_EXIT] = "SVC_OP_EXIT",
    [SVC_OP_ALLOC] = "SVC_OP_ALLOC",
    [SVC_OP_MAP] = "SVC_OP_MAP",
    [SVC_OP_YIELD] = "SVC_OP_YIELD",
    [SVC_OP_GET_SYSCNTL] = "SVC_OP_GET_SYSCNTL",
    [SVC_OP_GET_REG] = "SVC_OP_GET_REG",
    [SVC_OP_SET_REG] = "SVC_OP_SET_REG",
    [SVC_OP_TEST] = "SVC_OP_TEST",
    [SVC_OP_DISPATCH] = "SVC_OP_DISPATCH"
};

void el1_alloc_mem(op_alloc_mem_t *alloc)
{
    alloc->addr = mem_heap_allocate(alloc->len);
}

void el1_map_secure(op_map_mem_t *map)
{
    if ((map->type & OP_MAP_EL3) == OP_MAP_EL3) {
        smc_op_desc_t *desc = (smc_op_desc_t *)smc_interop_buf;
        memcpy(desc, map, sizeof(op_map_mem_t));
        if (desc->map.pa) {
            desc->map.pa = mem_lookup_pa(desc->map.va);
        }
        __smc(SMC_OP_MAP, desc);
    } else {
        mem_map_pa((uintptr_t)map->va, (uintptr_t)map->pa, 0, PTE_USER_RW);
    }
}

void el1_interop_test(op_test_t *desc)
{
    op_test_t *test = (op_test_t*)smc_interop_buf;

    memcpy(smc_interop_buf, desc, sizeof(smc_op_desc_t));
    if (test->val != test->orig >> test->count) {
        test->fail++;
    }
    test->val >>= 1;
    test->count++;

    __smc(SMC_OP_TEST, smc_interop_buf);
    if (test->val != test->orig >> test->count) {
        test->fail++;
    }
    test->val >>= 1;
    test->count++;
    memcpy(desc, smc_interop_buf, sizeof(smc_op_desc_t));
}

int el1_handle_svc(uint32_t op, svc_op_desc_t *desc,
                   uintptr_t __attribute((unused))far, uintptr_t elr)
{
    uint32_t ret = 0;

    DEBUG_MSG("Took an svc(%s) - desc = %p\n", svc_op_name[op], desc);
    switch (op) {
    case SVC_OP_EXIT:
        SMC_EXIT();
        break;
    case SVC_OP_YIELD:
        ret = SMC_YIELD();
        memcpy(desc, smc_interop_buf, sizeof(smc_op_desc_t));
        break;
    case SVC_OP_ALLOC:
        el1_alloc_mem((op_alloc_mem_t *)desc);
        break;
    case SVC_OP_MAP:
        el1_map_secure((op_map_mem_t *)desc);
        break;
    case SVC_OP_GET_SYSCNTL:
        desc->get.data = (uintptr_t)syscntl;
        break;
    case SVC_OP_GET_REG:
        if (desc->get.el == 1) {
            switch (desc->get.key) {
#if AARCH64
            case CURRENTEL:
                desc->get.data = READ_CURRENTEL();
                break;
            case CPTR_EL3:
                desc->get.data = READ_CPTR_EL3();
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
        } else if (desc->get.el == 3) {
            memcpy(smc_interop_buf, desc, sizeof(smc_op_desc_t));
            __smc(SMC_OP_GET_REG, smc_interop_buf);
            memcpy(desc, smc_interop_buf, sizeof(smc_op_desc_t));
        }
        break;
    case SVC_OP_SET_REG:
        if (desc->set.el == 1) {
            switch (desc->set.key) {
#if AARCH64
            case CURRENTEL:
                WRITE_CURRENTEL(desc->set.data);
                break;
            case CPTR_EL3:
                WRITE_CPTR_EL3(desc->set.data);
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
        } else if (desc->set.el == 3) {
            memcpy(smc_interop_buf, desc, sizeof(smc_op_desc_t));
            __smc(SMC_OP_SET_REG, smc_interop_buf);
        }
        break;
    case SVC_OP_TEST:
        el1_interop_test((op_test_t *)desc);
        break;
    case SVC_OP_DISPATCH:
        if (desc->disp.el == exception_level &&
            desc->disp.state == secure_state)  {
            run_test(desc->disp.fid, desc->disp.el,
                     desc->disp.state, desc->disp.arg);
        } else {
            memcpy(smc_interop_buf, desc, sizeof(smc_op_desc_t));
            __smc(SMC_OP_DISPATCH, smc_interop_buf);
            memcpy(desc, smc_interop_buf, sizeof(smc_op_desc_t));
        }
        break;
    default:
        DEBUG_MSG("Unrecognized SVC opcode: op = %d\n", op);
        break;
    }

    /* We always restore the elr just in case we hit a nested EL1 exception in
     * the handler.  This can happen as we run exception tests.
     */
    __set_exception_return(elr);

    return ret;
}

void el1_handle_exception(uintptr_t ec, uintptr_t iss, uintptr_t far,
     					uintptr_t elr)
{
    if (syscntl->excp.log) {
        syscntl->excp.taken = true;
        syscntl->excp.ec = ec;
        syscntl->excp.iss = iss;
        syscntl->excp.far = far;
        syscntl->excp.el = exception_level;
        syscntl->excp.state = secure_state;
    }

    switch (ec) {
    case EC_UNKNOWN:
        DEBUG_MSG("Unknown exception far = 0x%lx  elr = 0x%lx\n", far, elr);

        /* The preferred return address in the case of an undefined instruction
         * is the actual offending instruction.  In the case of ARMv7, we need
         * to decrement the address by 4 to get this behavior as the PC has
         * already been moved past this instruction.
         * If SKIP is enabled then we can just do nothing and get the correct
         * behavior.
         */
#if AARCH32
        if (syscntl->excp.action != EXCP_ACTION_SKIP) {
            elr += 4;
        }
#else
        if (syscntl->excp.action == EXCP_ACTION_SKIP) {
            elr +=4;
        }
#endif
        break;
    case EC_IABORT_LOWER:
        DEBUG_MSG("Instruction abort at lower level: far = %0lx\n", far);
        SMC_EXIT();
        break;
    case EC_IABORT:
        DEBUG_MSG("Instruction abort at EL1: far = %0lx\n", far);
        SMC_EXIT();
        break;
    case EC_DABORT_LOWER:
        DEBUG_MSG("Data abort at lower level: far = %0lx elr = %0lx\n",
                  far, elr);
        SMC_EXIT();
        break;
    case EC_DABORT:
        DEBUG_MSG("Data abort at EL1: far = %0lx elr = %0lx\n", far, elr);
        SMC_EXIT();
        break;
    case EC_WFI_WFE:
        DEBUG_MSG("WFI/WFE instruction exception far = 0x%lx  elr = 0x%lx\n",
                  far, elr);

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
            elr += 4;
        }
        break;
    default:
        DEBUG_MSG("Unhandled EL1 exception: ec = 0x%x iss = 0x%x\n", ec, iss);
        SMC_EXIT();
        break;
    }

    /* We always restore the elr just in case there was a nested EL1 exception.
     * In some cases up above, the elr has been adjusted.
     */
    __set_exception_return(elr);
}

void el1_start(uintptr_t base, uintptr_t size)
{
    uintptr_t addr = base;
    size_t len;

    printf("EL1 (%s) started...\n", sec_state_str);

    /* Unmap the init segement so we don't accidentally use it */
    for (len = 0; len < ((size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1));
         len += PAGE_SIZE, addr += PAGE_SIZE) {
        mem_unmap_va(addr);
    }

    void *pa = syscntl;
    syscntl = (sys_control_t *)mem_heap_allocate(PAGE_SIZE);
    mem_map_pa((uintptr_t)syscntl, (uintptr_t)pa, 0, PTE_USER_RW);
    mem_map_pa((uintptr_t)syscntl->smc_interop.buf_va,
               (uintptr_t)syscntl->smc_interop.buf_pa, 0, PTE_USER_RW);
    smc_interop_buf = syscntl->smc_interop.buf_va;

    el1_init_el0();

    return;
}
