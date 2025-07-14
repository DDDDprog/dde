#include "vga.h"
#include "kernel.h"

/* VGA state */
static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static int vga_row = 0;
static int vga_column = 0;
static uint8_t vga_color = 0x0F; /* White on black */

/* Helper functions */
static uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

static uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | bg << 4;
}

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_buffer = (uint16_t*)VGA_MEMORY;
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_column = 0;
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    vga_color = vga_entry_color(fg, bg);
}

void vga_set_cursor(int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        vga_column = x;
        vga_row = y;
    }
}

static void vga_scroll(void) {
    /* Move all lines up by one */
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    
    /* Clear the last line */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }
    
    vga_row = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
    } else if (c == '\b') {
        if (vga_column > 0) {
            vga_column--;
            vga_buffer[vga_row * VGA_WIDTH + vga_column] = vga_entry(' ', vga_color);
        }
    } else if (c == '\t') {
        vga_column = (vga_column + 8) & ~(8 - 1);
        if (vga_column >= VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) {
                vga_scroll();
            }
        }
    } else if (c >= 32) {
        vga_buffer[vga_row * VGA_WIDTH + vga_column] = vga_entry(c, vga_color);
        if (++vga_column == VGA_WIDTH) {
            vga_column = 0;
            if (++vga_row == VGA_HEIGHT) {
                vga_scroll();
            }
        }
    }
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

/* Simple printf implementation */
static void print_string(const char* str) {
    vga_puts(str);
}

static void print_number(int num, int base) {
    char buffer[32];
    char* ptr = buffer + 31;
    int negative = 0;
    
    *ptr = '\0';
    
    if (num == 0) {
        *(--ptr) = '0';
    } else {
        if (num < 0 && base == 10) {
            negative = 1;
            num = -num;
        }
        
        while (num > 0) {
            int digit = num % base;
            *(--ptr) = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
            num /= base;
        }
        
        if (negative) {
            *(--ptr) = '-';
        }
    }
    
    print_string(ptr);
}

void vga_printf(const char* format, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, format);
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd':
                    print_number(__builtin_va_arg(args, int), 10);
                    break;
                case 'x':
                    print_number(__builtin_va_arg(args, int), 16);
                    break;
                case 's':
                    print_string(__builtin_va_arg(args, char*));
                    break;
                case 'c':
                    vga_putchar(__builtin_va_arg(args, int));
                    break;
                case '%':
                    vga_putchar('%');
                    break;
                default:
                    vga_putchar('%');
                    vga_putchar(*format);
                    break;
            }
        } else {
            vga_putchar(*format);
        }
        format++;
    }
    
    __builtin_va_end(args);
}