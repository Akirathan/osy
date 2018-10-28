#include <api.h>
#include "../../include/defs.h"
#include "mm/my_falloc.h"

static void test_allocation(void)
{
    uintptr_t addr = 0;
    int err = my_frame_alloc(&addr, 2, VF_VA_AUTO);
    assert (err == EOK);
    err = my_frame_alloc(&addr, 2, VF_VA_AUTO);
    assert(err == EOK);
}

void test_run(void)
{
    my_frame_init();
    test_allocation();

    puts("Falloc test passed\n");
}
