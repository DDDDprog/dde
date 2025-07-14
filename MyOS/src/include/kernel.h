#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

/* Kernel version */
#define KERNEL_VERSION_MAJOR 1
#define KERNEL_VERSION_MINOR 0
#define KERNEL_VERSION_PATCH 0

/* Memory constants */
#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PAGE_SIZE 4096

/* Multiboot constants */
#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_FLAGS 0x00000003

/* Forward declarations */
struct multiboot_info;

/* Function prototypes */
void kernel_main(uint32_t magic, struct multiboot_info* mboot);
void kernel_panic(const char* message);
void idle_process(void);

/* Memory management */
void* kmalloc(size_t size);
void kfree(void* ptr);

/* String functions */
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
int strcmp(const char* str1, const char* str2);
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);

/* Port I/O functions */
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t data);

#endif /* KERNEL_H */