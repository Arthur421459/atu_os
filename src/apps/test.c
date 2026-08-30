/*
#include "atuos/core.h"
#include "atuos/time.h"

char a[12] = "Test.......";
char b[2] = "-";
char c[2] = "|";
void main(int argc, char **argv) {
    print_wpos(a, 0);
    bool a = false;
    while (true) {
        if (a) {
            print_wpos(b, 80);
        } else {
            print_wpos(c, 80);
        }
        a = !a;
        usleep(500);
    }
}*/
#include "atuos/core.h"
#include "atuos/time.h"
void main() {
    sleep(1);
    print_wpos("1 second", 0);
    sleep(1);
    print_wpos("2 seconds", 0);
    sleep(1);
    print_wpos("3 seconds", 0);
    sleep(1);
    print_wpos("4 seconds", 0);
    sleep(1);
    print_wpos("5 seconds", 0);
    sleep(55);
    print_wpos("1 minute ", 0);
    while (1);
}