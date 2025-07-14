#include "keyboard.h"
#include "interrupts.h"
#include "kernel.h"

/* Keyboard state */
static char keyboard_buffer[256];
static int buffer_head = 0;
static int buffer_tail = 0;
static int shift_pressed = 0;
static int caps_lock = 0;

/* US QWERTY keyboard layout */
static char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char scancode_to_ascii_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Port I/O functions */
extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t data);

static void keyboard_buffer_put(char c) {
    int next_head = (buffer_head + 1) % 256;
    if (next_head != buffer_tail) {
        keyboard_buffer[buffer_head] = c;
        buffer_head = next_head;
    }
}

static char keyboard_buffer_get(void) {
    if (buffer_head == buffer_tail) {
        return 0;
    }
    
    char c = keyboard_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % 256;
    return c;
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    /* Handle key releases (bit 7 set) */
    if (scancode & 0x80) {
        scancode &= 0x7F;
        if (scancode == KEY_SHIFT_LEFT || scancode == KEY_SHIFT_RIGHT) {
            shift_pressed = 0;
        }
        return;
    }
    
    /* Handle special keys */
    switch (scancode) {
        case KEY_SHIFT_LEFT:
        case KEY_SHIFT_RIGHT:
            shift_pressed = 1;
            return;
            
        case KEY_CAPS_LOCK:
            caps_lock = !caps_lock;
            return;
            
        case KEY_CTRL:
        case KEY_ALT:
            return; /* Ignore for now */
    }
    
    /* Convert scancode to ASCII */
    char ascii = 0;
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            ascii = scancode_to_ascii_shift[scancode];
        } else {
            ascii = scancode_to_ascii[scancode];
        }
        
        /* Handle caps lock for letters */
        if (caps_lock && ascii >= 'a' && ascii <= 'z') {
            ascii = ascii - 'a' + 'A';
        } else if (caps_lock && ascii >= 'A' && ascii <= 'Z') {
            ascii = ascii - 'A' + 'a';
        }
        
        if (ascii) {
            keyboard_buffer_put(ascii);
        }
    }
}

void keyboard_init(void) {
    buffer_head = 0;
    buffer_tail = 0;
    shift_pressed = 0;
    caps_lock = 0;
    
    /* Install keyboard interrupt handler */
    interrupt_install_handler(33, keyboard_handler);
}

char keyboard_getchar(void) {
    char c;
    while ((c = keyboard_buffer_get()) == 0) {
        __asm__ volatile ("hlt"); /* Wait for interrupt */
    }
    return c;
}

int keyboard_available(void) {
    return buffer_head != buffer_tail;
}