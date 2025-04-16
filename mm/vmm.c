#include "../include/mm/vmm.h"
#include "../include/video/vga.h"
#include "../include/kernel/panic/panic.h"

static uint8_t* page_bitmap = NULL;
static size_t total_pages = 0;
static size_t page_bitmap_size = 0;

void vmm_init(uint32_t* page_bitmap_start, uint64_t total_memory_bytes) {
    // Calculate total pages
    total_pages = total_memory_bytes / PAGE_SIZE;
    
    // Calculate bitmap size (1 bit per page, rounded up to nearest byte)
    page_bitmap_size = (total_pages + 7) / 8;
    
    // Set the bitmap pointer
    page_bitmap = (uint8_t*)page_bitmap_start;
    
    // Initialize all pages as free
    for (size_t i = 0; i < page_bitmap_size; i++) {
        page_bitmap[i] = 0;
    }
    
    // Mark first 1MB as used (256 pages)
    for (size_t i = 0; i < 256; i++) {
        size_t byte_idx = i / 8;
        size_t bit_idx = i % 8;
        page_bitmap[byte_idx] |= (1 << bit_idx);
    }
}

uint32_t* vmm_alloc_page(void) {
    // Start from page 256 (skip first 1MB)
    const size_t START_PAGE = 256;
    
    for (size_t i = START_PAGE; i < total_pages; i++) {
        size_t byte_idx = i / 8;
        size_t bit_idx = i % 8;
        
        if (!(page_bitmap[byte_idx] & (1 << bit_idx))) {
            page_bitmap[byte_idx] |= (1 << bit_idx);
            return (uint32_t*)(i * PAGE_SIZE);
        }
    }
    
    panic("PANIC! Out of memory: No free pages available");
    return NULL;
}

void vmm_free_page(uint32_t* page) {
    size_t page_idx = (size_t)page / PAGE_SIZE;
    
    if (page_idx >= total_pages) {
        panic("PANIC! Attempted to free invalid page");
        return;
    }
    
    size_t byte_idx = page_idx / 8;
    size_t bit_idx = page_idx % 8;
    page_bitmap[byte_idx] &= ~(1 << bit_idx);
}

size_t vmm_get_total_pages(void) {
    return total_pages;
}

size_t vmm_get_used_pages(void) {
    size_t used = 0;
    for (size_t i = 0; i < total_pages; i++) {
        size_t byte_idx = i / 8;
        size_t bit_idx = i % 8;
        if (page_bitmap[byte_idx] & (1 << bit_idx)) used++;
    }
    return used;
}

size_t vmm_get_free_pages(void) {
    return total_pages - vmm_get_used_pages();
}
