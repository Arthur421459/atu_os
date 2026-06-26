[org 0600h]
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 7C00h
    copybootmbr:
        mov cx, 100h
        mov si, 0x7c00
        mov di, 0x600
        rep movsw
    jmp 0:lwinit

lwinit:
    sti
    mov byte [bootDrive], dl
    .getbootpart:
        mov bx, PT1
        mov cx, 4
        .gbplp:
            mov al, byte [bx]
            test al, 0x80
            jnz .found
            add bx, 10h
            loop .gbplp
        jmp ERROR
        .found:
            mov WORD [PToff], bx
            add bx, 8
            xor eax, eax
            mov ebx, dword [bx]
            mov [lbadap], ebx
            mov si, DAP
            mov ah, 42h
            int 13h
            jc ERROR
            xor ax, ax
            mov ah, 1
            int 13h
            cmp ah, 0
            jne ERROR
        .jmpvbr:
            cmp WORD [0x7DFE], 0xAA55
            jne ERROR
            mov si, [PToff]
            jmp 0000:7c00h

ERROR:
    int 18h
DAP:
    db 0x10
    db 0x00
    sectorsquan dw 1
    ofs dw 0x7c00
    segm dw 0
    lbadap dq 0
    
times 218 - ($-$$) db 0

DiskTimeStamp times 8 db 0
bootDrive db 0
PToff dw 0
times 0x1ba - ($-$$) db 0

UID dd 0
PT1 times 16 db 0
PT2 times 16 db 0
PT3 times 16 db 0
PT4 times 16 db 0
dw 0xAA55