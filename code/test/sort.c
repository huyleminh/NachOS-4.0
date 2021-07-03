//Sort program

#include "syscall.h"

#define SIZE (100)

int A[SIZE];

int sortAscending(int a, int b)
{
    if (a > b)
        return 1;
    return 0;
}

int sortDescending(int a, int b)
{
    if (a < b)
        return 1;
    return 0;
}

void bubbleSort(int a[], int n, int (*sortOrder)(int, int))
{
    int i, j, temp;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n - 1; j++)
        {
            if (sortOrder(a[j], a[j + 1]) == 1)
            {
                temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }
}

int main()
{
    int i, j, n, type;

    PrintString("Input number of elements (<= 100): ");

    n = ReadNum();
    if (n > 100 || n <= 0)
    {
        PrintString("Invalid number of elements.\n");
        Halt();
    }
    PrintString("\n");
    for (i = 0; i < n; i++)
    {
        PrintString("Input element #");
        PrintNum(i + 1);
        PrintString(": ");
        A[i] = ReadNum();

        //2000000001 is the error case of system call SC_ReadNum
        if (A[i] == 2000000001)
        {
            PrintString("Invalid number.\n");
            Halt();
        }
        PrintString("\n");
    }

    while (1)
    {
        PrintString("Input type of sort order (1: ascending, 2: descending): ");
        type = ReadNum();
        PrintChar('\n');

        //2000000001 is the error case of system call SC_ReadNum
        if (type == 2000000001)
        {
            PrintString("Invalid number.\n");
            Halt();
        }

        //Check invalid type of sort order
        if (type == 1 || type == 2)
            break;
    }

    if (type == 1)
        bubbleSort(A, n, sortAscending);
    else
        bubbleSort(A, n, sortDescending);

    for (j = 0; j < n; j++)
    {
        PrintNum(A[j]);
        PrintString(" ");
    }

    PrintString("\n");

    Halt();
}