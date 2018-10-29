#include <api.h>
#include "../../include/defs.h"
#include "mm/my_falloc.h"

static void my_test_allocation(void)
{
    uintptr_t first_addr = 0;
    uintptr_t sec_addr = 0;
    int err = my_frame_alloc(&first_addr, 2, VF_VA_AUTO);
    assert (err == EOK);

    err = my_frame_alloc(&sec_addr, 2, VF_VA_AUTO);
    assert(err == EOK);
    assert(first_addr != sec_addr);

    err = my_frame_alloc(&first_addr, 2, VF_VA_USER);
    assert(err == ENOMEM);

    err = my_frame_alloc(&sec_addr, 2, VF_VA_USER);
    assert(err == ENOMEM);

    /* Points at frame that is above sec_addr. */
    uintptr_t big_addr = sec_addr + (2 * FRAME_SIZE);
    err = my_frame_alloc(&big_addr, 2, VF_VA_USER);
    assert(err == EOK);
}

void test_run(void)
{
    my_frame_init();
    my_frame_test();
    my_test_allocation();

    puts("Falloc test passed\n");
}
