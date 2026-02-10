/*
 * PROGRAM: fork_wait_exec_redirect.c
 *
 * PURPOSE: This program demonstrates four fundamental OS concepts:
 *   1. Process creation using fork()
 *   2. Process synchronization using wait()
 *   3. Program replacement using exec()
 *   4. I/O redirection (redirecting output to a file)
 *
 * WHAT IT DOES: Creates a child process that runs the 'wc' (word count)
 * command on this source file itself, but redirects the output to a file
 * instead of printing to the screen.
 */

#include <unistd.h>   /* Provides fork(), exec(), close(), getpid() */
#include <sys/wait.h> /* Provides wait() for parent to wait for child */
#include <stdio.h>    /* Provides printf(), fprintf() */
#include <stdlib.h>   /* Provides exit() */
#include <fcntl.h>    /* Provides open() and file control flags like O_CREAT */

int main()
{
    /*
     * Print a greeting along with our Process ID (PID).
     * Every process in the system has a unique PID.
     * getpid() returns the PID of the calling process.
     */
    printf("hello (pid:%ld)\n", (long)getpid());

    /*
     * fork() is the ONLY way to create a new process in Unix.
     *
     * WHAT HAPPENS: The OS creates an (almost) exact copy of this process.
     * After fork(), there are TWO processes running this same code!
     *
     * RETURN VALUES (this is the tricky part):
     *   - In the PARENT: fork() returns the child's PID (a positive number)
     *   - In the CHILD:  fork() returns 0
     *   - On ERROR:      fork() returns -1
     *
     * This different return value is how parent and child can do different things.
     */
    pid_t rc = fork();

    if (rc < 0)
    {
        /*
         * fork() failed - this is rare but possible (e.g., system out of memory).
         * fprintf to stderr ensures error messages aren't lost if stdout is redirected.
         */
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0)
    {
        /*
         * ========== CHILD PROCESS CODE ==========
         *
         * We know we're the child because fork() returned 0.
         * The child will run the 'wc' command, but first we set up I/O redirection.
         */

        /*
         * I/O REDIRECTION TRICK - This is the clever part!
         *
         * Step 1: Close standard output (file descriptor 1).
         * STDOUT_FILENO is a constant equal to 1.
         *
         * After this, the child process has no stdout - file descriptor 1 is "free".
         */
        close(STDOUT_FILENO);

        /*
         * Step 2: Open a file.
         *
         * KEY INSIGHT: When you open a file, Unix assigns the LOWEST available
         * file descriptor. Since we just closed fd 1, this open() gets fd 1!
         *
         * This means: anything written to "stdout" now goes to this file instead.
         * The wc command will think it's printing to the screen, but it's actually
         * writing to wc_output.txt.
         *
         * Flags explained:
         *   O_RDWR   - Open for reading and writing
         *   O_CREAT  - Create the file if it doesn't exist
         *   O_TRUNC  - If file exists, truncate it to zero length
         *   S_IRWXU  - Give owner read/write/execute permissions (this is actually
         *              a mode argument, slightly misplaced here but often works)
         */
        open("wc_output.txt", O_RDWR | O_CREAT | O_TRUNC | S_IRWXU);

        /*
         * Set up the argument array for the 'wc' (word count) program.
         *
         * This mimics what the shell does when you type: wc fork_wait_exec_redirect.c
         *
         * argv[0] = program name (by convention)
         * argv[1] = first argument (the file to count)
         * argv[2] = NULL (marks end of argument list - REQUIRED!)
         */
        char *argv[3];
        argv[0] = "wc";                        /* Program name */
        argv[1] = "fork_wait_exec_redirect.c"; /* File to analyze */
        argv[2] = NULL;                        /* Argument list terminator */

        /*
         * execvp() REPLACES this process's code with a completely different program.
         *
         * "exec" = execute a new program
         * "v"    = arguments passed as a vector (array)
         * "p"    = search PATH environment variable to find the program
         *
         * CRITICAL: If exec succeeds, it NEVER RETURNS! The child process becomes
         * the 'wc' program. The code below only runs if exec fails.
         *
         * First argument:  program to run ("wc")
         * Second argument: array of arguments (including program name)
         */
        execvp("wc", argv);

        /*
         * We only reach here if exec failed (e.g., program not found).
         * This is why we still need error handling after exec.
         */
        fprintf(stderr, "exec failed\n");
        exit(1);
    }
    else
    {
        /*
         * ========== PARENT PROCESS CODE ==========
         *
         * We know we're the parent because fork() returned a positive value
         * (the child's PID, stored in 'rc').
         */

        /*
         * wait() makes the parent BLOCK (pause) until the child terminates.
         *
         * Without wait(), the parent might finish and print before the child
         * completes its work - the output would be unpredictable.
         *
         * &status: Pointer to an integer where wait() stores information about
         *          how the child exited (exit code, signal that killed it, etc.)
         *
         * Return value: The PID of the child that terminated, or -1 on error.
         */
        int status;
        pid_t rc_wait = wait(&status);

        /*
         * By now, the child has finished. The 'wc' output is in wc_output.txt.
         * Print a summary showing:
         *   - rc:      The child's PID (what fork() returned)
         *   - rc_wait: The PID returned by wait() (should match rc)
         *   - getpid(): Our own PID (the parent's PID)
         */
        printf("parent of %ld (rc_wait: %ld) (pid: %ld)\n",
               (long)rc, (long)rc_wait, (long)getpid());
    }

    exit(0); /* Terminate successfully */
}