[bits 32]
global _start
extern main
extern syscall_init
_start:
    call syscall_init
    call main
    .a:
        jmp .a
