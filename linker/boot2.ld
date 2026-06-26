ENTRY(_start)

SECTIONS
{
    . = 0x8000;

    .text : {
        build/asm/boot2/boot2.o(.text*)
        build/c/boot2/boot2.o(.text*)
    }

    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(.bss*) }
}