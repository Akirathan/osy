#include <api.h>
#include <adt/bitmap.h>
#include "../../include/defs.h"

static void clear(uint8_t *buffer, const size_t buff_len)
{
    for (size_t i = 0; i < buff_len; i++) {
        *(buffer + i) = 0;
    }
}

static void test_range_succ(void)
{
    bitmap_t bitmap;
    uint8_t storage[10];
    bitmap_init(&bitmap, 4, storage);

    size_t index = 0;
    bool err = bitmap_allocate_range(&bitmap, 2, 0, bitmap.elements, &index);
    assert(err == true);
    assert(index == 0);

    err = bitmap_allocate_range(&bitmap, 2, 0, bitmap.elements, &index);
    assert(err == true);
    assert(index == 2);

    /* Bitmap should look like this: [1,1,1,1] */
    err = bitmap_check_range(&bitmap, 0, 4);
    assert(err == true);
}

static void test_range_fail(void)
{
    bitmap_t bitmap;
    uint8_t storage[10];
    bitmap_init(&bitmap, 2, storage);

    size_t index = 0;
    int err = bitmap_allocate_range(&bitmap, 4, 0, bitmap.elements, &index);
    assert(err == 0);
}

static void test_storage_move(void)
{
    bitmap_t bitmap;
    uint8_t storage[10];
    uint8_t *orig_storage_ptr = storage;
    clear(storage, 10);

    bitmap_init(&bitmap, 5, storage);
    size_t index = 0;
    bitmap_allocate_range(&bitmap, 2, 0, bitmap.elements, &index);
    bitmap_allocate_range(&bitmap, 2, 0, bitmap.elements, &index);

    assert(storage == orig_storage_ptr);
}

static void test_set_get(void)
{
    bitmap_t bitmap;
    const size_t storage_len = 2;
    uint8_t storage[storage_len];

    clear(storage, storage_len);

    bitmap_init(&bitmap, 8, storage);
    bitmap_set(&bitmap, 1, 1);
    bitmap_set(&bitmap, 4, 1);

    assert(bitmap_get(&bitmap, 1) == 1);
    assert(bitmap_get(&bitmap, 4) == 1);
}

static void test_check_range(void)
{
    bitmap_t bitmap;
    uint8_t storage[10];
    clear(storage, 10);

    bitmap_init(&bitmap, 5, storage);
    bitmap_set_range(&bitmap, 0, 2);

    /* This range is set */
    int err = bitmap_check_range(&bitmap, 0, 2);
    assert(err == true);

    /* This range is empty */
    err = bitmap_check_range(&bitmap, 2, 2);
    assert(err == false);

    bitmap_set(&bitmap, 2, 0);
    bitmap_set_range(&bitmap, 3, 2);

    /* Bitmap now looks: [1,1,0,1,1] */
    err = bitmap_check_range(&bitmap, 0, 5);
    assert(err == false);

    err = bitmap_check_range(&bitmap, 1, 3);
    assert(err == false);
}

/**
 * Tests the bitmap with storage beginning from _kernel_end
 */
static void test_in_kseg(void)
{
    bitmap_t bitmap;
    uintptr_t kernel_end = ADDR_IN_KSEG1((uint32_t)&_kernel_end);

    /* Initialize bitmap with storage in kernel_end (KSEG1). */
    clear((uint8_t *)kernel_end, FRAME_SIZE);
    bitmap_init(&bitmap, 236, (void *)kernel_end);

    /* Allocate first range. */
    size_t first_index = 0;
    bool err = bitmap_allocate_range(&bitmap, 2, 0, bitmap.elements, &first_index);
    assert(err == true);
    assert(first_index == 0);
    err = bitmap_check_range(&bitmap, 0, 2);
    assert(err == true);

    /* Allocate second range. */
    size_t sec_index = 0;
    err = bitmap_allocate_range(&bitmap, 2, 0, bitmap.elements, &sec_index);
    assert(err == true);
    assert(first_index != sec_index);
    assert(sec_index == 2);

    /* Check that both ranges were successfully allocated. */
    err = bitmap_check_range(&bitmap, 0, 4);
    assert(err == true);
}

void test_run(void)
{
    test_range_succ();
    test_range_fail();
    test_set_get();
    test_storage_move();
    test_check_range();
    test_in_kseg();
    puts("Bitmap test passed.\n");
}