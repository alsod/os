/*
 *
 * NAME:
 *   digenv  -  a program for using filters on printenv with pipes
 *
 * SYNTAX:
 *   digenv [arguments for grep]
 *
 * DESCRIPTION:
 *   digenv uses pipes to apply filters on the output from printenv, that is
 *   the enviroment variables. If arguments are passed, the filters will be grep 'arguments' | sort | pager
 *   if no arguments are passed, grep will be omitted. The pager is either a pager set in the environment variables
 *   or if no variable set, less is tried, if less not found, more is used.
 *
 * OPTIONS:
 * see man grep
 *
 * EXAMPLES:
 *   digenv PATH -n
 *
 * ENVIRONMENT:
 *   PAGER - Used to determine what pager to use, can be nonexistent.
 *
 * NOTES:
 * if parameters passed is unallowed for grep, the program will print the standard
 * grep error message.
 *
 * if less or pager in PATH not found, program will try more as a last resort.
 *
 */


#include <sys/types.h> /* definierar typen pid_t */
#include <sys/wait.h> /* definierar bland annat WIFEXITED */
#include <string.h> /* definierar bland annat WIFEXITED */
#include <errno.h> /* definierar errno */
#include <stdio.h> /* definierar bland annat stderr */
#include <stdlib.h> /* definierar bland annat rand() och RAND_MAX */
#include <unistd.h> /* definierar bland annat pipe() och STDIN_FILENO */

#define DEFAULT_PAGER ("less");

/*
 * contains a char array with the name of the filter
 * and a array of char arrays with the arguments to use with execvp eg {"filter_name", (char*) 0}
 *
 */
struct Filter {
    char *name;
    char **args;
};

/*
 * Get pager set in environment if any, if not sett use defined default pager
 */
char* getPager() {
    char* envPager = getenv("PAGER");

    if (envPager == NULL) {
        return DEFAULT_PAGER;
    } else
        return envPager;
}

/*
 * piper creates new processes for each filter and pipes the result between these.
 * returns 0 if no error.
 */
int piper(struct Filter *filters, int id) {
    int fd[2];
    int pid;
    int status;

    if ( pipe(fd) < 0)
        perror("pipe creation error"), exit(1);

    if ((pid = fork()) < 0) /* fork */
        perror("fork creation error"), exit(1);

    if (pid == 0) { /* Child */
        if(dup2(fd[1], 1) < 0) /* Connect stdout to pipe */
            perror("dup2() error in child"), exit(1);

        if(dup2(fd[1], 2) < 0) /* Connect stderr to pipe */
            perror("dup2() error in child"), exit(1);

        if(close(fd[0]) < 0) /* close read side */
            perror("close() read side error in child"), exit(1);
        if(close(fd[1]) < 0) /* close write side */
            perror("close() write side error in child"), exit(1);

        if(id-1 >= 0){  /* recurse until at first filter. */
            piper(filters, id-1);
        }
    } else { /* Parent */

        if(wait(&status) < 0) /* Wait for child process to terminate*/
            perror("wait() error"), exit(1);

        char buffer[50];
        

        
        if(dup2(fd[0], 0) < 0)/* Connect stdin to pipe */
            perror("dup2() error in parent"), exit(1);
        
     

        if(close(fd[1]) < 0) /* close write side */
            perror("close() write side error in parent"), exit(1);

        if(close(fd[0]) < 0) /* close read side */
            perror("close() read side error in parent"), exit(1);

/*
        if(read(0, buffer, sizeof(buffer)) > 0){

            perror("read error");
            printf("buffer: %s\n", buffer);
            exit(1);
        }
        
*/
      

        execvp(filters[id].name, filters[id].args); /* exec() filter */
        

  char *buf_comp = "";
        if(strcmp(buffer, buf_comp) == 1){
            perror("no output");
            exit(1);
        }

        char *test_name = "less";
        if(strcmp(filters[id].name, test_name) == 1)
            execlp("more", "more", (char *) 0); /* exec() filter */

        perror("execvp() error in parent");
        exit(1);
    }
    return 0;
}

int main(int argc, char** argv) {

    char* pager = getPager(); /* get the pager to use */
    argv[0] = "grep"; /* changes first element in argv so it can be used for exec */

    /*
     * argumentarrays to be used for exec
     */
    char *argsA[] = {"printenv", (char*) 0};
    char *argsB[] = {"sort", (char*) 0};
    char *argsC[] = {pager, (char*) 0};

    /*
     * two different filter sets, one if no arguments to main
     * and the other if there are arguments
     */
    struct Filter filters1[] = {
        {"printenv", argsA},
        {"sort", argsB},
        {pager, argsC}
    };

    struct Filter filters2[] = {
        {"printenv", argsA},
        {"grep", argv},
        {"sort", argsB},
        {pager, argsA}
    };

    if(argc > 1){ /*argument array not empty*/
        int maxIdFilters2 = 3; /* last allowed id in array */
        piper(filters2, maxIdFilters2); /* do forking and piping */
    }else{ /* no arguments to main*/
        int maxIdFilters1 = 2; /* last allowed id in array */
        piper(filters1, maxIdFilters1); /* do forking and piping */
    }


    return 0;

}
