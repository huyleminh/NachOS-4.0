//Test system call PrintString

#include "syscall.h"

int main() {
    int length = 20;
    char buffer[length + 1];

    PrintString("Input string (max length is 20): ");
    ReadString(buffer, length);
    PrintString(buffer);
    PrintString("\n");
    Halt();
}