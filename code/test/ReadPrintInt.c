#include "syscall.h"

int main() {
    int a = ReadNum();
    PrintNum(a);
    Halt();
}