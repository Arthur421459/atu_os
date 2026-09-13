[bits 32]
global sysentercall
sysentercall:
    push ebp
    push ebx
    push esi
    push edi

    mov eax, [esp+24]
    mov ebx, [esp+28]
    mov ecx, [esp+32]
    mov edx, [esp+36]

    mov esi, [esp+40]
    mov edi, [esp+44]

    push edx
    push ecx
    mov ecx, esp
    mov edx, .end

    sysenter ; THE INSTRUCTION
.end:
    mov ecx, [esp+28] ; we can use ecx
    mov [ecx], eax ; save eax

    pop ecx
    pop edx

    mov eax, [esp+20] ; eax is saved and we can use it
    mov [eax+4], ebx
    mov [eax+8], ecx
    mov [eax+12], edx
    mov [eax+16], esi
    mov [eax+20], edi
    pop edi
    pop esi
    pop ebx
    pop ebp
ret

