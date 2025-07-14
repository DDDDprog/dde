#include "memory.h"
#include "kernel.h"
#include "vga.h"

/* Global memory management variables */
static uint32_t total_memory = 0;
static uint32_t used_memory = 0;
static uint32_t kernel_end = 0;
static memory_block_t* memory_blocks = NULL;
static page_directory_t* kernel_directory = NULL;
static page_directory_t* current_directory = NULL;

/* Bitmap for physical page allocation */
static uint32_t* page_bitmap = NULL;
static uint32_t page_bitmap_size = 0;
static uint32_t total_pages = 0;

void memory_init(uint32_t mem_lower, uint32_t mem_upper) {
    total_memory = (mem_lower + mem_upper) * 1024; /* Convert KB to bytes */
    kernel_end = MEMORY_KERNEL_END;
    used_memory = kernel_end - MEMORY_KERNEL_START;
    
    vga_printf("Memory initialization:\n");
    vga_printf("  Lower memory: %d KB\n", mem_lower);
    vga_printf("  Upper memory: %d KB\n", mem_upper);
    vga_printf("  Total memory: %d KB\n", total_memory / 1024);
    vga_printf("  Kernel size: %d KB\n", used_memory / 1024);
    
    /* Initialize physical page bitmap */
    total_pages = total_memory / PAGE_SIZE;
    page_bitmap_size = (total_pages + 31) / 32; /* 32 bits per uint32_t */
    page_bitmap = (uint32_t*)(kernel_end);
    kernel_end += page_bitmap_size * sizeof(uint32_t);
    
    /* Mark all pages as used initially */
    for (uint32_t i = 0; i < page_bitmap_size; i++) {
        page_bitmap[i] = 0xFFFFFFFF;
    }
    
    /* Mark available pages as free */
    uint32_t available_start = kernel_end / PAGE_SIZE;
    uint32_t available_end = total_memory / PAGE_SIZE;
    
    for (uint32_t i = available_start; i < available_end; i++) {
        uint32_t index = i / 32;
        uint32_t bit = i % 32;
        page_bitmap[index] &= ~(1 << bit);
    }
    
    /* Initialize kernel heap */
    memory_blocks = (memory_block_t*)kernel_end;
    memory_blocks->address = kernel_end + sizeof(memory_block_t);
    memory_blocks->size = MEMORY_USER_START - memory_blocks->address;
    memory_blocks->free = 1;
    memory_blocks->next = NULL;
    
    vga_puts("Physical memory manager initialized\n");
}

