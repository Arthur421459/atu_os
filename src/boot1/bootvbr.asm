[ORG 0x7C00]
jmp short 0x34
times 3 db 0
block dw 0
dw 0
boot2sinb dw 0
times 43 db 0
cld
cli
mov ax, cs
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7BFF
sti
push si
push dx
mov bx, 512
mov ax, [block]
xor dx, dx
div bx
mov bx, ax
mov ax, [boot2sinb]
mul bx
mov [sectorsquan], ax
add si, 8
mov ebx, dword [si]
add ebx, 2
mov [lbadap], ebx
xor ax, ax
mov ah, 42h
mov si, DAP
pop dx
int 13h
jc err
pop si
jmp 0000:0x8000
err:
    pop si
    .lp:
        mov ax, 1234h
        jmp .lp

DAP:
    db 0x10
    db 0x00
    sectorsquan dw 1
    ofs dw 0x8000
    segm dw 0
    lbadap dq 2
times 510-($-$$) db 0
dw 0xAA55