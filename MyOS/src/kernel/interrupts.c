#include "interrupts.h"
#include "kernel.h"
#include "vga.h"
#include "timer.h"
#include "keyboard.h"

/* IDT and interrupt handlers */
static struct idt_entry idt[IDT_SIZE];
static struct idt_ptr idt_pointer;
static interrupt_handler_t interrupt_handlers[IDT_SIZE];

/* PIC constants */
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

/* Exception messages */
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void idt_init(void) {
    /* Initialize IDT pointer */
    idt_pointer.limit = sizeof(idt) - 1;
    idt_pointer.base = (uint32_t)&idt;
    
    /* Clear IDT and handlers */
    memset(&idt, 0, sizeof(idt));
    memset(&interrupt_handlers, 0, sizeof(interrupt_handlers));
    
    /* Remap PIC */
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);
    
    /* Install ISRs */
    idt_set_gate(0, (uint32_t)isr0, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, KERNEL_CODE_SEGMENT, 0x8E);
    
    /* Install IRQs */
    idt_set_gate(32, (uint32_t)irq0, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, KERNEL_CODE_SEGMENT, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, KERNEL_CODE_SEGMENT, 0x8E);
    
    /* Load IDT */
    __asm__ volatile ("lidt %0" : : "m"(idt_pointer));
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void interrupt_install_handler(uint8_t interrupt, interrupt_handler_t handler) {
    interrupt_handlers[interrupt] = handler;
}

/* ISR handler */
void isr_handler(uint32_t interrupt_number, uint32_t error_code) {
    if (interrupt_handlers[interrupt_number]) {
        interrupt_handlers[interrupt_number]();
    } else {
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
        vga_printf("Unhandled exception: %s (Error: 0x%x)\n", 
                   exception_messages[interrupt_number], error_code);
        kernel_panic("Unhandled CPU exception");
    }
}

/* IRQ handler */
void irq_handler(uint32_t irq_number, uint32_t error_code) {
    (void)error_code; /* Unused parameter */
    
    /* Handle specific IRQs */
    switch (irq_number) {
        case 32: /* Timer IRQ0 */
            timer_handler();
            break;
        case 33: /* Keyboard IRQ1 */
            keyboard_handler();
            break;
        default:
            if (interrupt_handlers[irq_number]) {
                interrupt_handlers[irq_number]();
            }
            break;
    }
    
    /* Send EOI to PIC */
    if (irq_number >= 40) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
}