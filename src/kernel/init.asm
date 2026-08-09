[bits 32]

section .kinit
global _init
global kpage_directory
_init:
    cli
    cld
    mov esp, basestack_top
    push esi ; preserve atuboot things
.setpg:
    mov eax, kpage_tablek1
    and eax, 0xFFFFF000 ; get page
    or eax, 0x401 ; kernel page dir | present
    mov [kpage_directory+3072], eax ; to 0xC0000000
    mov esi, kpage_tablek1
    mov ecx, 1024
    mov eax, 0x100
    call setpgtable

    mov eax, kpage_tablek2
    and eax, 0xFFFFF000 ; get page
    or eax, 0x401 ; kernel page dir | present
    mov [kpage_directory], eax ; to 0x0

    mov esi, kpage_tablek2
    mov ecx, 1024
    xor eax, eax
    call setpgtable
.enablepg:
    mov eax, kpage_directory
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000001 ; paging | protected mode
    mov cr0, eax
    jmp initdone
setpgtable:
    push eax
    shl eax, 12
    or eax, 0x403 ; present | kernelpg
    mov [esi], eax
    pop eax
    inc eax
    add esi, 4
    loop setpgtable
ret

align 4096
kpage_directory:
    times 1024 dd 0

kpage_tablek1:
    times 1024 dd 0

kpage_tablek2:
    times 1024 dd 0

basestack times 512 db 0
basestack_top:

section .text
extern afterkinit
extern afterpaging
global set_tss
global set_gdt
global set_idt
extern stack_top
%define offsetk (0xC0000000 - 0x100000)
extern kernel
initdone:
    mov eax, esp
    add eax, offsetk
    mov esp, eax

    call afterkinit ; let c configure everything :)
    mov esp, stack_top ; finally kernel stack!
    call afterpaging
    call kernel
    .hlt:
        jmp .hlt
set_gdt:
    mov eax, [esp+4]
    lgdt [eax] ; OMG gdt loaded lol
    jmp far 0x08:.end
.end:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

set_tss:
    mov ax, 0x28
    ltr ax
ret
set_idt:
    mov eax, [esp+4]
    lidt [eax] ; OMG idt loaded lol
    sti ; olá interrupções!!!!! (sem bios.........)
    ret