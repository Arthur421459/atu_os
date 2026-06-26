volatile char* tvideo = (volatile char*) 0xB8000;
void main() {
    tvideo[0] = 'a';
}