#include "heap.h"

uint8_t heap[MAX_HEAP_SIZE] = {0};

static uint32_t heap_offset = 0;

uint8_t heapMalloc(uint16_t size, uint8_t **start_addr, uint8_t **end_addr) {
    if ((size == 0U) || (start_addr == NULL) || (end_addr == NULL)) {
        return 0;
    }

    if ((heap_offset + size) > MAX_HEAP_SIZE) {
        return 0;
    }

    *start_addr = &heap[heap_offset];
    *end_addr   = *start_addr + size - 1U;

    heap_offset += size;

    return 1;
}
