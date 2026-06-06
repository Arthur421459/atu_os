[ORG 0x7c00]
jmp start
nop
db "atu os  " ; OEM ID
dw 200h ; bytes per sector
db 1 ; sectors per cluster
dw 8 ; reserved sectors
db 2 ; number of FATs
dw 200h ; max root dir entries
dw 0xFFFF ; total sectors (if zero, use largetotalsectors)
db 0xF8 ; media descriptor
dw 0x100 ; sectors per FAT
dw 63 ; sectors per track
dw 255 ; number of heads
dd 0 ; hidden sectors
dd 0 ; large total sectors
db 0x80 ; drive number
db 0 ; Windows NT flags
db 0x29 ; extended boot signature
dd 12345678h ; volume serial number
db "ATU OS    " ; volume label
db "FAT16   " ; file system type
start:
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7000
    inc ebx
    mov ah, 0
    mov al, 3
    int 10h
    mov [lbadap], ebx
    xor ebx, ebx
    call loadfatdriver
    mov ebx, [lbadap]
    dec ebx
    call far 0000:fatinit
    mov si, foldername
    xor ax, ax
    mov ds, ax
    call far 0000:fatcd
    xor ax, ax
    mov ds, ax
    mov si, kernelname
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    call far 0000:fatreadfile
    cmp cx, 1
    je ERROR
    jmp 0x1000:0000

loadfatdriver:
    mov cx, 5
    mov dl, 0x80
    mov ah, 42h
    mov si, DAP
    .lp:
        cmp cx, 0
        jle ERROR
        dec cx
        int 13h
        jc .lp
    ret
ret
testing:
    mov al, "a"
    call printchar
ret
ERROR:
    int 18h
DAP:
    db 0x10
    db 0x00
    sectorsquan dw 2
    ofs dw 0x0800
    segm dw 0
    lbadap dd 0
    lbadaph dd 0

%include 'print.inc'
fatdrvadd EQU 0x0800
fatinit EQU fatdrvadd
fatreadfile EQU fatdrvadd+2
fatcd EQU fatdrvadd+8
kernelname db 'KERNEL  BIN'
foldername db 'ATUOS      '
loadingkernel db 'Carregando kernel...',0
loaded db 'Carregado!',0

times 510-($-$$) db 0
dw 0xAA55