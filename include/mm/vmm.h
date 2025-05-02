// Updated vmm.h
#ifndef VMM_H
#define VMM_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PAGE_SIZE 4096
#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE   (1 << 1)
#define PAGE_USER    (1 << 2)

// Page Directory/Table Entry
typedef uint32_t page_table_entry_t;

// Page Table structure (contains 1024 entries)
typedef struct {
    page_table_entry_t entries[1024];
} page_table_t;

// Page Directory structure (contains 1024 entries)
typedef struct {
    page_table_entry_t entries[1024];
} page_directory_t;

// Virtual memory manager functions
void vmm_init(uint32_t* page_bitmap_start, uint64_t total_memory_bytes);
uint32_t* vmm_alloc_page(void);
void vmm_free_page(uint32_t* page);
size_t vmm_get_total_pages(void);
size_t vmm_get_used_pages(void);
size_t vmm_get_free_pages(void);

// Virtual memory mapping functions
void vmm_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
void vmm_unmap_page(uint32_t virtual_addr);
bool vmm_is_mapped(uint32_t virtual_addr);
uint32_t vmm_get_physical(uint32_t virtual_addr);
void vmm_switch_directory(page_directory_t* dir);
page_directory_t* vmm_get_kernel_directory(void);

#endif // VMM_H
