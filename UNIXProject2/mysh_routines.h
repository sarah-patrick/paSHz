#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MYSH_RL_BUFFER_SIZE 1024
#define MYSH_TOKEN_BUFFER_SIZE 64
#define MYSH_TOKEN_DELIMITER " \t\r\n\a"

int run_cmd(char **arguments);
char *read_input();
char **tokenize_line(char *line);
int pipe_commands(char *cmd1, char *cmd2, char **argv1, char **argv2);
int redirect_input(char *cmd, char *file, char **argv);
int redirect_output(char *cmd, char *file, char **argv);
int redir_in_out(char *cmd, char **argv, char *a_file, char *b_file);
