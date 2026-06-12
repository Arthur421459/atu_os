ENTRY(_start)

SECTIONS {
    . = 0x100000;
    .multiboot : {
        *(.multiboot)
    }

    .text : ALIGN(4K) {
        text_start = .;
        *(.text)
        text_end = .;
    }

    .rodata : ALIGN(4K) {
        *(.rodata)
    }

    
    .data : ALIGN(4K) { 
        data_start = .;
        *(.data)
        data_end = .;
    }

    .bss : ALIGN(4K) {
        bss_start = .;
        *(.bss)
        *(COMMON)
        bss_end = .;
    }

    .stack (NOLOAD) : {
        . = . + 0x4000;
        stack_top = .;
    }
}