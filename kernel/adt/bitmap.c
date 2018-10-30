#include "bitmap.h"

/** Finds appropriate byte and fills in byte_index.
 */
static uint8_t get_byte(const bitmap_t *bitmap, const size_t start, size_t *byte_index)
{
    size_t _byte_index = 0;
    _byte_index = start / BITMAP_ELEMENT_BITS;

    if (byte_index != NULL) {
        *byte_index = _byte_index;
    }

    return bitmap->bits[_byte_index];
}

/** Initializes appropriate bit mask.
 */
static uint8_t get_mask(const size_t start, const size_t count)
{
    uint8_t mask = 0;
    for (size_t i = 0; i < count; ++i) {
        mask |= 0x01;
        mask <<= 1;
    }
    mask <<= start;

    return mask;
}

static inline bool is_bit_set(const uint8_t byte, const size_t bit_index)
{
    assert(bit_index < 8);

    uint8_t mask = 0x01;
    mask <<= bit_index;

    return (byte & mask) == mask;
}

void bitmap_init (bitmap_t *bitmap, size_t elements, void *storage)
{
    assert(bitmap != NULL);
    assert(storage != NULL);
    assert(elements > 0);

    bitmap->elements = elements;
    bitmap->bits = storage;
}

/** Checks whether given range is set.
 * 
 * @return 1 (true) if given range is set.
 *         0 (false) otherwise.
 */
int bitmap_check_range (bitmap_t *bitmap, size_t start, size_t count)
{
    assert(bitmap != NULL);
    assert(start + count <= bitmap->elements);

    uint8_t byte = get_byte(bitmap, start, NULL);
    uint8_t mask = get_mask(start, count);

    /* Check if mask is set in byte. */
    if ((byte & mask) == mask) {
        return true;
    }
    else {
        return false;
    }
}

/** Sets the given range of bits to 1.
 */
void bitmap_set_range (bitmap_t *bitmap, size_t start, size_t count)
{
    assert(bitmap != NULL);
    assert(start + count <= bitmap->elements);

    size_t byte_index = 0;
    uint8_t byte = get_byte(bitmap, start, &byte_index);
    uint8_t mask = get_mask(start, count);

    byte |= mask;

    bitmap->bits[byte_index] = byte;
}

/** Sets the given range of bits to 0.
 */
void bitmap_clear_range (bitmap_t *bitmap, size_t start, size_t count)
{
    assert(bitmap != NULL);
    assert(start + count <= bitmap->elements);

    size_t byte_index = 0;
    uint8_t byte = get_byte(bitmap, start, &byte_index);
    uint8_t mask = get_mask(start, count);

    byte &= ~mask;

    bitmap->bits[byte_index] = byte;
}

/** Tries to find continuous gap of 0 bits and sets them to 1.
 * 
 * @param base Starting index.
 * @param constraint Maximum index.
 * @param[out] index Index of first allocated bit.
 */
int bitmap_allocate_range (bitmap_t *bitmap, size_t count, size_t base,
    size_t constraint, size_t *index)
{
    assert(bitmap != NULL);
    assert(index != NULL);
    assert(constraint < bitmap->elements);
    assert(base + count <= bitmap->elements);

    size_t max_index = 0;
    size_t max_len = 0;

    /* Iterate over bits. */
    /* TODO: make this cycle more efficient. */
    for (size_t i = 0; i < constraint; i++) {
        unsigned int val = bitmap_get(bitmap, i);
        if (val) {
            /* Check if this is first bit after gap. */
            if (max_index == 0) {
                max_index = i;
            }

            max_len++;
            /* Check if appropriate gap was found. */
            if (max_len == count) {
                bitmap_set_range(bitmap, max_index, count);
                *index = max_index;
                return true;
            }
        }
        else {
            max_index = 0;
            max_len = 0;
        }
    }
}
