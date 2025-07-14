; MyOS Boot Assembly
; Multiboot compliant kernel entry point

; Multiboot header constants
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000003
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Stack size
STACK_SIZE          equ 16384

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .bootstrap_stack, nobits
align 4
stack_bottom:
    resb STACK_SIZE
stack_top:

section .text
global start
extern kernel_main

start:
    ; Set up stack
    mov esp, stack_top
    
    ; Reset EFLAGS
    push 0
    popf
    
    ; Call kernel main with multiboot parameters
    push ebx        ; Multiboot info structure
    push eax        ; Multiboot magic number
    call kernel_main
    add esp, 8      ; Clean up stack
    
    ; Hang if kernel returns
    cli
.hang:
    hlt
    jmp .hang

; Global Descriptor Table (GDT)
section .data
align 4
gdt_start:
    ; Null descriptor
    dd 0x0
    dd 0x0
    
    ; Code segment descriptor
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10011010b    ; Access byte
    db 11001111b    ; Granularity byte
    db 0x00         ; Base (bits 24-31)
    
    ; Data segment descriptor
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10010010b    ; Access byte
    db 11001111b    ; Granularity byte
    db 0x00         ; Base (bits 24-31)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                ; Offset

; Code and data segment selectors
CODE_SEG equ gdt_start + 8
DATA_SEG equ gdt_start + 16