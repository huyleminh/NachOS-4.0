#include "syscall.h"

int main() {
    int length = 20;
    char buffer[length + 1];

    ReadString(buffer, length);
    PrintString(buffer);

    Halt();
}