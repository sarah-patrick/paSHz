#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "mysh_routines.h"

/*******************************************************
 *
 *******************************************************/
int run_cmd(char **arguments)
{
	if (arguments == NULL)
		return -1;
            
    pid_t pid;
    int status_of_child;
    int return_value;
    int m;                  /* outer iterator for arguments                */
    int n;                  /* inner iterator for arguments                */
    char io_changer = '\0'; /* to hold >, <, |, or $                       */
    int flag = 0;           /* for breaking out of outer loop if necessary */
    
    /* check for I/O redirection or piping from arguments */
    m = 0;
    while (arguments[m] != NULL) {
        n = 0;
        while (arguments[m][n] != '\0') {
            /* set io_changer to character and break if character is found */
            if (arguments[m][n] == '|' || arguments[m][n] == '<' ||
                arguments[m][n] == '>' || arguments[m][n] == '$')
            {
                io_changer = arguments[m][n];
                flag = 1;
                break;
            }
            n++;
        }
        /* break out of second loop if flag is set */
        if (flag) break;
        m++;
    }
    
//    m = 0;
//    while (arguments[m] != NULL) {
//        printf("arguments[%d]: %s\n", m, arguments[m]);
//        m++;
//    }
    
    /* if io_changer has one of the specified values, execute with *
     * redirection or piping. Otherwise, go with the default case  *
     * and just execute the command.                               */
    switch(io_changer)
    {
        case '|':
            /* pipe */
            return_value = pipe_commands(arguments[0], arguments[2]);
            break;
        
        case '<':
            /* redirect input */
            return_value = redirect_input(arguments[0], arguments[2]);
            break;
            
        case '>':
            /* redirect stdout */
            break;
        
        case '$':
            /* expand */
            break;
        
        default:
            pid = fork();
            if (pid == 0)
            {
                if (execvp(arguments[0], arguments) == -1) {
                    fprintf(stderr, "error running %s", arguments[0]);
                    return_value = -1;
                }
            }
            else if (pid > 0)
            {
                wait(&status_of_child);
                return_value = WEXITSTATUS(status_of_child);
            }
            else
            {
                fprintf(stderr, "error forking a process");
                return_value = -1;
            }
    }
	return return_value;
}

/*******************************************************
 *
 *******************************************************/
char *read_input()
{
	char *input = NULL;
    //unsigned long buffer_size = 0;
    ssize_t buffer_size = 0;
    getline(&input, &buffer_size, stdin);
    return input;
}

/*******************************************************
 *
 *******************************************************/
char **tokenize_line(char *line)
{
    int buffer_size = MYSH_TOKEN_BUFFER_SIZE;
	int position = 0;
	char **tokens = malloc(buffer_size * sizeof(char*));
	char *token = NULL;
	 
	if (!tokens) {
		fprintf(stderr, "mysh: error allocating memory for tokens\n");
		exit(1);
	}
	
	token = strtok(line, MYSH_TOKEN_DELIMITER);
	while (token != NULL) {
		tokens[position] = token;
		position++;
	
		if (position >= buffer_size) {
			buffer_size += MYSH_TOKEN_BUFFER_SIZE;
			tokens = realloc(tokens, buffer_size * sizeof(char*));
			if (!tokens) {
				fprintf(stderr, "mysh: error allocating memory for tokens\n");
				exit(1);
			}
		}
	
		token = strtok(NULL, MYSH_TOKEN_DELIMITER);
	}
	
	tokens[position] = NULL;
	return tokens; 
}

/*******************************************************
 *
 *******************************************************/
int pipe_commands(char *cmd1, char* cmd2)
{
    pid_t pid1;
    pid_t pid2;
    int pp[2];
    int ret_val;
    
    if (pipe(pp) < 0) {
        fprintf(stderr, "error creating pipe\n");
        return 1;
    }
    
    pid1 = fork();
    if (pid1 == 0) {
        dup2(pp[1], 1);
        close(pp[0]);
        close(pp[1]);
        if (execvp(cmd1, NULL) == -1) {
            fprintf(stderr, "error running %s", cmd1);
            ret_val = -1;
        }
    }
    
    pid2 = fork();
    if (pid2 == 0) {
        dup2(pp[0], 0);
        close(pp[0]);
        close(pp[1]);
        if (execvp(cmd2, NULL) == -1) {
            fprintf(stderr, "error running %s", cmd2);
            ret_val = -1;
        }
    }
    
    close(pp[0]);
    close(pp[1]);
    ret_val = 1;
    return ret_val;
}

/*******************************************************
 *
 *******************************************************/
//int redirect_input(char *file, char *cmd)
//{
//    pid_t pid;
//    
//    pid = fork();
//    if (pid == 0) {
//        int fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0600);
//        
//        if (fd < 0) {
//            fprintf(stderr, "error opening file descripter\n");
//            exit(1);
//        }
//        
//        dup2(fd, 1);
//        close(fd);
//        execvp(cmd, NULL);
//    }
//    
//    return 1;
//}

/*******************************************************
 *
 *******************************************************/
int redirect_input(char *cmd, char *file)
{
    int status;
    int ret_val;
    pid_t pid;
    
    pid = fork();
    if (pid == 0)
    {
        int fd = open(file, O_RDONLY, 0);
        dup2(fd, 0);
        close(fd);

        execvp(cmd, NULL);
        perror("execvp");
        exit(1);
    }
    else if (pid > 0)
    {
        wait(&status);
        ret_val = WEXITSTATUS(status);
    }
    else
    {
        fprintf(stderr, "error forking a process");
        ret_val = -1;
    }
    return -1;
}











