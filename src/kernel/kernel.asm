[bits 32]

; section .multiboot
; align 4
;     dd 0x1BADB002
;     dd 0x0
;     dd -(0x1BADB002)

section .text
global _start
extern kernel
extern clear_bss
extern int_handler
extern irq_handler
extern stack_top
extern binfo
extern syscall_enter
extern syscall_int

global int0
global int1
global int2
global int3
global int4
global int5
global int6
global int7
global int8
global int10
global int11
global int12
global int13
global int14
global int16
global int17
global int18
global int19
global int20
global int21

global irq0
global irq1
global irq5
global irq12

global irqmaslabel
global irqslavelabel
global intlabel

global syscallint
global syscallenter

global set_pag
global errlabel
global jmp_prog
extern retstart   
set_pag:
    mov eax, [esp+4]
    mov cr3, eax
ret

jmp_prog:
    cli
    mov ebx, [esp+4]
    mov ecx, [esp+8]
    mov eax, [esp+12]
    mov cr3, eax
    ; set data seg
    mov ax, 0x23 ; userdata seg | rpl = 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; after this, PLEASE DON'T USE POINTERS OR THINGS THAT USES RAM
    push 0x23 ; userdata seg | rpl = 3 AGAIN
    push ecx ; prog stack :)

    pushf ; flags
    pop eax ; get flags
    or eax, 0x200 ; enable int
    push eax ; flags again

    push 0x1B ; usercode seg | rpl = 3
    push ebx
    xor eax, eax
    mov ebx, eax
    mov ecx, eax

    iretd

; interrupts
int0:
    push dword 0
    push dword 0
    jmp int_common

int1:
    push dword 0
    push dword 1
    jmp int_common

int2:
    push dword 0
    push dword 2
    jmp int_common

int3:
    push dword 0
    push dword 3
    jmp int_common

int4:
    push dword 0
    push dword 4
    jmp int_common

int5:
    push dword 0
    push dword 5
    jmp int_common

int6:
    push dword 0
    push dword 6
    jmp int_common

int7:
    push dword 0
    push dword 7
    jmp int_common

int8:
    cli
    hlt

int10:
    push dword 10
    jmp int_common

int11:
    push dword 11
    jmp int_common

int12:
    push dword 12
    jmp int_common

int13:
    push dword 13
    jmp int_common

int14:
    push dword 14
    jmp int_common

int16:
    push dword 0
    push dword 16
    jmp int_common
int17:
    push dword 17
    jmp int_common

int18:
    push dword 0
    push dword 18
    jmp int_common

int19:
    push dword 0
    push dword 19
    jmp int_common

int20:
    push dword 0
    push dword 20
    jmp int_common

int21:
    push dword 21
    jmp int_common

int_common:
    push gs
    push fs
    push es
    push ds
    push ebp
    push esi
    push edi
    push edx
    push ecx
    push ebx
    push eax
    push esp ; stack pointer
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call int_handler
    add esp, 4
    pop eax
    pop ebx
    pop ecx
    pop edx
    pop edi
    pop esi
    pop ebp
    pop ds
    pop es
    pop fs
    pop gs
    add esp, 8
iretd

irq0:
    push dword 0
    jmp irq_common

; irq
; int = irq + 32
irq1:
    push dword 1
    jmp irq_common
irq12:
    push dword 12
    jmp irq_common
irq5:
    push dword 5
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
syscallenter:
    push edx ; eip
    push ecx ; esp
    push gs
    push fs
    push es
    push ds
    push ebp
    push esi
    push edi
    push ebx
    push eax
    push esp ; stack pointer
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call syscall_enter
    add esp, 4
    pop eax
    pop ebx
    pop edi
    pop esi
    pop ebp
    pop ds
    pop es
    pop fs
    pop gs
    pop ecx ; esp
    pop edx ; eip
sysexit ;NOITCURTSNI EHT 

syscallint:
    push gs
    push fs
    push es
    push ds
    push ebp
    push esi
    push edi
    push edx
    push ecx
    push ebx
    push eax
    push esp ; stack pointer
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call syscall_int
    add esp, 4
    pop eax
    pop ebx
    pop ecx
    pop edx
    pop edi
    pop esi
    pop ebp
    pop ds
    pop es
    pop fs
    pop gs
iretd
errlabel:
mov eax, 0xfab
cli
hlt
