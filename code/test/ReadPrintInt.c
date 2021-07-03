//Test system call ReadNum and PrintNum

#include "syscall.h"

int main()
{
    int a;
    PrintString("Input: ");
    a = ReadNum();

    if (a == 2000000001)
        PrintString("Invalid number.");
    else
        PrintNum(a);
    PrintString("\n");
    Halt();
}