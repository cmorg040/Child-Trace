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
 |  Compile/Run:  	unzip all files.
 |					Ensure that all files, including "spawnFunctions.h" are in the same directory.
 |					Type make at command line
 | 	 	 	 	  	Type "./spawn arg1 arg2 arg3..."
 |					where args are up to seven unique integers between 1-9
 |
 |					OR
 |
 | 				  	type "gcc spawnMain.c –o spawn"
 |                	then type "./spawn arg1 arg2 arg3..."
 |  +-----------------------------------------------------------------------------
 |
 |  Description:  This program is designed to manipulate forks and facilitate communication
 |				  between independent processes via shared memory allocation.
 |
 |        Input:  This program takes a between 1-7 unique integers as input.
 |
 |       Output:  This program outputs a series of traces statements detailing
 |				  the operations being performed by each process, and their
 |				  effects on shared memory.
 |
 |     Process:
 |				1. Validation: Validates that 1-7 unique integers between 0-9 have been input.
 |				2. Allocate shared memory
 |				3. Attach a pointer to shared memory.
 |				3a. Initialize shared memory with validated cmd input.
 |				4. Create a Child struct, and intialize it with the shared memory pointer.
 |				5. Use a for loop to spawn an appropriate number of children
 |				6. Each child accesses it's 'private' location in shared memory via its unique ID
 |				7. Each child doubles, then multiplies the value at its own shared mem location.
 |				8. Each child detaches from shared memory, then exits.
 |				9. The parent waits for each child, and stores their exit code and PID in a 2d array.
 |				10. Parent prints the stores child information.
 |				11. Parent detaches from shared memory.
 |
 |
 |   Required Features Not Included:
 |					1. Detaches but does not remove shared memory.
 |					1a. Calling free() on the detached memory pointer causes a segfault.
 |
 |
 |   Known Bugs:
 |					Some trace statements run simultaneously and are confusing in output.
 |
 |  *===========================================================================*/
#include "spawnFunctions.h"

int main(int argc, char *argv[])
{
	if (validateInput(argc, argv) != VALIDATION_SUCCESS)
	{
		return VALIDATION_FAILURE;
	}

	char write_buffer[BUF_SIZE];
	const int NUM_KIDS = (argc - 1);
	int childData[(NUM_KIDS)][CHILD_DATA_FIELDS];
	int *sharedMemPtr = malloc(sizeof *sharedMemPtr);
	pid_t parent_id = getpid();

	sharedMemPtr = allocateShMem(argc);

	Child child;
	Child *childPtr = &child;
	child.numChildren = NUM_KIDS;
	child.shMemPtr = sharedMemPtr;

	sprintf(write_buffer, "Initial State of Shared Memory\n");
	write(STDOUT_FD, write_buffer, strlen(write_buffer));

	printShMem(childPtr);
	initializeShMem(sharedMemPtr, argv, argc);

	sprintf(write_buffer, "Shared Memory after initialization\n");
	write(STDOUT_FD, write_buffer, strlen(write_buffer));

	printShMem(childPtr);

	if (spawnKids(parent_id, childPtr) != SPAWN_SUCCESS)
	{
		return SPAWN_FAILURE;
	}

	storeChildData(parent_id, argc, childData);

	if (getpid() != parent_id)
	{
		childLabor(childPtr);
		exit(CHILD_SUCCESS);
	}
	printChildData(NUM_KIDS, childData);

	sprintf(write_buffer, "Parent: Detaching Shared Memory\n");
	write(STDOUT_FD, write_buffer, strlen(write_buffer));
	shmdt(sharedMemPtr);
	puts("End of program");
	return EXIT_SUCCESS;

}

/*---------------------------- validateInput ---------------------
 |  Function validateInput
 |
 |  Purpose:  Validates the command line input.
 |
 |
 |  @param  int numArgs: the number of arguments from command line input
 |			char *cmdinput[]: the user input from command line
 |
 |
 |	Note: atoi will cast any non-integers to zero,
 |		  which will be flagged as invalid input.
 |
 |  @return  Returns FUNCTION SUCCESS if validation is successful,
 |			 Returns FUNCTION FAILURE otherwise.
 |
 *-------------------------------------------------------------------*/
