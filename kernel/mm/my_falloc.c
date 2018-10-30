#include "my_falloc.h"
#include <adt/bitmap.h>

static void clear_buffer(const uintptr_t addr, const size_t bytes_count);
static bool is_device_addr(const uintptr_t addr);
static bool mem_accessible(const uint32_t addr);
static size_t scan_memory(void);
static size_t count_bitmap_storage(const size_t frame_num);
static uintptr_t frame_to_addr(const size_t frame_index);
static size_t addr_to_frame(const uintptr_t addr);
static bool is_addr_aligned(const uintptr_t addr);
static void bitmap_lock_init(void);
static void bitmap_lock(void);
static void bitmap_unlock(void);
static bool allocate_range(const size_t count, const size_t base, const size_t constraint,
                           size_t *index);
static bool check_range(const size_t start, const size_t count);
static void set_range(const size_t start, const size_t count);
static void clear_range(const size_t start, const size_t count);

static bitmap_t bitmap;
static struct mutex bitmap_mtx;
/* Virtual address of memory (in KSEG1) from which the frames will be allocated. */
static uintptr_t frames_begin_kseg = 0;
/* Total number of usable frames. */
static size_t usable_frames = 0;

#define KSEG1_BASE 0xA0000000
#define VIRT_TO_PHYS(addr)  (((uintptr_t) addr) - (uintptr_t) KSEG1_BASE)
#define PHYS_TO_VIRT(addr)  (((uintptr_t) addr) + (uintptr_t) KSEG1_BASE)

static void clear_buffer(const uintptr_t addr, const size_t bytes_count)
{
    uint8_t *buffer = (uint8_t *)addr;

    for (size_t i = 0; i < bytes_count; ++i) {
        *buffer = 0;
        buffer++;
    }
}

/** Checks if given address belongs to some device.
 */
static bool is_device_addr(const uintptr_t addr)
{

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
    /* TODO: Check maximum amount of memory is not overrun. */
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

/** Gets physical address of given frame.
 * 
 * @return Physical address of the frame.
 */
static inline uintptr_t frame_to_addr(const size_t frame_index)
{
    return VIRT_TO_PHYS(frames_begin_kseg + (frame_index * FRAME_SIZE));
}

/** Converts physical address to frame index.
 * 
 * @return Index of the frame corresponding to @p addr.
 */
static inline size_t addr_to_frame(const uintptr_t addr)
{
    assert(is_addr_aligned(addr));

    /* Shifting by FRAME_WIDTH is the same as dividing by FRAME_SIZE */
    size_t frame_index = (PHYS_TO_VIRT(addr) - frames_begin_kseg) >> FRAME_WIDTH;
    return frame_index;
}

static inline bool is_addr_aligned(const uintptr_t addr)
{
    uintptr_t base_addr = PHYS_TO_VIRT(addr) - frames_begin_kseg;
    /* Check whether last 12 bits are 0. */
    return ((base_addr % FRAME_SIZE) == 0);
}

static inline void bitmap_lock_init(void)
{
    mutex_init(&bitmap_mtx);
}

static inline void bitmap_lock(void)
{
    mutex_lock(&bitmap_mtx);
}

static inline void bitmap_unlock(void)
{
    mutex_unlock(&bitmap_mtx);
}

/** Wrapper for bitmap_allocate_range
 */
static bool allocate_range(const size_t count, const size_t base, const size_t constraint,
                           size_t *index)
{
    bitmap_lock();
    bool err = bitmap_allocate_range(&bitmap, count, base, constraint, index);
    bitmap_unlock();
    return err;
}

/** Wrapper for bitmap_check_range
 */
static bool check_range(const size_t start, const size_t count)
{
    bitmap_lock();
    bool err = bitmap_check_range(&bitmap, start, count);
    bitmap_unlock();
    return err;
}

static void set_range(const size_t start, const size_t count)
{
    bitmap_lock();
    bitmap_set_range(&bitmap, start, count);
    bitmap_unlock();
}

/** Wrapper for bitmap_clear_range
 */
static void clear_range(const size_t start, const size_t count)
{
    bitmap_lock();
    bitmap_clear_range(&bitmap, start, count);
    bitmap_unlock();
}

void my_frame_init(void)
{
    uintptr_t kernel_end = ADDR_IN_KSEG1((uint32_t)&_kernel_end);
    size_t all_frames = scan_memory();
    size_t frames_for_bitmap = count_bitmap_storage(all_frames);
    usable_frames = all_frames - frames_for_bitmap;

    /* Initialize bitmap with clean storage. */
    clear_buffer(kernel_end, frames_for_bitmap * FRAME_SIZE);
    bitmap_init(&bitmap, usable_frames, (void *)kernel_end);

    bitmap_lock_init();

    frames_begin_kseg = ADDR_IN_KSEG1((uintptr_t)&_kernel_end);
    frames_begin_kseg += frames_for_bitmap * FRAME_SIZE;
}

/**
 * @param cnt Number of frames to allocate.
 */
int my_frame_alloc(uintptr_t *phys, const size_t cnt, const vm_flags_t flags)
{
    if (cnt == 0) {
        return ENOMEM;
    }

    if (flags & VF_VA_AUTO) {
        size_t index = 0;
        /* TODO: do not begin from 0 */
        bool err = allocate_range(cnt, 0, bitmap.elements, &index);
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
    /* Do not move with phys, but allocate anyway. */
    else if (flags & VF_VA_USER) {
        if (!is_addr_aligned(*phys)) {
            return EINVAL;
        }

        size_t frame_index = addr_to_frame(*phys);
        bool err = check_range(frame_index, cnt);
        if (err == true) {
            /* Given frame range is full. */
            return ENOMEM;
        }
        else {
            /* Allocate the frame range. */
            set_range(frame_index, cnt);
            return EOK;
        }
    }

    return EINVAL;
}

int my_frame_free(const uintptr_t phys, const size_t cnt)
{
    if (cnt == 0 || !is_addr_aligned(phys)) {
        return EINVAL;
    }

    size_t frame_index = addr_to_frame(phys);

    bool err = check_range(frame_index, cnt);
    if (err == true) {
        /* The given memory range was allocated --> free it */
        clear_range(frame_index, cnt);
        return EOK;
    }
    else {
        /* Given memory range was not allocated */
        return EINVAL;
    }
}

void my_frame_test(void)
{
    /* Test conversion from frame index to address and vice versa. */
    for (size_t i = 0; i < 10; i++) {
        uintptr_t addr = frame_to_addr(i);
        size_t frame_index = addr_to_frame(addr);
        assert(frame_index == i);
    }
}
