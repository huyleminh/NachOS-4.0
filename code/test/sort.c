#include "syscall.h"

#define SIZE (100)

int A[SIZE];

int sortAscending(int a, int b) {
    if (a > b)
        return 1;
    return 0;
}

int sortDescending(int a, int b) {
    if (a < b)
        return 1;
    return 0;
}

void bubbleSort(int a[], int n, int(*sortOrder)(int, int)) {
    int i, j, temp;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n - 1; j++) {
	   if (sortOrder(a[j], a[j + 1]) == 1) {
	      temp = a[j];
	      a[j] = a[j + 1];
	      a[j + 1] = temp;
    	   }
        }
    }
}

int main()
{
    int i, j, n;

    PrintString("Input number of elements (<= 100): ");

    n = ReadNum();
    if (n > 100) {
        Halt();
    }
    PrintString("\n");
    for (i = 0; i < n; i++) {
        PrintString("Input element #");
        PrintNum(i + 1);
        PrintString(": ");
        A[i] = ReadNum();
        PrintString("\n");
    }

    bubbleSort(A, n, sortAscending);

    for (j = 0; j < n; j++) {
        PrintNum(A[j]);
        PrintString(" ");
    }

    PrintString("\n");

    Halt();
}