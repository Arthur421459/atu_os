#include "kernel/idt.h"
#include "lib/string.h"
#include "kernel/gdt.h"
struct idt_entry idt[256];
struct idt_ptr itr;

void set_interrupt_idt(int i, uint32_t offset, uint8_t attributes, uint16_t selector) {
    idt[i].low_offset = offset & 0xFFFF;
    idt[i].high_offset = (offset >> 16) & 0xFFFF;
    
    idt[i].attributes = attributes;
    idt[i].selector = selector;

    idt[i].reserved_zero = 0; // nunca se sabe ne?
}

char *idtmensagens[] = {"Voce ja estudou matematica na escola?", "Verifique se voce quebrou o sistema", "/kill cpu",
"Verifique se voce quebrou o sistema parte 2", "Temos apenas 32 bits... E VOCE QUER MAIS", "Num deixo nao", "Que computador ruim e esse?",
"Eu odeio numeros fracionarios", 0, 0, "Meus parabens! voce fez uma tss invalida! :DDDD",
"Wow! um segmento invalido!", "Wow! um segmento invalido na pilha!", "Voce nao tem poder aqui", "Nunca confie em ponteiros...", 0,
"Voce ja estudou fracao na escola?", "Aprenda a contar com um computador", "O cara se superou", "Voce ja estudou fracao na escola?",
"tome cuidado com simulacoes...", "ret ou iret fracassado com sucesso!"};



extern void print_wpos(const char* str, int color, int pos);

char numstr[3];
char errstr[11];
char cr2str[11];

extern uintptr_t stack_top;

void int_handler(struct int_stack* stack) {
    print_wpos("Kernel Panic!", 0x07, 0);
    print_wpos("Int num:", 0x07, 80);
    print_wpos("Err code:", 0x07, 160);
    print_wpos("cr2:", 0x07, 240);
    print_wpos("Uma mensagem do kernel:", 0x07, 400);
    print_wpos(idtmensagens[stack->num], 0x07, 480);

    num_to_str(stack->num, numstr);
    print_wpos(numstr, 0x07, 89);

    hex_to_str(stack->err, errstr);
    print_wpos(errstr, 0x07, 170);

    uintptr_t cr2value;
    asm volatile ("mov %%cr2, %0" : "=r"(cr2value));

    hex_to_str(cr2value, cr2str);
    print_wpos(cr2str, 0x07, 245);

    while(1);
    ktss.esp0 = stack_top;
    ktss.ss0 = kerneldata_seg;
}

void config_idt() {
    itr.base = (uintptr_t)idt;
    itr.limit = sizeof(idt) - 1;
    for (int i = 0; i < 256; i++) {
        switch (i) {
            case 0:
                set_interrupt_idt(i, (uintptr_t)&int0, 0b10001110, 0x08);
                break;
            case 1:
                set_interrupt_idt(i, (uintptr_t)&int1, 0b10001111, 0x08);
                break;
            case 2:
                set_interrupt_idt(i, (uintptr_t)&int2, 0b10001110, 0x08);
                break;
            case 3:
                set_interrupt_idt(i, (uintptr_t)&int3, 0b10001111, 0x08);
                break;
            case 4:
                set_interrupt_idt(i, (uintptr_t)&int4, 0b10001111, 0x08);
                break;
            case 5:
                set_interrupt_idt(i, (uintptr_t)&int5, 0b10001110, 0x08);
                break;
            case 6:
                set_interrupt_idt(i, (uintptr_t)&int6, 0b10001110, 0x08);
                break;
            case 7:
                set_interrupt_idt(i, (uintptr_t)&int7, 0b10001110, 0x08);
                break;
            case 8:
                set_interrupt_idt(i, (uintptr_t)&int8, 0b10001110, 0x08);
                break;
            case 10:
                set_interrupt_idt(i, (uintptr_t)&int10, 0b10001110, 0x08);
                break;
            case 11:
                set_interrupt_idt(i, (uintptr_t)&int11, 0b10001110, 0x08);
                break;
            case 12:
                set_interrupt_idt(i, (uintptr_t)&int12, 0b10001110, 0x08);
                break;
            case 13:
                set_interrupt_idt(i, (uintptr_t)&int13, 0b10001110, 0x08);
                break;
            case 14:
                set_interrupt_idt(i, (uintptr_t)&int14, 0b10001110, 0x08);
                break;
            case 16:
                set_interrupt_idt(i, (uintptr_t)&int16, 0b10001110, 0x08);
                break;
            case 17:
                set_interrupt_idt(i, (uintptr_t)&int17, 0b10001110, 0x08);
                break;
            case 18:
                set_interrupt_idt(i, (uintptr_t)&int18, 0b10001110, 0x08);
                break;
            case 19:
                set_interrupt_idt(i, (uintptr_t)&int19, 0b10001110, 0x08);
                break;
            case 20:
                set_interrupt_idt(i, (uintptr_t)&int20, 0b10001110, 0x08);
                break;
            case 21:
                set_interrupt_idt(i, (uintptr_t)&int21, 0b10001110, 0x08);
                break;
            case 32:
                set_interrupt_idt(i, (uintptr_t)&irq0, 0b10001110, 0x08);
                break;
            case 33:
                set_interrupt_idt(i, (uintptr_t)&irq1, 0b10001110, 0x08);
                break;
            case 37:
                set_interrupt_idt(i, (uintptr_t)&irq5, 0b10001110, 0x08);
                break;
            case 44:
                set_interrupt_idt(i, (uintptr_t)&irq12, 0b10001110, 0x08);
                break;
            case 0xA7:
                set_interrupt_idt(i, (uintptr_t)&syscallint, 0b11101111, 0x08);
                break;
            default:
                if (i >= 0x20 && i < 0x28) {
                    set_interrupt_idt(i, (uintptr_t)&irqmaslabel, 0b10001110, 0x08);
                } else if (i >= 0x28 && i <= 0x2F) {
                    set_interrupt_idt(i, (uintptr_t)&irqslavelabel, 0b10001110, 0x08);
                } else {
                    set_interrupt_idt(i, (uintptr_t)&intlabel, 0b10001110, 0x08);
                }
                break;
        }
    }
    set_idt((uintptr_t)&itr);
}