; fat driver :)
[org 0x0800]
jmp fatinit
jmp readfile
jmp readinfo
jmp cd
jmp gotorootdir
fatinit:
    pusha
    push ds
    mov ax, cs
    mov ds, ax
    mov [partstart], dword ebx
    xor ebx, ebx
    mov ax, [0x7c0e]
    add ax, [partstart]
    mov [fatstart], ax
    mov ax, [0x7c16]
    mov [secperfat], ax
    mov al, [0x7c10]
    mov [fatquan], al
    mov ax, [secperfat]
    mul ax, [fatquan]
    add ax, [fatstart]
    mov [rootstart], ax
    mov [dirstart], ax
    mov ax, [0x7c11]
    mul ax, 32
    mov bx, [0x7c0b]
    dec ax
    add ax, bx
    xor dx, dx
    div bx
    mov [rootdirsectors], ax
    add ax, [rootstart]
    mov [cluster1], ax
    xor ax, ax
    mov al, [0x7c0d]
    mov [secpercluster], al
    popa
    pop ds
retf
readsector:
    
    push ds
    pusha
    mov ax, cs
    mov ds, ax

    mov cx, 5
    mov dl, 0x80
    mov ah, 42h
    mov si, readsectorDAP
    .lp:
        cmp cx, 0
        jle .err
        dec cx
        int 13h
        jc .lp
        jmp .end

    .err:
        xor ax, ax
        mov ax, 1
        int 13h
        popa
        pop ds
        ret
        .lpa:
            jmp .lpa

    .end:
        popa
        pop ds
        ret

readfile:
    pusha
    push es
    push bx
    call readrootdir
    cmp cx, 1
    je .err
    xor ax, ax
    mov ds, ax
    .readcluster:
        mov ax, [fileentry]
        mov si, ax
        add si, 11
        mov al, [si]
        test al, 0x20
        jz .err
        add si, 15
        xor eax, eax
        mov ax, word [si]
        sub ax, 2
        xor dx, dx
        mov bl, [secpercluster]
        mul bl
        add ax, [cluster1]
        mov [lbadap], eax
        add si, 2
        mov ebx, 200h
        mov eax, dword [si]
        add eax, ebx
        xor edx, edx
        dec eax
        div ebx
        mov [sectorsquan], ax
        pop bx
        pop es
        mov [segm], es
        mov [ofs], bx
        call readsector
    .end:
        xor bx, bx
        mov ax, 8000h
        mov [segm], bx
        mov [ofs], ax
        mov [sectorsquan], 1
        popa
        mov cx, 0
        mov ax, [fileentry]
        mov si, ax
        retf
    .err:
        xor bx, bx
        mov ax, 8000h
        mov [segm], bx
        mov [ofs], ax
        mov [sectorsquan], 1
        pop bx
        pop es
        popa
        mov cx, 1
        mov ax, [fileentry]
        mov si, ax
        retf
cd:
    ; si = expected file
    pusha
    call readrootdir
    cmp cx, 1
    je .err
    xor ax, ax
    mov ds, ax
    .cd1:
        mov ax, [fileentry]
        mov si, ax
        add si, 11
        mov al, [si]
        test al, 0x10
        jz .err
        add si, 15
        xor eax, eax
        mov ax, word [si]
        cmp ax, 0
        je .rd
        sub ax, 2
        xor dx, dx
        mov bl, [secpercluster]
        mul bl
        add ax, [cluster1]
        mov [dirstart], eax
    .end:
        xor bx, bx
        mov ax, 8000h
        mov [segm], bx
        mov [ofs], ax
        mov [sectorsquan], 1
        popa
        mov cx, 0
        retf
    .err:
        xor bx, bx
        mov ax, 8000h
        mov [segm], bx
        mov [ofs], ax
        mov [sectorsquan], 1
        popa
        mov cx, 1
        retf
    .rd:
        mov ebx, [rootstart]
        mov [dirstart], ebx
        jmp .end
gotorootdir:
    pusha
    push ds
    xor ax, ax
    mov ds, ax
    mov ebx, [rootstart]
    mov [dirstart], ebx
    pop ds
    popa
    retf
readrootdir:
    ; si = expected file
    ; es = sector
    ; bx = offset
    push ds
    pusha
    xor ax, ax
    mov es, ax
    mov di, rfinputstr1
    mov cx, 11
    cld
    rep movsb

    mov si, rfinputstr1
    mov ds, ax
    mov ebx, [dirstart]
    mov [lbadap], ebx

    call readsector

    xor ebx, ebx
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov si, 0x8000
    mov di, rfinputstr1

    .locateloop:
        xor ax, ax
        cmp word [si], ax
        je .errenda
        push si
        xor cx, cx
        call cmpfilename
        pop si
        cmp cx, 1
        je .reconfig
        jmp .enda
        .reconfig:
            add si, 32
            mov di, rfinputstr1
            cmp si, 8220h
            jae .rreconfig
            jmp .locateloop
        .rreconfig:
            inc [lbadap]
            call readsector
            jmp .locateloop
    .errenda:
        mov ax, si
        mov [fileentry], ax
        xor bx, bx
        mov ax, 8000h
        mov [segm], bx
        mov [ofs], ax
        mov [sectorsquan], 1
        popa
        pop ds
        mov cx, 1
        ret
    .enda:
        mov ax, si
        mov [fileentry], si
        xor bx, bx
        mov ax, 8000h
        mov [segm], bx
        mov [ofs], ax
        mov [sectorsquan], 1
        popa
        pop ds
        mov cx, 0
        ret
readinfo:
    pusha
    push ds
    push es
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ebx, [dirstart]
    mov [lbadap], ebx
    mov cx, 1
    mov [sectorsquan], 1
    .archivelp:
        cmp cx, 17
        jge .prend
        call readsector
        inc [lbadap]
        inc cx
        mov si, 1e0h
        mov al, [ds:si]
        test al, al
        jnz .archivelp
    .end:
        mov [sectorsquan], cx
        mov ebx, [dirstart]
        mov [lbadap], ebx
        call readsector
        xor bx, bx
        mov ax, 8000h
        mov [segm], bx
        mov [ofs], ax
        mov [sectorsquan], 1
        pop es
        pop ds
        popa
        retf
    .prend:
        mov cx, 16
        jmp .end
stack_broken:
    mov al, "a"
    call printchar
    jmp stack_broken
lp:
    mov al, "b"
    call printchar
    jmp lp
cmpfilename:
    xor ax, ax
    ; si = input di = correct
    mov ah, [di]
    mov al, [si]
    inc di
    inc si
    cmp cx, 11
    je .end
    inc cx
    cmp ah, al
    jne .err
    jmp cmpfilename
    .err:
        mov cx, 1
        ret
    .end:
        xor cx, cx
        ret
ret
%include "print.inc"
fileentry dw 0
rfinputstr1 times 11 db 0
partstart dw 0
rootdirsectors dw 0
cluster1 dw 0
fatstart dw 0
secperfat dw 0
secpercluster db 0
fatquan db 0
dirstart dd 0
rootstart dd 0
saved_sp dw 0

readsectorDAP:
    db 0x10
    db 0x00
    sectorsquan dw 1
    ofs dw 8000h
    segm dw 0
    lbadap dd 0
    lbadaph dd 0