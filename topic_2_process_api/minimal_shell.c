/*
 * A MINIMAL SHELL (Command Line Interpreter)
 * ==========================================
 * This program demonstrates the fundamental pattern used by ALL Unix shells:
 *   1. Read a command from the user
 *   2. Fork a child process
 *   3. Child runs the command using exec()
 *   4. Parent waits for child to finish
 *   5. Repeat
 *
 * KEY CONCEPT: The separation of fork() and exec() is what makes shells powerful.
 * Between fork() and exec(), the shell can set up redirections, pipes, etc.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 100 /* Maximum length of a command the user can type */

int main()
{
    char buf[MAXLINE]; /* Buffer to store the command typed by user */
    pid_t rc;          /* Will hold return value of fork() */

    /*
     * Print the shell prompt: "%"
     * Real shells use "$" or ">" but we use "%" to distinguish our mini-shell
     */
    printf("%% ");

    /*
     * THE MAIN SHELL LOOP
     * ===================
     * fgets() reads one line from stdin (keyboard) into buf
     * Returns NULL when user types Ctrl+D (end of input), which exits the loop
     */
    while (fgets(buf, MAXLINE, stdin) != NULL)
    {

        /*
         * HOUSEKEEPING: Remove the newline character
         * When user types "ls" and presses Enter, buf contains "ls\n"
         * We need to replace '\n' with '\0' (null terminator) so it becomes "ls"
         * Otherwise, execvp would try to run a program literally named "ls\n"
         */
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = '\0';

        /*
         * STEP 1: CREATE A NEW PROCESS
         * ============================
         * fork() creates an (almost) identical copy of this process
         *
         * After fork(), we have TWO processes running this same code:
         *   - Parent (the shell): rc = child's PID (positive number)
         *   - Child (will run the command): rc = 0
         *   - If fork failed: rc = -1 (negative)
         */
        rc = fork();

        if (rc < 0)
        {
            /* FORK FAILED - system couldn't create a new process */
            fprintf(stderr, "Fork error\n");
            exit(1);
        }
        else if (rc == 0)
        {
            /*
             * CHILD PROCESS - This code runs in the newly created process
             * =============
             * The child's job: transform itself into the program the user requested
             */

            /*
             * Set up arguments for execvp()
             * execvp() expects: execvp(program_name, argument_array)
             *
             * argv[0] = program name (convention: first arg is always the program name)
             * argv[1] = NULL (signals end of argument list)
             *
             * NOTE: This simple shell doesn't handle command-line arguments!
             * A real shell would parse "ls -l" into argv = {"ls", "-l", NULL}
             */
            char *argv[2];
            argv[0] = buf;  /* The command to run, e.g., "ls" */
            argv[1] = NULL; /* No additional arguments */

            /*
             * STEP 2: REPLACE CHILD'S CODE WITH THE REQUESTED PROGRAM
             * ========================================================
             * execvp() does NOT create a new process!
             * It REPLACES the current process's code with the new program.
             *
             * After successful exec, the child is no longer running this shell code—
             * it's now running "ls" or "pwd" or whatever the user typed.
             *
             * The 'p' in execvp means: search for the program in PATH directories
             * So typing "ls" works without typing "/bin/ls"
             *
             * CRITICAL: If execvp succeeds, it NEVER RETURNS!
             * The lines below only run if execvp FAILS.
             */
            execvp(buf, argv);

            /* If we reach here, exec failed (program not found, no permission, etc.) */
            fprintf(stderr, "exec error\n");
            exit(1);
        }
        else
        {
            /*
             * PARENT PROCESS (rc > 0, rc is the child's PID)
             * ==============
             * The parent (our shell) waits here for the child to finish
             */

            /*
             * STEP 3: WAIT FOR CHILD TO COMPLETE
             * ===================================
             * wait(NULL) blocks the parent until ANY child exits
             *
             * Without wait(), the shell would immediately print the next prompt
             * while the command is still running — that would be confusing!
             *
             * The NULL means we don't care about the child's exit status.
             * A real shell might use waitpid() to get the exit code.
             */
            wait(NULL);

            /* Child finished, print prompt for next command */
            printf("%% ");
        }
    }

    exit(0); /* User typed Ctrl+D, exit the shell gracefully */
}