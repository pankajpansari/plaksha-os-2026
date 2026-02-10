/*
 * CONCEPT: This program demonstrates the three fundamental UNIX process API calls:
 *   1. fork()  - Creates a new child process (a copy of the parent)
 *   2. exec()  - Replaces the current process with a different program
 *   3. wait()  - Parent waits for child to finish before continuing
 *
 * This fork-exec-wait pattern is exactly how a shell (like bash) runs commands!
 * When you type "ls" in a terminal, the shell does: fork → child execs "ls" → parent waits
 */

#include <unistd.h>   /* Provides fork(), exec(), getpid() - core UNIX process functions */
#include <sys/wait.h> /* Provides wait() - for parent to wait on child */
#include <stdio.h>    /* Provides printf(), fprintf() - for printing output */
#include <stdlib.h>   /* Provides exit() - to terminate a process */

int main()
{
    /*
     * getpid() returns the Process ID (PID) of the calling process.
     * Every process in UNIX has a unique PID - it's like a process's "name" to the OS.
     * At this point, only ONE process exists (the original/parent).
     */
    printf("hello (pid:%ld)\n", (long)getpid());

    /*
     * fork() is the UNIX way to create a new process.
     *
     * MAGIC MOMENT: After fork() returns, there are now TWO processes running
     * this same code! Both continue from this exact point, but with different
     * return values:
     *   - Parent receives: the child's PID (a positive number)
     *   - Child receives:  0
     *   - On error:        -1 (no child created)
     *
     * Think of it as cloning - the child is an almost exact copy of the parent,
     * but they have separate memory spaces (changes in one don't affect the other).
     */
    pid_t rc = fork(); /* pid_t is a special type for storing process IDs */

    if (rc < 0)
    {
        /*
         * fork() failed - no child was created.
         * This is rare but can happen if system is out of resources.
         * stderr is used for error messages (separate from normal output).
         */
        fprintf(stderr, "fork failed\n");
        exit(1); /* Exit with status 1 to indicate error */
    }
    else if (rc == 0)
    {
        /*
         * THIS CODE RUNS IN THE CHILD PROCESS ONLY
         *
         * rc == 0 means "I am the child" - this is how the child knows who it is.
         * Notice getpid() now returns a DIFFERENT PID than the parent printed above!
         */
        printf("child (pid:%ld)\n", (long)getpid());

        /*
         * Prepare arguments for exec(). This is how we pass command-line arguments
         * to the new program we're about to run.
         *
         * We're setting up to run: wc fork_wait_exec.c
         * (wc = "word count" - counts lines, words, and bytes in a file)
         *
         * argv[0] = program name (by convention)
         * argv[1] = first argument (the file to count)
         * argv[2] = NULL (REQUIRED - marks end of argument list)
         */
        char *argv[3];
        argv[0] = "wc";
        argv[1] = "fork_wait_exec.c"; /* This file itself! */
        argv[2] = NULL;               /* Must be NULL-terminated */

        /*
         * execvp() REPLACES the current process with a new program.
         *
         * KEY INSIGHT: exec() does NOT create a new process - it transforms the
         * current process into something else entirely. The child's code, data,
         * stack - everything - gets replaced by the "wc" program.
         *
         * execvp("wc", argv):
         *   - "wc" = the program to run
         *   - argv = the arguments to pass
         *   - The 'p' in execvp means "search PATH" (finds wc in /usr/bin/wc)
         *   - The 'v' means "arguments passed as a vector (array)"
         *
         * If exec() succeeds, the lines below NEVER execute - because this
         * code no longer exists! The child is now running "wc" instead.
         */
        execvp("wc", argv);

        /*
         * If we reach here, exec() FAILED (maybe program not found).
         * On success, exec() never returns - the process becomes something else.
         */
        fprintf(stderr, "exec failed\n");
        exit(1);
    }
    else
    {
        /*
         * THIS CODE RUNS IN THE PARENT PROCESS ONLY
         *
         * rc > 0 means "I am the parent" and rc contains the child's PID.
         */

        /*
         * wait() blocks (pauses) the parent until the child terminates.
         *
         * Why is this important?
         *   - Without wait(), parent might finish and exit before child
         *   - Parent can learn if child succeeded or failed via 'status'
         *   - Prevents "zombie" processes (dead children not cleaned up)
         *
         * Returns the PID of the terminated child (should match 'rc').
         * The 'status' variable gets filled with how the child exited.
         */
        int status;
        pid_t rc_wait = wait(&status);

        /*
         * Now the child has finished (wc has counted the lines/words/bytes).
         * The parent resumes and prints this message.
         *
         * Notice: This printf ALWAYS appears AFTER the child's output (wc's output)
         * because wait() guarantees the parent doesn't continue until child is done.
         */
        printf("parent of %ld (rc_wait: %ld) (pid: %ld)\n",
               (long)rc,      /* Child's PID (from fork) */
               (long)rc_wait, /* Child's PID (from wait - should match) */
               (long)getpid() /* Parent's own PID */
        );
    }

    exit(0); /* Exit with status 0 to indicate success */
}