int validateInput(int numArgs, char *cmdInput[])
{
	printf("numArgs: %d\n", numArgs);
	if (numArgs < MIN_ARGS || numArgs > MAX_ARGS)
	{
		printf(
				"Error, invalid number of arguments. Please input %d-%d integers.\n",
				(MIN_ARGS - 1), (MAX_ARGS - 1));
		return VALIDATION_FAILURE;
	}
	int argsIterator = 0;
	char currentArg[BUF_SIZE];
	int uniqueChecker[UNIQUE_INT_RANGE] = { 0 };

	//argsIterator is initialized to 1 because the 0th element of argv is the execution command.
	for (argsIterator = 1; argsIterator < numArgs; argsIterator++)
	{
		strcpy(currentArg, cmdInput[argsIterator]);
		if (currentArg[SECOND_CHAR] != END_OF_STRING)
		{
			printf("Error, invalid input.\n"
					"Please input integers between %c - %c.\n",
			INPUT_MIN_ASCII, INPUT_MAX_ASCII);
			return VALIDATION_FAILURE;
		}

		//we add one to the INPUT_MIN_ASCII because there is already a special case handling zero.
		if (currentArg[FIRST_CHAR]
				< (INPUT_MIN_ASCII + 1)|| currentArg[FIRST_CHAR] > INPUT_MAX_ASCII)
		{
			printf("Error, input #%d is not an integer.\n"
					"Please input integers between %c - %c.\n", argsIterator,
			INPUT_MIN_ASCII, INPUT_MAX_ASCII);

			return VALIDATION_FAILURE;
		}
		//check uniqueness of input.
		//If an index has already been incremented, the current argument is not unique.
		if (uniqueChecker[atoi(currentArg)] == 0)
		{
			uniqueChecker[atoi(currentArg)]++;
		}
		else
		{
			printf("Error, duplicate input detected.\n"
					"Please input unique integers between %c - %c.\n",
			INPUT_MIN_ASCII, INPUT_MAX_ASCII);

			return VALIDATION_FAILURE;
		}

	}
	return VALIDATION_SUCCESS;
}

/*---------------------------- initializeShMem ---------------------
 |  Function allocateShMem
 |
 |  Purpose:  Allocates a segment of shared memory.
 |
 |
 |  @param  int numArgs: the number of arguments from command line input
 |
 |
 |  @return  Returns a pointer to the shared memory space.
 |
 *-------------------------------------------------------------------*/
int *allocateShMem(int numArgs)
{
	puts("Requesting shared memory.");

	int sharedMemID;
	/* shmget(key, size, flag)
	 * IPC_PRIVATE is the key for the shared memory segment.
	 * numArgs * sizeof(int) allocates space equal to the number of arguments multiplied by sizeof(int)
	 * IPC_CREAT | 0666 grants server level read/write permission.
	 * Just '0666' would be used to grant client permissions.
	 * In this case, server = parent, and client = child.
	 */
	sharedMemID = shmget(IPC_PRIVATE, numArgs * sizeof(int), IPC_CREAT | 0666);

	if (sharedMemID < 0)
	{
		puts("Error getting shared memory ID. Terminating program.");
		exit(ALLOCATION_FAILURE); //FIXME: Don't use an exit().
		//exit is used here because the function must return a pointer, not a return code.
	}

	return attachSharedMem(sharedMemID);
}

/*---------------------------- attachSharedMem ---------------------
 |  Function attachSharedMem
 |
 |  Purpose:  Attaches a shared memory ID to a pointer.
 |
 |	Source: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/process.html
 |
 |  @param  int memID: The ID of a shared memory location
 |
 |
 |  @return  int *sharedMemPtr: a pointer to the input shared memory location.
 |
 *-------------------------------------------------------------------*/
int *attachSharedMem(int memID)
{
	puts("Attaching shared memory.");
	int *sharedMemPtr = malloc(sizeof *sharedMemPtr);

	/* Attaches a void type pointer to the shared memory that we just created.
	 * The child processes will use this pointer to access the shared memory space.
	 * Void pointers are hard to work with, so we cast it to an integer pointer.
	 */

	sharedMemPtr = (int *) shmat(memID, NULL, 0);
	return sharedMemPtr;
}

