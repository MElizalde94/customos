/* linker.ld - Linker script for CustomOS */

ENTRY(_start)

SECTIONS
{
    /* Start at 1MB, conventional place for kernels */
    . = 1M;

    /* Multiboot header must be in first 8KB */
    .boot :
    {
        *(.multiboot)
    }

    /* Read-only code and data */
    .text ALIGN(4K) : AT(ADDR(.text))
    {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata))
    {
        *(.rodata)
    }

    /* Read-write data */
    .data ALIGN(4K) : AT(ADDR(.data))
    {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss))
    {
        *(COMMON)
        *(.bss)
    }
}