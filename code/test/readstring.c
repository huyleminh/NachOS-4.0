/* readstring.c
 *	Simple program to test whether the systemcall interface works.
 *	
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int
main()
{
  // char* buffer;
  int length = 20;
  char* buffer;

  ReadString(buffer, length);

  Halt();
  /* not reached */
}
