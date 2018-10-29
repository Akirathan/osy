#ifndef MY_FALLOC_H
#define MY_FALLOC_H

#include <api.h>

void my_frame_init(void);
int my_frame_alloc(uintptr_t *phys, const size_t cnt, const vm_flags_t flags);
int my_frame_free (const uintptr_t phys, const size_t cnt);
void my_frame_test(void);

#endif
