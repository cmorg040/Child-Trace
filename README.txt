This program is designed to be informative about the process of spawning child processes.
The parent process in this program allocates a space for shared memory, then spawns 1-7 child processes.
Each child process performs a 'task', with trace statements explaining each function called by the child.

The children are each assigned a unique ID, between 1-7(corresponding to the number of children spawned). 
Each unique ID corresponds to an array index for the integer array stored in shared memory.
Each child will double, then multiply by its unique ID, the value at its index in shared memory. 

Input: This program takes 1-7 unique integers in the value range 1-9. 
Repeat integers, E.G. '1, 2, 3, 3, 4' will not be accepted. 

Output: This program outputs a series of trace statements, tracking the activity of the parent and each child process. 
