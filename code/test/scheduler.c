#include "syscall.h"

int main()
{
    int pingId, pongId;
    PrintString("Ping-Pong program is running ...\n");
    pingId = Exec("./ping.c");
    pongId = Exec("./pong.c");
}