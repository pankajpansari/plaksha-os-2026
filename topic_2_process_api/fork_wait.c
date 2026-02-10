/*
 * CONCEPT: Process Creation and Synchronization
 *
 * This program demonstrates two fundamental OS concepts:
 * 1. fork() - How a process creates a new (child) process
 * 2. wait() - How a parent process waits for its child to finish
 *
 * Key insight: After fork(), there are TWO processes running
 * the same code, but fork() returns different values to each,
 * allowing them to take different paths.
 */

#include <unistd.h>   /* Provides fork(), getpid() */
#include <sys/wait.h> /* Provides wait() */
#include <stdio.h>    /* Provides printf(), fprintf() */
#include <stdlib.h>   /* Provides exit() */

int main()
{
    /*
     * getpid() returns the Process ID (PID) of the calling process.
     * At this point, only ONE process exists - the original (parent).
     * This line executes exactly once.
     */
    printf("hello (pid:%ld)\n", (long)getpid());

    /*
     * fork() is the UNIX way to create a new process.
     *
     * WHAT HAPPENS: The OS creates an almost-exact copy of this process.
     * Now TWO processes exist, both about to return from fork().
     *
     * THE MAGIC: fork() returns DIFFERENT values to parent and child:
     *   - In the PARENT: returns the child's PID (a positive number)
     *   - In the CHILD:  returns 0
     *   - On failure:    returns -1 (no child created)
     *
     * This is how parent and child can execute different code paths.
     */
    pid_t rc = fork();

    if (rc < 0)
    {
        /*
         * fork() failed - no child was created.
         * This might happen if the system is out of memory
         * or has reached the maximum number of processes.
         */
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0)
    {
        /*
         * CHILD PROCESS executes this block.
         * rc == 0 tells us we are the newly created child.
         *
         * Note: getpid() here returns the CHILD's PID, which is
         * different from what the parent printed above.
         */
        printf("child (pid:%ld)\n", (long)getpid());
        exit(0); /* Child's work is done; terminate cleanly */
    }
    else
    {
        /*
         * PARENT PROCESS executes this block.
         * rc contains the child's PID (the return value of fork()).
         *
         * wait(&status) does two things:
         * 1. BLOCKS the parent until the child terminates
         * 2. Returns the PID of the terminated child
         *
         * WHY USE wait()?
         * - Ensures child finishes before parent continues
         * - Makes output order DETERMINISTIC (child always prints first)
         * - Prevents "zombie" processes (more on this later in the course)
         *
         * Without wait(), we wouldn't know if child or parent prints first!
         * The CPU scheduler would decide, making output non-deterministic.
         */
        int status; /* Will store child's exit status (not used here) */
        pid_t rc_wait = wait(&status);

        /*
         * At this point, the child has definitely finished.
         * - rc:      child's PID (from fork())
         * - rc_wait: should equal rc (the child that terminated)
         * - getpid(): parent's PID (same as the "hello" line above)
         */
        printf("parent of %ld (rc_wait: %ld) (pid: %ld)\n",
               (long)rc, (long)rc_wait, (long)getpid());
    }

    exit(0);
}