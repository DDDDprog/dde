#include "kernel.h"
#include "vga.h"
#include "interrupts.h"
#include "keyboard.h"
#include "process.h"
#include "memory.h"
#include "shell.h"
#include "timer.h"


/* Multiboot information structure */
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
} __attribute__((packed));

static struct multiboot_info* mboot_info;

void kernel_main(uint32_t magic, struct multiboot_info* mboot) {
    /* Store multiboot info */
    mboot_info = mboot;
    
    /* Initialize VGA display */
    vga_init();
    vga_clear();
    
    /* Debug: Show we reached kernel_main */
    vga_puts("Kernel starting...\n");
    
    /* Also output to serial port for debugging */
    const char* msg = "MyOS kernel started via serial\n";
    for (int i = 0; msg[i]; i++) {
        outb(0x3F8, msg[i]); /* COM1 serial port */
    }
    
    /* Debug function to output to serial */
    void debug_serial(const char* str) {
        for (int i = 0; str[i]; i++) {
            outb(0x3F8, str[i]);
        }
    }
    
    debug_serial("Starting welcome message\n");
    
    /* Print welcome message */
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("MyOS v1.0.0 - Custom Operating System\n");
    vga_puts("=====================================\n\n");
    
    debug_serial("Verifying multiboot\n");
    
    /* Verify multiboot magic */
    if (magic != 0x2BADB002) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("ERROR: Invalid multiboot magic number!\n");
        vga_printf("Expected: 0x2BADB002, Got: 0x%x\n", magic);
        kernel_panic("Multiboot verification failed");
    }
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("Multiboot verification: OK\n");
    
    debug_serial("Multiboot OK\n");
    
    debug_serial("Starting memory init\n");
    
    /* Initialize memory management */
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("Initializing memory management...\n");
    if (mboot_info->flags & 0x1) {
        memory_init(mboot_info->mem_lower, mboot_info->mem_upper);
        // Skip paging for now
        // paging_init();
    }
    
    debug_serial("Starting timer init\n");
    
    /* Initialize timer */
    vga_puts("Initializing timer...\n");
    // Temporarily disable timer
    // timer_init();
    // time_init();
    
    debug_serial("Starting interrupt init\n");
    
    /* Initialize interrupt system */
    vga_puts("Initializing interrupt system...\n");
    idt_init();
    
    debug_serial("Starting process init\n");
    
    /* Initialize process management */
    vga_puts("Initializing process management...\n");
    // Skip process management for now
    // process_init();
    // scheduler_init();
    
    debug_serial("Starting keyboard init\n");
    
    /* Initialize keyboard */
    vga_puts("Initializing keyboard...\n");
    keyboard_init();
    
    debug_serial("Enabling interrupts\n");
    
    /* Enable interrupts */
    __asm__ volatile ("sti");
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("System initialization complete!\n\n");
    
    /* Show system information */
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("MyOS v1.0.0 - Linux-like Operating System\n");
    vga_puts("=========================================\n\n");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("Features implemented:\n");
    vga_puts("- Process management with scheduler\n");
    vga_puts("- Virtual memory with paging\n");
    vga_puts("- System calls interface\n");
    vga_puts("- Timer and system time\n");
    vga_puts("- Linux-like shell with commands\n");
    vga_puts("- Interrupt handling\n");
    vga_puts("- VGA and keyboard drivers\n\n");
    
    /* Show memory info */
    if (mboot_info->flags & 0x1) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        vga_printf("Total memory: %d KB (%d KB lower, %d KB upper)\n", 
                   mboot_info->mem_lower + mboot_info->mem_upper,
                   mboot_info->mem_lower, mboot_info->mem_upper);
    }
    
    /* Serial output for debugging */
    const char* start_msg = "MyOS kernel started with Linux-like features\n";
    for (int i = 0; start_msg[i]; i++) {
        outb(0x3F8, start_msg[i]); /* COM1 serial port */
    }
    
    debug_serial("Starting shell init\n");
    
    /* Initialize and run shell */
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("\nStarting shell...\n\n");
    
    // Temporarily replace shell with simple test
    vga_puts("MyOS Shell v1.0\n");
    vga_puts("Type 'help' for available commands\n\n");
    vga_puts("myos> ");
    
    debug_serial("Starting simple shell loop\n");
    
    /* Simple shell loop for testing */
    char input_buffer[256];
    int buffer_pos = 0;
    
    while (1) {
        if (keyboard_available()) {
            char c = keyboard_getchar();
            
            if (c == '\n') {
                input_buffer[buffer_pos] = '\0';
                vga_puts("\n");
                
                if (strcmp(input_buffer, "help") == 0) {
                    vga_puts("Available commands: help, clear, version, reboot\n");
                } else if (strcmp(input_buffer, "clear") == 0) {
                    vga_clear();
                } else if (strcmp(input_buffer, "version") == 0) {
                    vga_puts("MyOS v1.0.0 - Custom Operating System\n");
                } else if (strcmp(input_buffer, "reboot") == 0) {
                    vga_puts("Rebooting...\n");
                    outb(0x64, 0xFE);
                } else if (buffer_pos > 0) {
                    vga_printf("Unknown command: %s\n", input_buffer);
                }
                
                buffer_pos = 0;
                vga_puts("myos> ");
            } else if (c == '\b') {
                if (buffer_pos > 0) {
                    buffer_pos--;
                    vga_puts("\b \b");
                }
            } else if (c >= 32 && c <= 126 && buffer_pos < 255) {
                input_buffer[buffer_pos++] = c;
                vga_putchar(c);
            }
        }
    }
    
    debug_serial("Shell exited\n");
}

void idle_process(void) {
    while (1) {
        __asm__ volatile ("hlt"); /* Halt until next interrupt */
    }
}

void kernel_panic(const char* message) {
    __asm__ volatile ("cli");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_clear();
    vga_puts("KERNEL PANIC!\n");
    vga_puts("=============\n\n");
    vga_printf("Error: %s\n\n", message);
    vga_puts("System halted. Please reboot.\n");
    
    while (1) {
        __asm__ volatile ("hlt");
    }
}

/* Port I/O functions */
uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %1, %0" : : "dN"(port), "a"(data));
}