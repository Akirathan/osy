#include <api.h>
#include <adt/bitmap.h>
#include "../../include/defs.h"

void my_frame_init(void);
int my_frame_alloc (uintptr_t *phys, const size_t cnt, const vm_flags_t flags);

static bitmap_t bitmap;
/* Address of memory from which the frames will be allocated. */
static uintptr_t frames_begin = 0;
/* Total number of usable frames. */
static size_t usable_frames = 0;

#define MAX_MEMORY_MB 512U

static void test_allocation(void)
{
    uintptr_t addr = 0;
    int err = my_frame_alloc(&addr, 2, VF_VA_AUTO);
    assert (err == EOK);
}

/** Tests whether given address is usable
 * 
 * Tries to write one byte to memory within the frame boundaries and then read
 * the value.
 * 
 * @param addr Starting address of a frame.
 */
static bool mem_accessible(const uint32_t addr)
{
    const uint32_t testing_offset = 10;
    const uint8_t testing_value = 42;
    uint8_t *ptr_to_addr = (uint8_t *) (addr + testing_offset);

    *ptr_to_addr = testing_value;

    if (*ptr_to_addr == testing_value) {
        return true;
    }
    else {
        return false;
    }
}

/** Scan the whole usable memory
 * 
 * @return Number of usable frames.
 */
static size_t scan_memory(void)
{
    uint32_t addr = ADDR_IN_KSEG1((uint32_t)&_kernel_end);
    unsigned int increments = 0;

    while (mem_accessible(addr)) {
        addr += (uint32_t) FRAME_SIZE;
        increments++;
    }

    assert(increments > 0);
    return (size_t) increments;
}

/** Counts how many frames will bitmap need
 * 
 * @return Number of frames needed for bitmap.
 */
static size_t count_bitmap_storage(const size_t frame_num)
{
    /* How many frames can be held in a bitmap that is allocated in one frame. */
    const size_t one_frame_bitmap = FRAME_SIZE * BITMAP_ELEMENT_BITS;

    /* How many frames to allocate for the bitmap. */
    size_t frames_for_bitmap = (frame_num / one_frame_bitmap) + 1;
    assert(frames_for_bitmap >= 1 && frames_for_bitmap <= 4);

    return frames_for_bitmap;
}

static uint32_t frame_to_addr(const size_t frame_num)
{
    // TODO: return from usable_memory
    return ADDR_IN_KSEG1(frame_num * FRAME_SIZE);
}

void my_frame_init(void)
{
    uintptr_t kernel_end = ADDR_IN_KSEG1((uint32_t)&_kernel_end);
    size_t all_frames = scan_memory();
    size_t frames_for_bitmap = count_bitmap_storage(all_frames);
    usable_frames = all_frames - frames_for_bitmap;

    bitmap_init(&bitmap, usable_frames, (void *)kernel_end);

    frames_begin = ADDR_IN_KSEG1((uintptr_t)&_kernel_end);
    frames_begin += frames_for_bitmap * FRAME_SIZE;
}

/**
 * @param cnt Number of frames to allocate.
 */
int my_frame_alloc(uintptr_t *phys, const size_t cnt, const vm_flags_t flags)
{
    if (flags & VF_VA_AUTO) {
        size_t index = 0;
        bool err = bitmap_allocate_range(&bitmap, cnt, 0, bitmap.elements, &index);
        if (err != false) {
            /* Allocation successfull. */
            *phys = frame_to_addr(index);
            return EOK;
        }
        else {
            /* Cannot allocate. */
            return ENOMEM;
        }
    }

    return EINVAL;
}

void test_run(void)
{
    my_frame_init();
    test_allocation();

    puts("Test passed\n");
}
