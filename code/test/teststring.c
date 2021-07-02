#include "syscall.h"

int main() {
    int length = 20;
    char buffer[length + 1];

    PrintString("le hoang anh");
    ReadString(buffer, length);
    PrintString(buffer);

    Halt();
}