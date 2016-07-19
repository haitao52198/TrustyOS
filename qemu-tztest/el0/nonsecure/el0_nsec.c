#include "el0_common.h"
#include "tztest.h"

char *sec_state_str[] = {"secure", "nonsecure"};
uint32_t secure_state = NONSECURE;
const uint32_t exception_level = EL0;
sys_control_t *syscntl = NULL;

int main()
{
    svc_op_desc_t desc;

    printf("EL0 (%s) started...\n", sec_state_str[secure_state]);

    /* Fetch the system-wide control structure */
    __svc(SVC_OP_GET_SYSCNTL, &desc);
    syscntl = ((sys_control_t *)desc.get.data);

    /* Allocate and globally map test control descriptor */
    syscntl->test_cntl = (test_control_t*)alloc_mem(0, 0x1000);
    map_va(syscntl->test_cntl, 0x1000, OP_MAP_ALL);

    /* If we didn't get a valid control structure then something has already
     * gone drastically wrong.
     */
    if (!syscntl) {
        DEBUG_MSG("Failed to acquire system control structure\n");
        __svc(SVC_OP_EXIT, &desc);
    }

    tztest_start();

    __svc(SVC_OP_EXIT, NULL);

    return 0;
}
