
#include <api.h>

static size_t total_mem = 0;

static void error_handler (void)
{
    printk("Memory allocation failed, total memory allocated = %d\n", total_mem);
}

void test_run (void)
{
    const size_t chunk_size = 8;

    for (size_t i = 0; i < 10000; i++) {
        void *ptr = malloc(chunk_size);

        if (ptr == NULL) {
            error_handler();
            return;
        }

        total_mem += chunk_size;
    }

    printk("Memory allocation succeeded, total memory allocated = %d\n", total_mem);
}