void paging_init(void) {
    /* Create kernel page directory */
    kernel_directory = (page_directory_t*)kmalloc(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    
    /* Identity map the first 4MB (kernel space) */
    for (uint32_t i = 0; i < 1024; i++) {
        uint32_t physical_addr = i * PAGE_SIZE;
        map_page(physical_addr, physical_addr, PAGE_PRESENT | PAGE_WRITE);
    }
    
    /* Switch to our page directory */
    switch_page_directory(kernel_directory);
    
    /* Enable paging */
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; /* Set PG bit */
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
    
    vga_puts("Paging enabled\n");
}

void* kmalloc(uint32_t size) {
    /* Align size to 4-byte boundary */
    size = (size + 3) & ~3;
    
    memory_block_t* block = memory_blocks;
    while (block) {
        if (block->free && block->size >= size) {
            /* Split block if it's much larger than needed */
            if (block->size > size + sizeof(memory_block_t) + 16) {
                memory_block_t* new_block = (memory_block_t*)(block->address + size);
                new_block->address = block->address + size + sizeof(memory_block_t);
                new_block->size = block->size - size - sizeof(memory_block_t);
                new_block->free = 1;
                new_block->next = block->next;
                
                block->size = size;
                block->next = new_block;
            }
            
            block->free = 0;
            used_memory += size;
            return (void*)block->address;
        }
        block = block->next;
    }
    
    return NULL; /* Out of memory */
}

void kfree(void* ptr) {
    if (!ptr) return;
    
    memory_block_t* block = memory_blocks;
    while (block) {
        if (block->address == (uint32_t)ptr) {
            block->free = 1;
            used_memory -= block->size;
            
            /* Merge with next block if it's free */
            if (block->next && block->next->free) {
                block->size += block->next->size + sizeof(memory_block_t);
                block->next = block->next->next;
            }
            
            /* Merge with previous block if it's free */
            memory_block_t* prev = memory_blocks;
            while (prev && prev->next != block) {
                prev = prev->next;
            }
            if (prev && prev->free) {
                prev->size += block->size + sizeof(memory_block_t);
                prev->next = block->next;
            }
            
            return;
        }
        block = block->next;
    }
}

uint32_t alloc_page(void) {
    for (uint32_t i = 0; i < page_bitmap_size; i++) {
        if (page_bitmap[i] != 0xFFFFFFFF) {
            for (int bit = 0; bit < 32; bit++) {
                if (!(page_bitmap[i] & (1 << bit))) {
                    page_bitmap[i] |= (1 << bit);
                    return (i * 32 + bit) * PAGE_SIZE;
                }
            }
        }
    }
    return 0; /* Out of physical memory */
}

void free_page(uint32_t page) {
    uint32_t page_num = page / PAGE_SIZE;
    uint32_t index = page_num / 32;
    uint32_t bit = page_num % 32;
    
    if (index < page_bitmap_size) {
        page_bitmap[index] &= ~(1 << bit);
    }
}

void map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    uint32_t page_dir_index = virtual_addr >> 22;
    uint32_t page_table_index = (virtual_addr >> 12) & 0x3FF;
    
    page_directory_entry_t* dir_entry = &current_directory->entries[page_dir_index];
    
    if (!dir_entry->present) {
        /* Create new page table */
        uint32_t page_table_phys = alloc_page();
        if (!page_table_phys) return;
        
        page_table_t* page_table = (page_table_t*)page_table_phys;
        memset(page_table, 0, sizeof(page_table_t));
        
        dir_entry->address = page_table_phys >> 12;
        dir_entry->present = 1;
        dir_entry->write = 1;
        dir_entry->user = (flags & PAGE_USER) ? 1 : 0;
    }
    
    page_table_t* page_table = (page_table_t*)(dir_entry->address << 12);
    page_table_entry_t* table_entry = &page_table->entries[page_table_index];
    
    table_entry->address = physical_addr >> 12;
    table_entry->present = (flags & PAGE_PRESENT) ? 1 : 0;
    table_entry->write = (flags & PAGE_WRITE) ? 1 : 0;
    table_entry->user = (flags & PAGE_USER) ? 1 : 0;
    
    /* Invalidate TLB entry */
    __asm__ volatile ("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}

void switch_page_directory(page_directory_t* dir) {
    current_directory = dir;
    __asm__ volatile ("mov %0, %%cr3" : : "r"(dir) : "memory");
}

void memory_info(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Memory Information:\n");
    vga_puts("==================\n");
    
    vga_printf("Total memory: %d KB\n", total_memory / 1024);
    vga_printf("Used memory:  %d KB\n", used_memory / 1024);
    vga_printf("Free memory:  %d KB\n", (total_memory - used_memory) / 1024);
    vga_printf("Total pages:  %d\n", total_pages);
    
    /* Count free pages */
    uint32_t free_pages = 0;
    for (uint32_t i = 0; i < page_bitmap_size; i++) {
        for (int bit = 0; bit < 32; bit++) {
            if (!(page_bitmap[i] & (1 << bit))) {
                free_pages++;
            }
        }
    }
    
    vga_printf("Free pages:   %d\n", free_pages);
    vga_printf("Used pages:   %d\n", total_pages - free_pages);
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

uint32_t get_free_memory(void) {
    return total_memory - used_memory;
}

uint32_t get_used_memory(void) {
    return used_memory;
}