#include <api.h>
#include "../../include/defs.h"
#include "mm/my_falloc.h"

/**
 * Do some allocations and then test an address that should be free.
 */
static void my_test_allocation(void)
{
    uintptr_t first_addr = 0;
    uintptr_t sec_addr = 0;
    int err = my_frame_alloc(&first_addr, 2, VF_VA_AUTO);
    assert (err == EOK);

    err = my_frame_alloc(&sec_addr, 2, VF_VA_AUTO);
    assert(err == EOK);
    assert(first_addr != sec_addr);

    /* Check allocations. */
    err = my_frame_alloc(&first_addr, 2, VF_VA_USER);
    assert(err == ENOMEM);
    err = my_frame_alloc(&sec_addr, 2, VF_VA_USER);
    assert(err == ENOMEM);

    /* Points at frame that is above sec_addr. */
    uintptr_t big_addr = sec_addr + (2 * FRAME_SIZE);
    err = my_frame_alloc(&big_addr, 2, VF_VA_USER);
    assert(err == EOK);

    /* Cleanup */
    err = my_frame_free(first_addr, 2);
    assert(err == EOK);
    err = my_frame_free(sec_addr, 2);
    assert(err == EOK);
    err = my_frame_free(big_addr, 2);
}

static void my_test_alloc_and_free(void)
{
    uintptr_t first_addr = 0;
    uintptr_t _first_addr = 0;
    int err = my_frame_alloc(&first_addr, 2, VF_VA_AUTO);
    _first_addr = first_addr;
    assert(err == EOK);

    err = my_frame_free(first_addr, 2);
    assert(err == EOK);

    err = my_frame_alloc(&first_addr, 2, VF_VA_AUTO);
    assert(err == EOK);
    /* Memory was freed - it should be allocated into same address. */
    assert(first_addr == _first_addr);

    /* Cleanup */
    err = my_frame_free(first_addr, 2);
    assert(err == EOK);
}

/**
 * Allocate huge chunk of memory and free the whole block by subchunks.
 */
static void my_test_alloc_and_free_3(void)
{
    const size_t count = 10;
    uintptr_t addr = 0;
    int err = my_frame_alloc(&addr, count, VF_VA_AUTO);
    assert(err == EOK);

    for (size_t i = 0; i < count; ++i) {
        err = my_frame_free(addr, 1);
        assert(err == EOK);
        addr += FRAME_SIZE;
    }
}

/**
 * Allocates small chunks of memory and tests whether they are consecutive.
 */
static void my_test_alloc_continuous(void)
{
    uintptr_t first_addr = 0;
    int err = my_frame_alloc(&first_addr, 2, VF_VA_AUTO);
    assert(err == EOK);

    uintptr_t sec_addr = 0;
    err = my_frame_alloc(&sec_addr, 2, VF_VA_AUTO);
    assert(err == EOK);
    assert(sec_addr == first_addr + (2 * FRAME_SIZE));
}

void test_run(void)
{
    my_frame_init();
    my_frame_test();

    my_test_allocation();
    my_test_alloc_and_free();
    my_test_alloc_and_free_3();
    my_test_alloc_continuous();

    puts("Falloc test passed\n");
}
