[bits 16]
extern drive
extern boot2main
extern vbe_info
global _start
_start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov [drive], dl ; salvar o drive
    ; mov ax, 0x4f02
    ; mov bx, 324
    ; int 10h
    ; mov ax, 0x4f01
    ; mov cx, 324
    ; mov di, vbe_info
    ; int 10h
    cli ; adeus interrupções.....
    in al, 0x92
    test al, 2 ; ver se a20 já foi ativado
    jnz .pass2
    or al, 2 ; caso não.....
    and al, 0xFE
    out 0x92, al
.pass2:
    lgdt [gdt_desc]
    mov eax, cr0
    or eax, 1 ; not even :|
    mov cr0, eax
    jmp 0x08:start32

gdt:
    dq 0 ; null entry
.code_seg:
    dw 0xFFFF ; limit low
    dw 0 ; base low
    db 0 ; base mid
    db 0x9A ; access
    db 0xCF ; granularity + limit high
    db 0x00 ; base high
.data_seg:
    dw 0xFFFF ; limit low
    dw 0 ; base low
    db 0 ; base mid
    db 0x92 ; access
    db 0xCF ; granularity + limit high
    db 0x00 ; base high
gdt_end:

gdt_desc:
    dw gdt_end - gdt - 1
    dd gdt
[bits 32]
start32:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    call boot2main
    jmp eax
    hlt

