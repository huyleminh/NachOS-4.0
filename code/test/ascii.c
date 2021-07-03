//Ascii table program
//Print all printable ascii character from 32 to 126

#include "syscall.h"

int main()
{
    int i, count;
    PrintString("Ascii Table:\n");

    count = 0;
    for (i = 32; i < 127; i++)
    {
        PrintNum(i);
        PrintChar('\t');
        if (i == 32)
            PrintString("Space");
        else
            PrintChar((char)i);
        count++;

        if (count == 8) {
            PrintChar('\n');
            count = 0;
        }
        else PrintChar('\t');
    }

    PrintChar('\n');

    Halt();
}
