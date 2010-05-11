/* 
 * File:   small_shell.c
 * Author: alsod
 *
 * Created on November 24, 2009, 7:55 PM
 */
#define _XOPEN_SOURCE 500
#define _GNU_SOURCE


#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

/**
 *
 */
void pr_exit(int status) {
    if (WIFEXITED(status))
        printf("Process terminated normally, exit status = %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("Process terminated abnormally, signal number = %d, %s\n", WTERMSIG(status), strsignal(WTERMSIG(status)));
    else if (WIFSTOPPED(status))
        printf("Child stopped, signal number = %d, %s\n", WSTOPSIG(status), strsignal(WSTOPSIG(status)));

}

/* SIGCHLD handler. */
static void sigchld_hdl(int sig) {

    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("\nBackground process with pid %d terminated.\n", pid);
    }

}


/*
 * 
 */
int main(int argc, char** argv) {

    /* signaling*/
    struct sigaction act;
    memset(&act, 0, sizeof (act));
    act.sa_handler = sigchld_hdl;

    if (sigaction(SIGCHLD, &act, 0)) {
        perror("sigaction");
        return 1;
    }

    /* process */
    pid_t pid;
    int status;

    /* process timing */
    struct timeval start_time;
    struct timeval end_time;
    int runtime_s;
    double runtime_ms;

    /* command parsing */
    char command[500];
    char *params[6];

    char *home = getenv("HOME");
    int background = 0;




    while (1) {

        if (background != 1) /* will duplicate prompt otherwise */
            printf("prompt: ");

        background = 0; /* reset to default process mode*/


        while(fgets(command, 499, stdin) == NULL) { /* restart if fgets get interupted.*/
            printf("promptb: ");
        }

        strtok(command, "\n"); /* strip away the newline character*/

        /*-------------- Start of argument parsing -----------------------*/

        char *token;
        token = strtok(command, " "); /* get first token */

        int i = 0;
        while (token != NULL) {
            params[i] = token;
            token = strtok(NULL, " "); /* get tokens until no more is found*/
            i++;
        }

        i = i - 1;
        if (i > 0) {
            /* Check for & */
            if (strcmp(params[i], "&") == 0) {
                background = 1;
            }
            params[i] = (char) 0; /* dont need the & anymore */
        }
        params[i + 1] = (char) 0;

        /*----------------- End of argument parsing -----------------------*/

        /*----------------- Check for internal commands -------------------*/

        if (strcmp(params[0], "exit") == 0) { /* Check for exit */
            printf("----Bye----\n");
            exit(0); /* exiting shell */

        } else if (strcmp(params[0], "cd") == 0) { /* Check for cd */
            if (chdir(params[1]) < 0) {
                chdir(home); /* if directory not found, go to home directory */
            }
            /*----------------- End check for internal commands ----------------*/
        } else { /*Fork and execute*/
            sighold(SIGCHLD);
            if ((pid = fork()) < 0) {
                perror("Fork failed");
                exit(errno);
            }

            gettimeofday(&start_time, NULL); /* get start time for process */
            /*--------------------- In child ----------------------------------*/
            if (pid == 0) { /* Child */
                if (background < 1) {
                    fprintf(stderr, "\nSpawned foreground process, pid: %d\n", getpid());
                } else {
                    fprintf(stderr, "\nSpawned background process, pid: %d\npromptc:", getpid());
                }


                if (execvp(params[0], params) < 0) {
                    perror("exec failed");
                    exit(errno);
                }
                /*--------------------- End Child --------------------------------*/
                /*--------------------- Parent -----------------------------------*/
            } else { /* Parent */

                if (background < 1) {
                    waitpid(pid, &status, 0); /*wait for child*/

                    gettimeofday(&end_time, NULL); /* get end time for process */

                    /* do runtime calculations*/
                    runtime_s = end_time.tv_sec - start_time.tv_sec;
                    runtime_ms = ((end_time.tv_usec)-(start_time.tv_usec)) / 1000.0;

                    /* print some information about process and its exit status */
                    fprintf(stderr, "\nForeground process: %d terminated.\nRuntime: %d s %.*f ms\n", pid, runtime_s, 3, runtime_ms);
                    pr_exit(status);
                    printf("\n");
                }
                sigrelse(SIGCHLD);
                /*------------------------- End Parent ---------------------------*/
            }

        }

    }
    exit(0);
}

