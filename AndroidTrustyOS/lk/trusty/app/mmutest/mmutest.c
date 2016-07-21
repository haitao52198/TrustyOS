/*
 * Copyright (c) 2015, Google Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <err.h>
#include <kernel/thread.h>
#include <kernel/vm.h>
#include <lk/init.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MMUTEST_RUN_FATAL_TESTS 0

int mmutest_arch_rodata_pnx(void);
int mmutest_arch_data_pnx(void);
int mmutest_arch_rodata_ro(void);

int mmutest_arch_store_uint32(uint32_t *ptr, bool user);
int mmutest_arch_nop(int ret);
int mmutest_arch_nop_end(int ret);

static int mmutest_vmm_store_uint32(uint arch_mmu_flags, bool user)
{
    int ret;
    void *ptr;
    uint arch_mmu_flags_query = ~0;

    ret = vmm_alloc_contiguous(vmm_get_kernel_aspace(), "mmutest", PAGE_SIZE,
                               &ptr, 0, 0, arch_mmu_flags);

    if (ret)
        return ret;

    arch_mmu_query((vaddr_t)ptr, NULL, &arch_mmu_flags_query);
    if (arch_mmu_flags_query != arch_mmu_flags) {
        printf("%s: Warning: arch_mmu_query flags, 0x%x, does not match requested flags, 0x%x\n",
               __func__, arch_mmu_flags_query, arch_mmu_flags);
    }

    ret = mmutest_arch_store_uint32(ptr, user);

    vmm_free_region(vmm_get_kernel_aspace(), (vaddr_t)ptr);
    return ret;
}

static int mmutest_vmm_store_uint32_kernel(uint arch_mmu_flags)
{
    return mmutest_vmm_store_uint32(arch_mmu_flags, false);
}

static int mmutest_vmm_store_uint32_user(uint arch_mmu_flags)
{
    return mmutest_vmm_store_uint32(arch_mmu_flags, true);
}

static int mmu_test_nx(void)
{
    int ret;
    void *ptr;
    size_t len;
    int (*nop)(int ret);

    ret = vmm_alloc_contiguous(vmm_get_kernel_aspace(), "mmutest", PAGE_SIZE,
                               &ptr, 0, 0, ARCH_MMU_FLAG_PERM_NO_EXECUTE);
    if (ret)
        return ret;

    nop = ptr;
    len = mmutest_arch_nop_end - mmutest_arch_nop;

    memcpy(ptr, mmutest_arch_nop, len);
    arch_sync_cache_range((addr_t)ptr, len);

    ret = nop(0);

    vmm_free_region(vmm_get_kernel_aspace(), (vaddr_t)ptr);

    return ret;
}

static void print_result(int ret, const char *funcname, int expect)
{
    if (ret == expect) {
        printf("PASSED - %s\n", funcname);
    } else {
        printf("*FAILED* - %s - expected %d, got %d\n", funcname, expect, ret);
    }
}

#define RUN_TEST(name, expect) \
    print_result(name(), #name, expect)

#define RUN_TEST1(name, arg, expect) \
    print_result(name(arg), #name "(" #arg ")", expect)

#define RUN_TEST2(name, arg1, arg2, expect) \
    print_result(name(arg), #name "(" #arg1 "," #arg2 ")", expect)

static void mmutest_init(uint level)
{
    RUN_TEST(mmutest_arch_rodata_pnx, ERR_FAULT);
    RUN_TEST(mmutest_arch_data_pnx, ERR_FAULT);
    RUN_TEST(mmutest_arch_rodata_ro, ERR_FAULT);

    RUN_TEST1(mmutest_vmm_store_uint32_kernel, ARCH_MMU_FLAG_CACHED, 0);
    RUN_TEST1(mmutest_vmm_store_uint32_kernel, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_PERM_USER, 0);
    RUN_TEST1(mmutest_vmm_store_uint32_kernel, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_PERM_NO_EXECUTE, 0);
    RUN_TEST1(mmutest_vmm_store_uint32_kernel, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_PERM_NO_EXECUTE|ARCH_MMU_FLAG_PERM_USER, 0);
    RUN_TEST1(mmutest_vmm_store_uint32_kernel, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_PERM_RO, ERR_FAULT);
    RUN_TEST1(mmutest_vmm_store_uint32_kernel, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_PERM_RO|ARCH_MMU_FLAG_PERM_USER, ERR_FAULT);
    RUN_TEST1(mmutest_vmm_store_uint32_kernel, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_NS, 2);
    RUN_TEST1(mmutest_vmm_store_uint32_kernel, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_NS|ARCH_MMU_FLAG_PERM_USER, 2);
    RUN_TEST1(mmutest_vmm_store_uint32_user, ARCH_MMU_FLAG_CACHED, ERR_GENERIC);
    RUN_TEST1(mmutest_vmm_store_uint32_user, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_PERM_USER, 0);
    RUN_TEST1(mmutest_vmm_store_uint32_user, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_PERM_NO_EXECUTE, ERR_GENERIC);
    RUN_TEST1(mmutest_vmm_store_uint32_user, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_PERM_NO_EXECUTE|ARCH_MMU_FLAG_PERM_USER, 0);
    RUN_TEST1(mmutest_vmm_store_uint32_user, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_PERM_RO, ERR_GENERIC);
    RUN_TEST1(mmutest_vmm_store_uint32_user, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_PERM_RO|ARCH_MMU_FLAG_PERM_USER, ERR_FAULT);
    RUN_TEST1(mmutest_vmm_store_uint32_user, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_NS, ERR_GENERIC);
    RUN_TEST1(mmutest_vmm_store_uint32_user, ARCH_MMU_FLAG_CACHED|ARCH_MMU_FLAG_NS|ARCH_MMU_FLAG_PERM_USER, 2);
#if MMUTEST_RUN_FATAL_TESTS
    printf("Starting fatal tests, expect crash\n");
    RUN_TEST(mmu_test_nx, ERR_FAULT);
#endif
}

LK_INIT_HOOK(mmutest, mmutest_init, LK_INIT_LEVEL_APPS);
