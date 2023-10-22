#include <stdio.h>
#include "shell.h"

// Shell waiting for the user to write a command
void print_prompt1(void)
{
    fprintf(stderr, "$ ");
}


// Incase of multi-line command
void print_prompt2(void)
{
    fprintf(stderr, "> ");
}