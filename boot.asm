; boot.asm - Simple bootloader for CustomOS
; author cybermyki

BITS 32

section .multiboot
    ; Multiboot header for GRUB
    MULTIBOOT_MAGIC equ 0x1BADB002
    MULTIBOOT_FLAGS equ 0x0
    MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
    
    align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .bss
    align 16
    stack_bottom:
        resb 16384  ; 16 KB stack
    stack_top:

section .text
    global _start
    extern kernel_main

_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Reset EFLAGS
    push 0
    popf
    
    ; Call kernel main function
    call kernel_main
    
    ; Halt if kernel returns
.hang:
    cli
    hlt
    jmp .hang

; Global Descriptor Table (GDT) for protected mode
section .data
gdt_start:
    dq 0x0000000000000000  ; Null descriptor
    
gdt_code:
    dw 0xFFFF    ; Limit
    dw 0x0000    ; Base (low)
    db 0x00      ; Base (middle)
    db 10011010b ; Access byte
    db 11001111b ; Flags + Limit (high)
    db 0x00      ; Base (high)
    
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
    
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start