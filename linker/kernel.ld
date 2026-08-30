ENTRY(_init)

KERNEL_VIRT = 0xC0000000;
KERNEL_PHYS = 0x00100000;

SECTIONS
{
    /*Kernel init*/
    . = KERNEL_PHYS;

    .kinit ALIGN(4K) :
    {
        start_kinit = .;
        *(.kinit)
        end_kinit = .;
    }

    /*Higher half*/
    . += KERNEL_VIRT - KERNEL_PHYS;

    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRT + KERNEL_PHYS)
    {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRT + KERNEL_PHYS)
    {
        *(.rodata*)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRT + KERNEL_PHYS)
    {
        *(.data)
    }

    .bss ALIGN(4K) (NOLOAD) :
    {
        bss_start = .;
        *(.bss)
        *(COMMON)
        bss_end = .;
    }

    .stack ALIGN(4K) (NOLOAD) :
    {
        . += 0x4000;
        stack_topld = .;
    }

    ldheap_start = .;
}