/*---------------------------- detachSharedMem ---------------------
 |  Function detachSharedMem
 |
 |  Purpose:  Detaches a shared memory location from a Child object.
 |
 |	Source: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/process.html
 |
 |  @param  Child *Slave: A pointer to a child object to detach from.
 |
 |
 |  @return  none
 |
 *-------------------------------------------------------------------*/
void detachSharedMem(Child *slave)
{
	char write_buffer[BUF_SIZE];
	sprintf(write_buffer, "%*sChild: %d Detaching Shared Memory\n",
			((slave->uniqueID * CHILD_INDENT)), "", slave->uniqueID);
	write(STDOUT_FD, write_buffer, strlen(write_buffer));
	shmdt(slave->shMemPtr);
}

/*---------------------------- initializeShMem ---------------------
 |  Function initializeShMem
 |
 |  Purpose:  Initializes the shared memory space with user input.
 |
 |	Source: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/process.html
 |
 |  @param  int *shMemPtr: a pointer to the shared memory space
 |			char *cmdInput[]: the users input from command line
 |			int numArgs: the number of user arguments.
 |
 |
 |  @return  None
 |
 *-------------------------------------------------------------------*/
void initializeShMem(int *shMemPtr, char *cmdInput[], int numArgs)
{
	int shMemIterator = 0;
	for (shMemIterator = 1; shMemIterator < numArgs; shMemIterator++)
	{
		shMemPtr[shMemIterator] = atoi(cmdInput[shMemIterator]);
	}
}

/*---------------------------- printShMem ---------------------
 |  Function printShMem
 |
 |  Purpose:  Prints the shared memory space in its current state.
 |
 |
 |  @param  Child *slave: A pointer to a Child structure.
 |
 |
 |  @return  None.
 |
 *-------------------------------------------------------------------*/
void printShMem(Child *slave)
{
	char write_buf[BUF_SIZE];

	int shMemIter = 0;
	if (slave->uniqueID == PARENT_ID)
	{
		for (shMemIter = 0; shMemIter <= slave->numChildren; shMemIter++)
		{
			sprintf(write_buf, "Parent: sharedMem[%d] = %d\n", shMemIter,
					slave->shMemPtr[shMemIter]);
			write(STDOUT_FD, write_buf, strlen(write_buf));
		}
	}
	else
	{
		for (shMemIter = 0; shMemIter <= slave->numChildren; shMemIter++)
		{
			sprintf(write_buf, "%*sChild: %d sharedMem[%d] = %d\n",
					((slave->uniqueID * CHILD_INDENT)), "", slave->uniqueID,
					shMemIter, slave->shMemPtr[shMemIter]);
			write(STDOUT_FD, write_buf, strlen(write_buf));
		}
	}
}

/*---------------------------- spawnKids ---------------------
 |  Function spawnKids
 |
 |  Purpose:  spawns child processes
 |
 |
 |  @param  pid_t parent_id: the parent process ID, used to restrict spawning priveleges.
 |			Child *slave: A pointer to a Child structure.
 |
 |
 |  @return  Returns a success or failure code.
 |
 *-------------------------------------------------------------------*/
int spawnKids(pid_t parent_id, Child *slave)
{
	char write_buffer[BUF_SIZE];

	pid_t current_pid = 0;
	int kidsSpawned = 0;

	sprintf(write_buffer, "Parent: Spawning children.\n");
	write(STDOUT_FD, write_buffer, strlen(write_buffer));

	for (kidsSpawned = 1; kidsSpawned <= slave->numChildren; kidsSpawned++)
	{
		current_pid = getpid();
		if (current_pid == parent_id) //only parent will call fork()
		{
			if (fork() < 0)
			{
				return SPAWN_FAILURE;
			}
			slave->uniqueID = kidsSpawned;
		}
	}
	return SPAWN_SUCCESS;
}

/*---------------------------- childLabor ---------------------
 |  Function childLabor
 |
 |  Purpose:  Performs child process tasks.
 |
 |
 |  @param  Child *slave: A pointer to a child structure.
 |
 |
 |  @return  none
 |
 *-------------------------------------------------------------------*/

