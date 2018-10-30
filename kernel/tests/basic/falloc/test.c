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
    err = my_frame_free(first_addr, 4);
    assert(err == EOK);
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
 * This test leaves fragmented memory intentionally.
 */
static void my_test_alloc_and_free_fragmented(void)
{
    uintptr_t addr = 0;
    int err = my_frame_alloc(&addr, 10, VF_VA_AUTO);
    assert(err == EOK);

    uintptr_t fourth_frame_addr = addr + FRAME_SIZE * 3;
    err = my_frame_free(fourth_frame_addr, 1);
    assert(err == EOK);

    /* Try to free a block that was already freed. */
    err = my_frame_free(fourth_frame_addr, 1);
    assert(err == EINVAL);

    uintptr_t fifth_frame_addr = fourth_frame_addr + FRAME_SIZE;
    err = my_frame_free(fifth_frame_addr, 1);
    assert(err == EOK);

    /* Try to allocate 2 consecutive blocks that were just freed. */
    err = my_frame_alloc(&fourth_frame_addr, 2, VF_VA_USER);
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

void test_run(void)
{
    my_frame_init();
    my_frame_test();

    my_test_allocation();
    my_test_alloc_and_free();
    my_test_alloc_and_free_fragmented();
    my_test_alloc_and_free_3();

    puts("Falloc test passed\n");
}
