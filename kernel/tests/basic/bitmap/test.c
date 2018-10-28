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
    int err = bitmap_allocate_range(&bitmap, 2, 0, bitmap.elements, &index);
    assert(err == 1);
    assert(index == 0);

    err = bitmap_allocate_range(&bitmap, 2, 0, bitmap.elements, &index);
    assert(err == 1);
    assert(index == 2);
    (void) err;
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

void test_run(void)
{
    test_range_succ();
    test_range_fail();
    test_set_get();
    test_storage_move();
    puts("Bitmap test passed.\n");
}