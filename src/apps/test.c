#include <atuos/core.h>
#include <atuos/time.h>

char a[] = "Test.......";
char b[] = "-";
char c[] = "|";
void main() {
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
}
