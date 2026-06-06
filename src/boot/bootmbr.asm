[org 0600h]
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, ax
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
            mov cx, 1
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
            mov ebx, [lbadap]
            mov si, DAP
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
times 0x1b4 - ($-$$) db 0

UID times 10 db 0
PT1:
    isactive db 0x80
    chsinit db 0, 0, 4
    parttype db 0x06
    chsend db 0xFF, 0xFF, 0xFF
    lbainit db 0x04, 0, 0, 0
    lbaend db 0xFC, 0xFF, 0, 0

PT2 times 16 db 0
PT3 times 16 db 0
PT4 times 16 db 0
dw 0xAA55