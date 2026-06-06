[bits 32]

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x0
    dd -(0x1BADB002)

section .text
global _start
global set_gdt
extern kernel
extern clear_bss
extern int_handler
extern irq_handler
extern stack_top
global int0
global irq0
global irq1
global irqmaslabel
global irqslavelabel
global set_idt
global intlabel

_start:
    cli
    cld
    mov esp, stack_top
    call clear_bss
    call kernel
    .lp: jmp .lp
set_gdt:
    mov eax, [esp+4]
    lgdt [eax] ; OMG gdt loaded lol
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp far 0x08:.end
.end:
    ret

set_idt:
    mov eax, [esp+4]
    lidt [eax] ; OMG idt loaded lol
    sti
    ret

; interrupts
int0:
    push dword 0
    jmp int_common

; irq
; int = irq + 32
irq1:
    push dword 1
    jmp irq_common

irq0:
    push dword 0
    jmp irq_common
irq_common:
    pusha
    mov ax, ds
    push ax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push [esp+34]
    call irq_handler
    add esp, 4
    pop ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 4
iretd
int_common:
    pusha
    mov ax, ds
    push ax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push [esp+34]
    call int_handler
    add esp, 4
    pop ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 4
iretd
irqmaslabel:
    pusha
    xor eax, eax
    mov al, 0x20
    out 0x20, al
    popa
iretd
irqslavelabel:
    pusha
    xor eax, eax
    mov al, 0x20
    out 0xA0, al
    out 0x20, al
    popa
iretd
intlabel:
iretd