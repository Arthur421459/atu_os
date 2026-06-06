#ifndef STRING_H
#define STRING_H
static inline int cmpstr(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b)
            return 0;
        a++;
        b++;
    }
    return (*a == *b);
}
char captalize_letter(unsigned char letter) {
    if ((letter >= 'A' && letter <= 'Z') || (letter >= 'a' && letter <= 'z')) {
        return letter ^ 0x20;
    } else {
        return letter;
    }
}
#endif
