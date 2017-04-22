/************************* PREPROCESSOR DIRECTIVES ************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "mysh_routines.h"

/*************************** FUNCTION PROTOTYPES **************************/
int exe_cmd(char **arguments);

/********************** BUILTIN FUNCTION PROTOTYPES ***********************/
int mycd(char **arguments);
int mypwd();
int myexit();
int mysh_number_of_builtins();

/********** builtin command names and array of function pointers **********/
char *builtin_commands[] = { "mycd", "mypwd", "myexit" };
int (*builtin_functions[]) (char **) = { &mycd, &mypwd, &myexit };

int mysh_number_of_builtins() {
    return sizeof(builtin_commands) / sizeof(char *);
}

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
    char *user_input;
    char **arguments;
    int running;
    int num_tokens = 0;
	
    do
    {
        printf("\nEnter a command for da DJ\n");
        printf(" ___\n");
        printf("d-_-b\n");
        printf(" ~|~\n");
        printf(" / \\   -> ");
        user_input = read_input();
        arguments = tokenize_line(user_input);
        running = exe_cmd(arguments);
        
        /* free dynamically-allocated memory */
        free(user_input);
        free(arguments);
        
    } while (!running);
	
	return 0;
}

/************************** FUNCTION DEFINITIONS **************************/

/*******************************************************
 * execute_cmd takes the user input that has been tokenized
 *   and then runs that command by calling run_cmd(char **)
 * returns the result of either a function pointer (if the
 *   tokenized input matches a builtin function) or run_cmd(char **)
 *******************************************************/
int exe_cmd(char **arguments)
{
    int index = 0;
    int num_builtins = mysh_number_of_builtins();
    
    /* return if the first argument, the command, is an empty string */
    if(arguments[0] == NULL)
        return 1;
    
    /* Iterate through the builtin commands and compare the arguments *
     * with them their names. If the command is found in the array of *
     * builtin function names, use function return a function pointer *
     * to exectute the particular builtin function.                   */
    while (index < num_builtins) {
        if (strcmp(arguments[0], builtin_commands[index]) == 0)
            return (*builtin_functions[index])(arguments);
        index++;
    }
    
    return run_cmd(arguments);
}

/********************** BUILTIN FUNCTION DEFINITIONS **********************/

/*******************************************************
 * mycd calls the chdir(char **) function to change the 
 *   working directory of the current environment mysh is 
 *   running in.
 * returns 1 so that mysh can continue looping since the
 *   variable 'running' in main needs to be assigned 1
 *   for that purpose.
 *******************************************************/
int mycd(char **arguments)
{
    if (arguments[1] == NULL)
    {
        fprintf(stderr, "An argument is needed for \"mycd\"\n");
    }
    else
    {
        if (chdir(arguments[1]) != 0)
        {
            fprintf(stderr, "\"mycd\": failed to change directories\n");
        }
    }
    return 0;
}

/*******************************************************
 * mypwd calls the getcwd(char *, int) function to find the 
 *   current directory of the environment mysh is running in.
 * returns 1 instead of 0 so that my the variable 'running'
 *   in main will be assigned 1, thereby allowing the loop
 *   in main to continue.
 *******************************************************/
int mypwd()
{
    char current_dir[1024];
    
    if (getcwd(current_dir, sizeof(current_dir)) != NULL)
    {
        printf("Working directory:\n");
        printf("%s\n", current_dir);
    }
    else
    {
        fprintf(stderr, "error determining current directory\n");
    }
    return 0;
}

/*******************************************************
 * myexit returns 0 so that the variable 'running' will
 *   be assigned 0 and thus allow the loop in main to end,
 *   thereby terminating mysh.
 *******************************************************/
int myexit()
{
    return 1;
}

