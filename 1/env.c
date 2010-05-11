/* 
 * File:   env.c
 * Author: alsod
 *
 * Created on October 22, 2009, 9:44 AM
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * 
 */
int main(int argc, char** argv, char **envp) {
    int i;
    for (i = 0; envp[i] != NULL; i++) {
        printf("%2d:%s\n",i, envp[i]);
    }
}

