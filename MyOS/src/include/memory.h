#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024
#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PAGE_NUMBER (KERNEL_VIRTUAL_BASE >> 22)

/* Page directory and table entry flags */
#define PAGE_PRESENT    0x001
#define PAGE_WRITE      0x002
#define PAGE_USER       0x004
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040

/* Memory regions */
#define MEMORY_KERNEL_START 0x100000
#define MEMORY_KERNEL_END   0x400000
#define MEMORY_USER_START   0x400000
#define MEMORY_USER_END     0x800000

typedef struct page_directory_entry {
    uint32_t present    : 1;
    uint32_t write      : 1;
    uint32_t user       : 1;
    uint32_t writethrough : 1;
    uint32_t cache_disable : 1;
    uint32_t accessed   : 1;
    uint32_t reserved   : 1;
    uint32_t page_size  : 1;
    uint32_t global     : 1;
    uint32_t available  : 3;
    uint32_t address    : 20;
} __attribute__((packed)) page_directory_entry_t;

typedef struct page_table_entry {
    uint32_t present    : 1;
    uint32_t write      : 1;
    uint32_t user       : 1;
    uint32_t writethrough : 1;
    uint32_t cache_disable : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t reserved   : 1;
    uint32_t global     : 1;
    uint32_t available  : 3;
    uint32_t address    : 20;
} __attribute__((packed)) page_table_entry_t;

typedef struct page_directory {
    page_directory_entry_t entries[PAGE_ENTRIES];
} page_directory_t;

typedef struct page_table {
    page_table_entry_t entries[PAGE_ENTRIES];
} page_table_t;

/* Physical memory management */
typedef struct memory_block {
    uint32_t address;
    uint32_t size;
    int free;
    struct memory_block* next;
} memory_block_t;

/* Memory management functions */
void memory_init(uint32_t mem_lower, uint32_t mem_upper);
void paging_init(void);
void* kmalloc(uint32_t size);
void kfree(void* ptr);
void* vmalloc(uint32_t size);
void vfree(void* ptr);

/* Page management */
uint32_t alloc_page(void);
void free_page(uint32_t page);
void map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
void unmap_page(uint32_t virtual_addr);
page_directory_t* create_page_directory(void);
void switch_page_directory(page_directory_t* dir);

/* Memory information */
void memory_info(void);
uint32_t get_free_memory(void);
uint32_t get_used_memory(void);

#endif