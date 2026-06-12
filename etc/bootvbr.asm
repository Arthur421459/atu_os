[ORG 0x7C00]
times 5 db 0
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
push dx
mov bx, 512
mov ax, [block]
xor dx, dx
div bx
mov bx, ax
mov ax, [boot2sinb]
mul bx
pop dx
xor ax, ax
mov es, ax
mov bx, 0x8000
mov ah, 2
mov al, 5
mov dh, 0
mov ch, 0
mov cl, 3
int 13h
jc err
jmp 0000:0x8000
err:
    mov ah, 0eh
    mov al, 'a'
    int 10h
    jmp err

times 510-($-$$) db 0
dw 0xAA55