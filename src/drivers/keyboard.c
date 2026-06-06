#include "drivers/keyboard.h"
#include "lib/buffer.h"
#include "lib/string.h"
#include "lib/io.h"
char kbd_normal[128] =
{
    [0x01] = 27,

    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = '\b',

    [0x0F] = '\t',
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '`',
    [0x1B] = '[',
    [0x1C] = '\n',

    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = 'c',
    [0x28] = '~',

    [0x29] = '\'',
    [0x2B] = ']',

    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = ';',

    [0x37] = '*',

    [0x39] = ' ',

    [0x4A] = '-',
    [0x4E] = '+',

    [0x56] = '\\',
    [0x73] = '/',
    [0x7E] = '.'
};

char kbd_shift[128] =
{
    [0x01] = 27,

    [0x02] = '!',
    [0x03] = '@',
    [0x04] = '#',
    [0x05] = '$',
    [0x06] = '%',
    [0x07] = '"',
    [0x08] = '&',
    [0x09] = '*',
    [0x0A] = '(',
    [0x0B] = ')',
    [0x0C] = '_',
    [0x0D] = '+',
    [0x0E] = '\b',

    [0x0F] = '\t',
    [0x10] = 'Q',
    [0x11] = 'W',
    [0x12] = 'E',
    [0x13] = 'R',
    [0x14] = 'T',
    [0x15] = 'Y',
    [0x16] = 'U',
    [0x17] = 'I',
    [0x18] = 'O',
    [0x19] = 'P',
    [0x1A] = '`',
    [0x1B] = '{',
    [0x1C] = '\n',

    [0x1E] = 'A',
    [0x1F] = 'S',
    [0x20] = 'D',
    [0x21] = 'F',
    [0x22] = 'G',
    [0x23] = 'H',
    [0x24] = 'J',
    [0x25] = 'K',
    [0x26] = 'L',
    [0x27] = 'C',
    [0x28] = '^',

    [0x29] = '"',
    [0x2B] = '}',

    [0x2C] = 'Z',
    [0x2D] = 'X',
    [0x2E] = 'C',
    [0x2F] = 'V',
    [0x30] = 'B',
    [0x31] = 'N',
    [0x32] = 'M',
    [0x33] = '<',
    [0x34] = '>',
    [0x35] = ':',

    [0x37] = '*',

    [0x39] = ' ',

    [0x4A] = '-',
    [0x4E] = '+',

    [0x56] = '|',
    [0x73] = '?',
    [0x7E] = '.'
};
c_buffer keyb_buffer = {
    .head = 0,
    .tail = 0
};
unsigned char key_state[128];
uint8_t special_states;
void irq1code() {
    uint8_t scancode = inb(0x60);

    key_state[scancode & 0x7F] = !(scancode & 0x80);
    if (scancode & 0x80) {
        return;
    }
    switch (scancode) {
        case 0x46:
            special_states ^= 1;
            break;
        case 0x3A:
            special_states ^= 2;
            break;
        case 0x45:
            special_states ^= 4;
            break;
    }
    int next = (keyb_buffer.head + 1) % 128;
    if (next == keyb_buffer.tail) {return;}
    keyb_buffer.buffer[keyb_buffer.head] = scancode;
    keyb_buffer.head = next;
}
out_key waitget_key() {
    while (keyb_buffer.head == keyb_buffer.tail) {
        asm volatile ("hlt");
    }
    out_key key;
    key.scancode = keyb_buffer.buffer[keyb_buffer.tail];
    if (key_state[0x2A] || key_state[0x36]) {
        key.asciicode = kbd_shift[key.scancode];
    } else {
        key.asciicode = kbd_normal[key.scancode];
    }
    if (special_states & 2) {
        key.asciicode = captalize_letter(key.asciicode);
    }
    keyb_buffer.tail = (keyb_buffer.tail + 1) % 128;
    return key;
}