// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchconsole.h"
#include "main.h"
#include "synchconsole.h"
#include "syscall.h"
#include "ksyscall.h"
#include <stdlib.h>
#include <time.h>

#define MAX_CHAR_ARRAY 255
#define MaxFileLength 32

int numThreads = 0;

char *User2System(int virtAddr, int limit)
{
	int oneChar;
	char *kernelBuffer = NULL;
	kernelBuffer = new char[limit + 1];
	if (kernelBuffer == NULL)
		return kernelBuffer;

	memset(kernelBuffer, 0, limit + 1);

	for (int i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuffer[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuffer;
}

int System2User(int virtAddr, int length, char *buffer)
{
	if (length < 0)
		return -1;
	if (length == 0)
		return 0;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < length && oneChar != 0);
	return i;
}

void IncreasePC()
{
	/* set previous program counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set program counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	/* set next program counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
// ----------------------------------------------------------------------
void addrSpaceExecute(AddrSpace *space) {
    space->Execute();   // run the program
}

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case NoException:
	{
		return;
	}
	case PageFaultException:
	{
		DEBUG(dbgFile, "\nNo valid translation found");
		SysHalt();
		break;
	}
	case ReadOnlyException:
	{
		DEBUG(dbgFile, "\nWrite attemped to page marked read-only");
		SysHalt();
		break;
	}
	case BusErrorException:
	{
		DEBUG(dbgFile, "\nTranslation resulted invalid physical address");
		SysHalt();
		break;
	}
	case AddressErrorException:
	{
		DEBUG(dbgFile, "\nUnaligned reference or one that was beyond the end of the address space");
		SysHalt();
		break;
	}
	case OverflowException:
	{
		DEBUG(dbgFile, "\nInteger overflow in add or sub");
		SysHalt();
		break;
	}
	case IllegalInstrException:
	{
		DEBUG(dbgFile, "\nUnimplemented or reserved instr");
		SysHalt();
		break;
	}
	case NumExceptionTypes:
	{
		DEBUG(dbgFile, "\nNumber exception types");
		SysHalt();
		break;
	}
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
		{
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
			SysHalt();
			ASSERTNOTREACHED();
			break;
		}
		case SC_Add:
		{
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							(int)kernel->machine->ReadRegister(5)); /* int op2 */
			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);
			/* Modify return point */
			IncreasePC();
			cout << "Result is " << result << endl;
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Exec:
		{
			DEBUG(dbgFile, "\nSC_PrintString");

			DEBUG(dbgFile, "\nReading virtual address of buffer data");
			int virtAddr = kernel->machine->ReadRegister(4); // first argument

			char *userProgName = User2System(virtAddr, MAX_CHAR_ARRAY);	
			char buf[255];
			bzero(buf, 255);
			sprintf(buf, "p%d", numThreads);
			Thread *mythread = new Thread(buf);
			mythread->pId = numThreads++;

			if (userProgName != NULL) {
				AddrSpace *space = new AddrSpace;
				ASSERT(space != (AddrSpace *)NULL);
				mythread->space = space;

				// load the program into the space
				if (space->Load(userProgName)) {
					mythread->Fork((VoidFunctionPtr) addrSpaceExecute, (void *)space);
					kernel->machine->WriteRegister(2, mythread->pId);
				}
			}

			delete[] userProgName;
			IncreasePC();
			return;
		}
		case SC_Exit:
		{
			DEBUG(dbgAddr, "Program exit\n");
			int re = kernel->machine->ReadRegister(4);
			// cout << "Return value" << re << endl;
			kernel->currentThread->Finish();
			break;
		}
		case SC_Close:
		{
			int id = kernel->machine->ReadRegister(4);
			if (id >= 0 && id <= 14)
			{
				//FileSystem *fileSystem = new FileSystem();
				if (kernel->fileSystem->Open((char *)id))
				{
					delete kernel->fileSystem->Open((char *)id);
					//kernel->fileSystem->Open((char*)id) = NULL;
					kernel->machine->WriteRegister(2, 0);
					break;
				}
			}
			kernel->machine->WriteRegister(2, -1);
			break;
		}
		case SC_Create:
		{
			int virtAddr;
			char *filename;
			DEBUG(dbgSys, "\nSC_Create call...");
			DEBUG(dbgSys, "\nReading virtual address of filename");
			virtAddr = kernel->machine->ReadRegister(4);
			DEBUG(dbgSys, "\nReading filename");
			filename = User2System(virtAddr, MaxFileLength + 1); //max length = 32
			if (strlen(filename) == 0)
			{
				DEBUG(dbgSys, "\nFile name is not valid");
				kernel->machine->WriteRegister(2, -1);
				break;
			}
			if (filename == NULL)
			{
				DEBUG(dbgSys, "\nNot enough memory in system");
				kernel->machine->WriteRegister(2, -1);
				delete filename;
				break;
			}
			DEBUG(dbgSys, "\nFinish reading filename.");
			if (!kernel->fileSystem->Create(filename, 0))
			{
				kernel->machine->WriteRegister(2, -1);
				delete filename;
				break;
			}
			kernel->machine->WriteRegister(2, 0);
			delete filename;
			break;
		}

		//Start custom systemcall
		case SC_ReadString:
		{
			DEBUG(dbgFile, "\nSC_ReadString");

			DEBUG(dbgFile, "\nReading virtual address of buffer data");
			int addressBuffer = kernel->machine->ReadRegister(4); // first agrument

			DEBUG(dbgFile, "\nReading length of string");
			int length = kernel->machine->ReadRegister(5); // second argument

			// copy string from user space to system space
			char *kernelBuffer = User2System(addressBuffer, length);

			// Read string into buffer.
			int realLength = kernel->synchConsoleIn->Read(kernelBuffer, length);

			// copy string from system space to user space
			System2User(addressBuffer, realLength + 1, kernelBuffer);

			delete[] kernelBuffer;
			kernelBuffer = NULL;

			IncreasePC();
			return;
			// ASSERTNOTREACHED();
			// break;
		}
		case SC_PrintString:
		{
			DEBUG(dbgFile, "\nSC_PrintString");

			DEBUG(dbgFile, "\nReading virtual address of buffer data");
			int virtAddr = kernel->machine->ReadRegister(4); // first argument

			// copy string from user space to system space
			// 255 is max number of characters in char[]
			char *kernelBuffer = User2System(virtAddr, MAX_CHAR_ARRAY);

			// print the string on the screen
			kernel->synchConsoleOut->Print(kernelBuffer);

			delete[] kernelBuffer;
			kernelBuffer = NULL;

			IncreasePC();
			return;
			// ASSERTNOTREACHED();
			// break;
		}

		case SC_ReadChar:
		{
			//Array of buffers to save all input from user
			char *buffers = new char[MAX_CHAR_ARRAY + 1];

			int i = 0;
			while (true)
			{
				char inputCh = kernel->synchConsoleIn->GetChar();
				buffers[i++] = inputCh;

				//In case user press Enter, break
				if (inputCh == '\n')
					break;
				if (i == MAX_CHAR_ARRAY)
					break;
			}

			//Put the first character into register
			kernel->machine->WriteRegister(2, (int)buffers[0]);
			IncreasePC();
			delete[] buffers;

			return;
		}

		case SC_PrintChar:
		{
			//Read the first argument
			char ch = (char)kernel->machine->ReadRegister(4);

			//Print to the console
			kernel->synchConsoleOut->PutChar(ch);
			IncreasePC();

			return;
		}

		case SC_ReadNum:
		{
			char *buffer;
			int MAX_BUFFER = 11;
			buffer = new char[MAX_BUFFER + 1];
			bool overflowFlag = false;
			int temp = 0;
			while (true)
			{
				char numbytes = kernel->synchConsoleIn->GetChar();

				if (temp == MAX_BUFFER)
				{
					if (buffer[0] == '-')
					{
						if (buffer[1] == '2')
						{
							for (int i = 2; i < temp; i++)
								if (buffer[i] != '0')
								{
									overflowFlag = true;
									break;
								}
						}
						else if (buffer[1] > '2')
							overflowFlag = true;
					}
					else
						overflowFlag = true;
				}
				else if (temp > MAX_BUFFER)
					overflowFlag = true;
				if (overflowFlag)
				{
					DEBUG(dbgSys, "\nOverflow");
					kernel->machine->WriteRegister(2, 2000000001);

					IncreasePC();
					delete[] buffer;
					return;
				}
				if (numbytes == '\n')
					break;
				buffer[temp] = numbytes;
				temp++;
			}

			int number = 0;

			// check negative
			bool isNegative = false;
			int firstNumIndex = 0;
			int lastNumIndex = 0;
			if (buffer[0] == '-')
			{
				isNegative = true;
				firstNumIndex = 1;
				lastNumIndex = 1;
			}

			// check each char in array
			for (int i = firstNumIndex; i < temp; i++)
			{
				if (buffer[i] == '.') // accept xxx.00000 is integer
				{
					int j = i + 1;
					for (; j < temp; j++)
					{
						if (buffer[j] != '0')
						{
							DEBUG(dbgSys, "\nThe integer number is not valid");
							kernel->machine->WriteRegister(2, 2000000001);

							IncreasePC();
							delete[] buffer;
							return;
						}
					}
					lastNumIndex = i - 1;
					break;
				}
				else if (buffer[i] < '0' || buffer[i] > '9')
				{
					DEBUG(dbgSys, "\nThe integer number is not valid");
					kernel->machine->WriteRegister(2, 2000000001);

					IncreasePC();
					delete[] buffer;
					return;
				}
				lastNumIndex = i;
			}

			//convert to integer
			for (int i = firstNumIndex; i <= lastNumIndex; i++)
			{
				number = number * 10 + (int)(buffer[i] - 48);
			}
			if (isNegative)
			{
				number = number * -1;
			}

			kernel->machine->WriteRegister(2, number);

			IncreasePC();
			delete[] buffer;
			return;
		}
		case SC_PrintNum:
		{
			int number = kernel->machine->ReadRegister(4);
			if (number == 0)
			{
				kernel->synchConsoleOut->PutChar('0');

				IncreasePC();
				return;
			}

			bool isNegative = false;
			int numberOfNum = 0; //number of digits
			int firstNumIndex = 0;

			//check negative for output
			if (number < 0)
			{
				isNegative = true;
				number = number * -1;
				firstNumIndex = 1;
			}

			//count number of digits
			int temp = number;
			while (temp != 0)
			{
				numberOfNum++;
				temp /= 10;
			}

			char *buffer;
			int MAX_BUFFER = 11;
			buffer = new char[MAX_BUFFER + 1];
			for (int i = firstNumIndex + numberOfNum - 1; i >= firstNumIndex; i--)
			{
				buffer[i] = (char)((number % 10) + 48);
				number /= 10;
			}
			if (isNegative)
			{
				buffer[0] = '-';
				for (int i = 0; i <= numberOfNum; i++)
				{
					kernel->synchConsoleOut->PutChar(buffer[i]);
				}

				delete[] buffer;

				IncreasePC();
				return;
			}
			for (int i = 0; i < numberOfNum; i++)
			{
				kernel->synchConsoleOut->PutChar(buffer[i]);
			}

			delete[] buffer;

			IncreasePC();
			return;
		}

		case SC_RandomNum:
		{
			srand(time(NULL));
			int randomNumber = rand();

			// count the number of digits of random number.
			int countDigits = 0;
			int cloneRandomNumber = randomNumber;
			while (cloneRandomNumber != 0)
			{
				++countDigits;
				cloneRandomNumber /= 10;
			}

			char *buffer = new char[countDigits + 1];
			memset(buffer, 0, countDigits + 1);

			// copy each character of random number to char array.
			cloneRandomNumber = randomNumber;
			for (int i = countDigits - 1; i > -1; i--)
			{
				buffer[i] = (cloneRandomNumber % 10) + '0';
				cloneRandomNumber /= 10;
			}

			// print random number.
			kernel->synchConsoleOut->Print(buffer);
			delete[] buffer;

			IncreasePC();
			return;
		}

			//End--------------------------------------------

		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}
	ASSERTNOTREACHED();
}
