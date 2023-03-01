#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 80    /* The maximum length command */
#define MAX_HISTORY 10 /* The maximum number of commands in history */

char history[MAX_HISTORY][MAX_LINE]; /* history buffer */
int history_count = 0;               /* number of commands in history */

int add_to_history(char *command)
{
    /* Add a command to the history buffer */
    if (history_count < MAX_HISTORY)
    {
        strcpy(history[history_count], command);
        history_count++;
    }
    else
    {
        /* Shift all the commands in the buffer by one */
        for (int i = 0; i < MAX_HISTORY - 1; i++)
        {
            strcpy(history[i], history[i + 1]);
        }
        strcpy(history[MAX_HISTORY - 1], command);
    }
}

int parse_input(char *input, char **args)
{
    /* Parse the input into separate arguments */
    int argc = 0;
    args[argc] = strtok(input, " \n");
    while (args[argc] != NULL)
    {
        argc++;
        args[argc] = strtok(NULL, " \n");
    }
    return argc;
}

int should_run_in_background(int argc, char **args)
{
    /* Check if the command should be run in the background */
    return (argc > 0 && strcmp(args[argc - 1], "&") == 0);
}

int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */

    while (should_run) {
        printf("osh>");
        fflush(stdout);

        /* Read input from the user */
        char input[MAX_LINE];
        fgets(input, MAX_LINE, stdin);

        /* Check if the user wants to execute the most recent command */
        if (strcmp(input, "!!\n") == 0) {
            if (history_count == 0) {
                printf("No commands in history.\n");
                continue;
            }
            strcpy(input, history[history_count-1]);
            printf("%s", input); /* echo the command */
        } else {
            /* Add the command to the history buffer */
            add_to_history(input);
        }

        /* Parse the input into separate arguments */
        int argc = parse_input(input, args);

        /* Check if the command should be run in the background */
        int ampersand = should_run_in_background(argc, args);
        if (ampersand && argc > 0) {
            args[argc-1] = NULL; /* remove the ampersand from the argument list */
        }

        /* Fork a child process */
        pid_t pid = fork();

        if (pid < 0) {
            /* Error occurred */
            fprintf(stderr, "Fork failed.\n");
            return 1;
        } else if (pid == 0) {
            /* Child process */
            if (strcmp(args[0], "exit") == 0) {
                should_run = 0;
                return 0;
            }
            execvp(args[0], args);
            fprintf(stderr, "Command not found.\n");
            return 1;
        } else {
            /* Parent process */
            if (!ampersand) {
                /* Wait for the child process to finish */
                wait(NULL);
            }
        }
    }
    return 0;
}