#include "syscall.h"

int main() {
    char ch = ReadChar();
    PrintChar(ch);
    PrintChar('\n');
    Halt();
}