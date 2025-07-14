/* MyOS Linker Script */
/* Defines memory layout and section organization */

ENTRY(start)

SECTIONS
{
    /* Kernel starts at 1MB physical address */
    . = 0x00100000;

    /* Multiboot header must be early in the file */
    .multiboot ALIGN(4K) : {
        *(.multiboot)
    }

    /* Text section - executable code */
    .text ALIGN(4K) : {
        *(.text)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    /* Initialized data */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Uninitialized data (BSS) */
    
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }

    /* Define symbols for kernel end */
    kernel_end = .;
}