void childLabor(Child *slave)
{
	char write_buffer[BUF_SIZE];
	sprintf(write_buffer, "%*sChild: %d Initialized\n",
			((slave->uniqueID * CHILD_INDENT)), "", slave->uniqueID);
	write(STDOUT_FD, write_buffer, strlen(write_buffer));

	sprintf(write_buffer, "%*sChild: %d My unique ID is: %d\n",
			((slave->uniqueID * CHILD_INDENT)), "", slave->uniqueID,
			slave->uniqueID);
	write(STDOUT_FD, write_buffer, strlen(write_buffer));

	sprintf(write_buffer, "%*sChild: %d Initial state of Shared Memory:\n",
			((slave->uniqueID * CHILD_INDENT)), "", slave->uniqueID);
	write(STDOUT_FD, write_buffer, strlen(write_buffer));

	printShMem(slave);
	doubleIndex(slave);
	multiplyIndex(slave);

	sprintf(write_buffer, "%*sChild: %d Final state of Shared Memory:\n",
			((slave->uniqueID * CHILD_INDENT)), "", slave->uniqueID);
	write(STDOUT_FD, write_buffer, strlen(write_buffer));

	printShMem(slave);
	detachSharedMem(slave);
}

/*---------------------------- doubleIndex ---------------------
 |  Function doubleIndex
 |
 |  Purpose:  Doubles the value of a child's unique location in shared memory.
 |
 |
 |  @param  Child *slave: A pointer to a child structure.
 |
 |
 |  @return  none
 |
 *-------------------------------------------------------------------*/

void doubleIndex(Child *slave)
{
	char write_buffer[BUF_SIZE];
	sprintf(write_buffer, "%*sChild: %d Doubling Index\n",
			((slave->uniqueID * CHILD_INDENT)), "", slave->uniqueID);
	write(STDOUT_FD, write_buffer, strlen(write_buffer));
	slave->shMemPtr[slave->uniqueID] *= DOUBLE;
}

/*---------------------------- multiplyIndex ---------------------
 |  Function multiplyIndex
 |
 |  Purpose:  Multiplies the value of a child's unique location in shared memory
 |			  by its index.
 |
 |
 |  @param  Child *slave: A pointer to a child structure.
 |
 |
 |  @return  none
 |
 *-------------------------------------------------------------------*/

void multiplyIndex(Child *slave)
{
	char write_buffer[BUF_SIZE];
	sprintf(write_buffer, "%*sChild: %d Multiplying Index\n",
			((slave->uniqueID * CHILD_INDENT)), "", slave->uniqueID);
	write(STDOUT_FD, write_buffer, strlen(write_buffer));
	slave->shMemPtr[slave->uniqueID] *= slave->uniqueID;
}
/*---------------------------- storeChildData ---------------------
 |  Function storeChildData
 |
 |  Purpose:  Stores a child's exit code and pid into an array.
 |
 |
 |  @param  pid_t parent id: The parent process's ID.
 |	@param	int numArgs: The number of command line arguments.
 |	@param	int childData[][] The array in which to store child info.
 |
 |
 |  @return  none
 |
 *-------------------------------------------------------------------*/
void storeChildData(pid_t parent_id, int numArgs,
		int childData[][CHILD_DATA_FIELDS])
{
	int status = 0;
	pid_t child_pid = 0;
	if (getpid() == parent_id)
	{
		int childIter = 0;
		for (childIter = 0; childIter < (numArgs); childIter++)
		{
			child_pid = wait(&status);
			childData[childIter][CHILD_PID] = child_pid;
			childData[childIter][CHILD_EXIT_CODE] = status;
		}
	}
}
/*---------------------------- printChildData ---------------------
 |  Function printChildData
 |
 |  Purpose:  Stores a child's exit code and pid into an array.
 |
 |
 |  @param  pid_t parent id: The parent process's ID.
 |	@param	int numArgs: The number of command line arguments.
 |	@param	int childData[][] The array in which to store child info.
 |
 |
 |  @return  none
 |
 *-------------------------------------------------------------------*/
void printChildData(int numChildren, int childData[][CHILD_DATA_FIELDS])
{
	char write_buf[BUF_SIZE];

	int childIter = 0;
	for (childIter = 0; childIter < numChildren; childIter++)
	{
		sprintf(write_buf, "Child PID: %d\n", childData[childIter][CHILD_PID]);
		write(STDOUT_FD, write_buf, strlen(write_buf));
		sprintf(write_buf, "Child Exit Code: %d\n\n",
				childData[childIter][CHILD_EXIT_CODE]);
		write(STDOUT_FD, write_buf, strlen(write_buf));
	}
}
