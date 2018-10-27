#include <api.h>
#include "../../include/defs.h"

void scan_memory_test(void);

void scan_memory_test(void)
{
    // Try to trigger exception
    size_t boundary = 1024 * 1024; // 1 MB
    size_t address = KSEG1_BASE + 40 * boundary;
    uint8_t *ptr_to_high_addr = (uint8_t *) address;
    //uint8_t *ptr_to_correct_addr = (uint8_t *) (_kernel_end + 300);

    printk("Address = %u\n", address);

    *ptr_to_high_addr = 23; // This does not trigger an exception
    if (*ptr_to_high_addr == 23) {
        printk("Successfully written to %u\n", address);
    }
    else {
        printk("Not written to %u\n", address);
    }

    uint8_t *ptr_to_correct_addr = (uint8_t *) (KSEG1_BASE + boundary + 30);
    *ptr_to_correct_addr = 23;
    if (*ptr_to_correct_addr == 23) {
        printk("Successfully written to %u\n", (KSEG1_BASE + boundary + 30));
    }
    else {
        printk("Not written to %u\n", (KSEG1_BASE + boundary + 30));
    }
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

    return (size_t) increments;
}

void test_run(void)
{
    size_t frames = scan_memory();
    printk("Test passed, number of frames = %u\n", frames);
}