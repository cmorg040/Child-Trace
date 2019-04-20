/*=============================================================================
 |   Source code:  spawnMain.c
 |           Author:  Carrel Morgan
 |     Student ID:  3759826
 |    Assignment:  Program #6
 |
 |            Course:  COP 4338
 |           Section:  U02
 |        Instructor:  William Feild
 |        Due Date:  04/18/2019, at the beginning of class
 |
 |	I hereby certify that this collective work is my own
 |	and none of it is the work of any other person or entity.
 |	______________________________________ [Signature]
 |
 |        Language:  C
 |
 |  +-----------------------------------------------------------------------------
 |
 |  Description:  This header file stores the function declarations, dependencies,
 |				  and constants required to run the source code: spawnMain.c
 |
 |  *===========================================================================*/

#include <stdio.h>		//standard input output functions
#include <stdlib.h>		//standard C library functions
#include <sys/shm.h>	//functions for shared memory allocation.
#include <sys/ipc.h>	//constants for use in allocating Shared Memory.
#include <sys/types.h>	//Allows us to use the pid_t type.
#include <unistd.h>		//allows use of some unicode functions
#include <string.h>		//allows comparison of strings and write buffers.
#include <math.h>		//enables basic math functionality.
#include <stdbool.h>	//Allows use of booleans.
#include <signal.h>		//Includes signal codes.
#include <sys/wait.h>	//Allows communicaiton between parent and child.

#define STDOUT_FD 1	//file descriptor argument for the write() function.
//Directs write() to print to screen(stdout)

#define CHILD_SUCCESS 0
#define VALIDATION_FAILURE -1
#define VALIDATION_SUCCESS 1
#define ALLOCATION_FAILURE -2
#define ALLOCATION_SUCCESS 2
#define SPAWN_FAILURE -3
#define SPAWN_SUCCESS 3

#define MIN_ARGS 2 //execution + at least one integer.
#define MAX_ARGS 8

#define INPUT_MIN_ASCII '0'
#define INPUT_MAX_ASCII '9'
#define UNIQUE_INT_RANGE 10

#define FIRST_CHAR 0
#define SECOND_CHAR 1

#define PARENT_ID 0

#define ASCII_ZERO '0'
#define ASCII_NINE '9'

#define END_OF_STRING '\0'

#define BUF_SIZE 200

#define CHILD_INDENT 8

#define CHILD_DATA_FIELDS 2

#define CHILD_PID 0
#define CHILD_EXIT_CODE 1

#define DOUBLE 2

typedef struct {
	int numChildren;
	int uniqueID;
	int *shMemPtr;
} Child;

int validateInput(int numArgs, char *cmdInput[]);
bool isInteger(char *aString);

int *allocateShMem(int numArgs);
int *attachSharedMem(int memID);
void detachSharedMem(Child *slave);
void initializeShMem(int *shMemPtr, char *cmdInput[], int numArgs);
void printShMem(Child *slave);

int spawnKids(pid_t parent_id, Child *slave);
void childLabor(Child *slave);
void doubleIndex(Child *slave);
void multiplyIndex(Child *slave);

void storeChildData(pid_t parent_id, int numArgs, int childData[][CHILD_DATA_FIELDS]);
void printChildData(int numChildren, int childData[][CHILD_DATA_FIELDS]);
