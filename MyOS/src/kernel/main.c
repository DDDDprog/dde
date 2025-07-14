#include "kernel.h"
#include "vga.h"
#include "interrupts.h"
#include "keyboard.h"

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

/* Simple shell command structure */
struct command {
    const char* name;
    const char* description;
    void (*function)(void);
};

/* Shell commands */
void cmd_help(void);
void cmd_clear(void);
void cmd_version(void);
void cmd_memory(void);
void cmd_reboot(void);

static struct command commands[] = {
    {"help", "Show available commands", cmd_help},
    {"clear", "Clear the screen", cmd_clear},
    {"version", "Show kernel version", cmd_version},
    {"memory", "Show memory information", cmd_memory},
    {"reboot", "Reboot the system", cmd_reboot},
    {NULL, NULL, NULL}
};

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
    
    /* Print welcome message */
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("MyOS v1.0.0 - Custom Operating System\n");
    vga_puts("=====================================\n\n");
    
    /* Verify multiboot magic */
    if (magic != 0x2BADB002) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("ERROR: Invalid multiboot magic number!\n");
        vga_printf("Expected: 0x2BADB002, Got: 0x%x\n", magic);
        kernel_panic("Multiboot verification failed");
    }
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("Multiboot verification: OK\n");
    
    /* Skip interrupt initialization for testing */
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("Skipping interrupt initialization for testing...\n");
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("System initialization complete!\n\n");
    
    /* Show memory info */
    if (mboot_info->flags & 0x1) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        vga_printf("Available memory: %d KB lower, %d KB upper\n\n", 
                   mboot_info->mem_lower, mboot_info->mem_upper);
    }
    
    /* Display success message */
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("MyOS kernel loaded successfully!\n");
    vga_puts("This is a basic operating system kernel.\n\n");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("Features implemented:\n");
    vga_puts("- Multiboot compliance\n");
    vga_puts("- VGA text mode driver\n");
    vga_puts("- Basic string functions\n");
    vga_puts("- Memory management basics\n\n");
    
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts("System is now idle. Kernel halted.\n");
    
    /* Serial output for debugging */
    const char* halt_msg = "MyOS kernel halted successfully\n";
    for (int i = 0; halt_msg[i]; i++) {
        outb(0x3F8, halt_msg[i]); /* COM1 serial port */
    }
    
    /* Infinite loop */
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void cmd_help(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("Available commands:\n");
    vga_puts("==================\n");
    
    for (int i = 0; commands[i].name != NULL; i++) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        vga_printf("  %-10s", commands[i].name);
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_printf(" - %s\n", commands[i].description);
    }
}

void cmd_clear(void) {
    vga_clear();
}

void cmd_version(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_printf("MyOS Version %d.%d.%d\n", 
               KERNEL_VERSION_MAJOR, 
               KERNEL_VERSION_MINOR, 
               KERNEL_VERSION_PATCH);
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("Built with GCC and NASM\n");
    vga_puts("Copyright (c) 2024 MyOS Project\n");
}

void cmd_memory(void) {
    if (mboot_info->flags & 0x1) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        vga_puts("Memory Information:\n");
        vga_puts("==================\n");
        vga_printf("Lower memory: %d KB\n", mboot_info->mem_lower);
        vga_printf("Upper memory: %d KB\n", mboot_info->mem_upper);
        vga_printf("Total memory: %d KB\n", mboot_info->mem_lower + mboot_info->mem_upper);
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("Memory information not available\n");
    }
}

void cmd_reboot(void) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts("Rebooting system...\n");
    
    /* Reboot using keyboard controller */
    uint8_t temp;
    __asm__ volatile ("cli");
    do {
        temp = inb(0x64);
        if (temp & 0x01) {
            inb(0x60);
        }
    } while (temp & 0x02);
    
    outb(0x64, 0xFE);
    
    /* Fallback: triple fault */
    __asm__ volatile ("int $0x00");
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