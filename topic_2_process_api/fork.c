/*
 * PROGRAM: Demonstrating fork() - Process Creation in UNIX
 *
 * KEY CONCEPT: This program shows how a single process becomes TWO
 * processes using fork(). After fork() returns, both the original
 * process (parent) and the new process (child) continue executing
 * from the same point in the code - right after the fork() call.
 *
 * The key point: fork() returns different values to parent and child,
 * allowing them to take different paths through the if-else.
 */

#include <unistd.h>   /* Provides fork(), getpid() - POSIX system calls */
#include <stdio.h>    /* Provides printf(), fprintf() for output */
#include <stdlib.h>   /* Provides exit() to terminate the process */

int main()
{
    /*
     * Before fork(): Only one process is running.
     * getpid() returns the Process ID - a unique number the OS assigns
     * to identify each running process.
     */
    printf("hello (pid:%ld)\n", (long)getpid());

    /*
     * fork() does something remarkable: it creates an almost exact copy
     * of the current process. After this line:
     *   - There are now two processes running
     *   - Both have the same code, same variables (copies), same point of execution
     *   - The child gets its own address space (memory is copied, not shared)
     *
     * But fork() returns different values:
     *   - To the PARENT: returns the child's PID (a positive number)
     *   - To the CHILD:  returns 0
     *   - On failure:    returns -1 (no child created)
     *
     * pid_t is a special type for process IDs (typically an integer).
     */
    pid_t rc = fork();

    /*
     * From here on, two processes are executing this same code!
     * (unless fork failed)
     *
     * the code below runs twice, once in each process, * but 'rc' has a 
     * different value in each one.
     */

    if (rc < 0)
    {
        /*
         * FORK FAILED - Only the original process reaches here.
         * This can happen if the system is out of resources (memory,
         * process table slots, etc.)
         *
         * fprintf(stderr, ...) writes to standard error - good practice
         * for error messages since they won't be mixed with normal output.
         */
        fprintf(stderr, "fork failed\n");
        exit(1); /* Exit with status 1 to indicate an error */
    }
    else if (rc == 0)
    {
        /*
         * THE CHILD PROCESS EXECUTES THIS BLOCK
         *
         * rc == 0 means "I am the child"
         *
         * Notice: getpid() now returns a DIFFERENT number than before!
         * The child has its own unique PID assigned by the OS.
         *
         * Important: The child did NOT execute the printf("hello...") above.
         * It came into existence AT the fork() call and continues from there.
         */
        printf("child (pid:%ld)\n", (long)getpid());
    }
    else
    {
        /*
         * THE PARENT PROCESS EXECUTES THIS BLOCK
         *
         * rc > 0 means "I am the parent, and rc is my child's PID"
         *
         * The parent can use this PID to:
         *   - Wait for the child to finish (using wait())
         *   - Send signals to the child
         *   - Track which child is which (if multiple children)
         *
         * getpid() here returns the SAME value as in the first printf -
         * the parent's PID hasn't changed.
         */
        printf("parent of %ld (pid: %ld)\n", (long)rc, (long)getpid());
    }

    /*
     * BOTH processes reach here and exit.
     *
     * exit(0) terminates the process with status 0 (success).
     * Each process exits independently.
     */
    exit(0);
}