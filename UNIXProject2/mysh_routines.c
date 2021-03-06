/*******************************************************
 * File Name: mysh_routines.c
 * 
 * This file contains function implementations called in
 * the main method of mysh.c. It implements functions that
 * reading of user input, tokenize that input, and running
 * a command(s) via forking a process and I/O redirection 
 * and piping.
 *******************************************************/
#include "mysh_routines.h"

/*******************************************************
 * read_input reads input from the keyboard.
 * returns that input as a an array of characters. 
 *******************************************************/
char *read_input()
{
	char *input = NULL;
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
	 
	if (!tokens)
	{
		fprintf(stderr, "mysh: error allocating memory for tokens\n");
		exit(1);
	}
	
	token = strtok(line, MYSH_TOKEN_DELIMITER);
	while (token != NULL)
	{
		tokens[position] = token;
		position++;
	
		if (position >= buffer_size)
		{
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
 * run_cmd takes the tokenized arguments and determines
 *   how it wants to execute them depending on if they 
 *   require input or output redirect, piping, or expansion.
 * returns value of -1 to allow mysh to keep running.
 *******************************************************/
int run_cmd(char **arguments)
{
	if (arguments == NULL)
		exit(1);
            
    pid_t pid;
    int status_of_child;
    int return_value = 0;
    int buf_size = MYSH_TOKEN_BUFFER_SIZE;
    int m;                  /* outer iterator for arguments                 */
    int n;                  /* inner iterator for arguments                 */
    int i;                  /* counter for 1st and possibly 2nd set of args */
    int j;					/* iterator second set of args                  */
    int held_index;         /* placeholder where arguments is temp stopped  */
    int flag =0;			/* if set, will get 2nd set of args             */
    char io_changer = '\0'; /* to hold >, <, |, or $                        */
    
    /* variables for exexcvp */
    char **argv1 = malloc(buf_size * sizeof(char*));
    char **argv2 = malloc(buf_size * sizeof(char*));
    char a_file_name[MYSH_TOKEN_BUFFER_SIZE];
    char b_file_name[MYSH_TOKEN_BUFFER_SIZE];
    char exp_command[MYSH_TOKEN_BUFFER_SIZE];
    char command2[MYSH_TOKEN_BUFFER_SIZE];
    
    /* check for I/O redirection, expansion, or piping from arguments   */
    /* parse the arguments into two sets of command line args if needed */
    m = 0;
    while (arguments[m] != NULL)
    {
		/* if there is a '<' in the input line */
		if (arguments[m][0] == '<')
		{
			io_changer = '<';
			for (i = 0; i < m; i++)
				argv1[i] = arguments[i];
			
			/* check to see if a file name exists after < */
			m++;
			if (arguments[m] == NULL)
			{
				fprintf(stderr, "no file provied to read from\n");
				return 0;
			}
			
			/* get the name of the file to be redirected */
			i = 0;
			while (arguments[m][i] != '\0')
			{
				a_file_name[i] = arguments[m][i];
				i++;
			}
			a_file_name[i] = '\0';
			
			/* check to see if redirection to output file needed */
			m++;
			if (arguments[m] != NULL)
			{
				if (arguments[m][0] == '>')
				{
					io_changer = 'b';
					
					/* Make sure the user entered a file name for */
					/* input redirection.                         */
					m++;
					if (arguments[m] == NULL)
					{
						fprintf(stderr, "no file provided to write to\n");
						return 0;
					}
					
					/* get the name of the file to be written to */
					i = 0;
					while (arguments[m][i] != '\0')
					{
						b_file_name[i] = arguments[m][i];
						i++;		
					}
					b_file_name[i] = '\0';
				}
			
				break;
			}
		}
		/* if there is a '>' in the input line */
		else if (arguments[m][0] == '>')
		{
			io_changer = '>';
			for (i = 0; i < m; i++)
				argv1[i] = arguments[i];
			
			/* get the file name and then break out of loop */
			m++;
			if (arguments[m] == NULL)
			{
				fprintf(stderr, "no file provided to write to\n");
				return 0;
			} 

			/* get the name of the file to be written to */
			i = 0;
			while (arguments[m][i] != '\0')
			{
				b_file_name[i] = arguments[m][i];
				i++;
			}
			b_file_name[i] = '\0';
			
			break;
		}
		
		/* there is a '$()' in the input line */
		else if (arguments[m][0] == '$')
		{
			io_changer = '$';
			
			/* get the name of the command to be extracted */ 
			n = 1;
			i = 0;
			while (arguments[m][n] != '\0')
			{
				if (arguments[m][n] != '(' && arguments[m][n] != ')')
				{
					exp_command[i] = arguments[m][n];
					i++;
				}
				n++;
			}
			exp_command[i] = '\0';
			
			/* get command line arguments for the two commands */
			argv1[0] = arguments[0];
			argv1[1] = NULL;
			argv2[0] = exp_command;
			argv2[i] = NULL;

			break;
		}
		
		/* if there is a '|' in the input line */
		else if (arguments[m][0] == '|')
		{
			flag = 1;
			io_changer = '|';
			for (i = 0; i < m; i++) {
				argv1[i] = arguments[i];
			}
			argv1[i] = NULL;
			held_index = m + 1;
		}
		m++;
	}

    /* get the 2nd command and its set of command line args if piping */
    if (flag)
    {
		/* get the name of the second command */
		i = 0;
		while (arguments[held_index][i] != '\0') {
			command2[i] = arguments[held_index][i];
			i++;
		}
		command2[i] = '\0';
		
		/* get the command line arguments for command2 */
		for (i = held_index, j = 0; i < m; i++, j++)
			argv2[j] = arguments[i];
		argv2[j] = NULL;
	}
    
    /* if io_changer has one of the specified values, execute with *
     * redirection or piping. Otherwise, go with the default case  *
     * and just execute the command.                               */
    switch(io_changer)
    {   
        case '<':
            /* redirect input */
            return_value = redirect_input(arguments[0], a_file_name, argv1);
            break;
            
        case '>':
            /* redirect stdout */
            return_value = redirect_output(arguments[0], b_file_name, argv1);
            break;
            
        case 'b':
			/* redirect stdout and stdin */
			return_value = redir_in_out(arguments[0], argv1,
										a_file_name, b_file_name);
			break;
        
        case '$':
            /* expand: pipe but in the opposite direction */
            return_value = pipe_commands(exp_command, arguments[0], argv2,
										 argv1);
            break;
            
        case '|':
            /* pipe */
            return_value = pipe_commands(arguments[0], command2, argv1,
										 argv2);
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
    
    free(argv1);
    free(argv2);
	return return_value;
}

/*******************************************************
 *
 *******************************************************/
int redirect_input(char *cmd, char *file, char **argv)
{
    int status;
    int ret_val;
    int fd;
    int newfd;
    pid_t pid;
    
    pid = fork();
    if (pid == 0)
    {
        fd = open(file, O_RDONLY);
        newfd = dup2(fd, 0);
        if (newfd != 0) {
			fprintf(stderr, "Could not duplicate fd to 0.\n");
			ret_val = -1;
		}
		else
        {
			close(fd);
			execvp(cmd, argv);
			fprintf(stderr, "execvp\n");
			ret_val = -1;
        }
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
    
    return ret_val;
}

/*******************************************************
 *
 *******************************************************/
int redirect_output(char *cmd, char *file, char **argv)
{
	int status;
	int ret_val;
	int fd;
    pid_t pid;
    
    pid = fork();
    if (pid == 0)
    {
		close(1);
        fd = creat(file, 0644);
        
        if (fd < 0) {
            fprintf(stderr, "error opening file descripter\n");
            ret_val = -1;
        }
        execvp(cmd, argv);
        fprintf(stderr, "execvp\n");
        ret_val = -1;
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
    
    return ret_val;
}

/*******************************************************
 *
 *******************************************************/
int redir_in_out(char *cmd, char **argv, char *a_file, char *b_file)
{
	int status;
    int ret_val;
    int fd;
    int newfd;
    pid_t pid;
    
    pid = fork();
    if (pid == 0)
    {
		/* redirect input */
        fd = open(a_file, O_RDONLY);
        newfd = dup2(fd, 0);
        if (newfd != 0) {
			fprintf(stderr, "Could not duplicate fd to 0.\n");
			ret_val = -1;
		}
		else
        {
			close(fd);
			
			/* redirect output */
			close(1);
			fd = creat(b_file, 0644);
        
			if (fd < 0) {
				fprintf(stderr, "error opening file descripter 1\n");
				ret_val = -1;
			}
			
			/* execute the command */
			execvp(cmd, argv);
			fprintf(stderr, "execvp\n");
			ret_val = -1;
        }
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
}

/*******************************************************
 *
 *******************************************************/
int pipe_commands(char *cmd1, char* cmd2, char **argv1, char **argv2)
{
    pid_t pid;
    int pp[2];
    int status;
    int ret_val;
    
    /* get a pipe */
    if (pipe(pp) < 0) {
        fprintf(stderr, "error creating pipe\n");
        return -1;
    }
  
    /* get a process */
    pid = fork();
    if (pid < 0)
    {
		fprintf(stderr, "error forking process\n");
		ret_val = -1;
	}
	else if (pid > 0)		/* parent will execute cmd2     */
	{
		close(pp[1]);		/* parent doesn't write to pipe */
		
		if (dup2(pp[0], 0) == -1) {
			fprintf(stderr, "Could not redirect stdin\n");
			return -1;
		}
		
		close(pp[0]);		/* stdin is duped, close pipe   */
		
		if (execvp(cmd2, argv2) == -1)
		{
			fprintf(stderr, "error running %s\n", cmd2);
			if (cmd2[0] =='$') 
			{
				fprintf(stderr, "%s should be the second argument\n", cmd2);
			}
			return -1;
		}
		
		wait(&status);
		ret_val = WEXITSTATUS(status);
	}
	else					/* child will execute cmd1      */
	{
       close(pp[0]);		/* child doesn't read from pipe */
       
        if (dup2(pp[1], 1) == -1) {
			fprintf(stderr, "Could not redirect stdout\n");
			return -1;
		}
		
        close(pp[1]);
        
        if (execvp(cmd1, argv1) == -1) {
            fprintf(stderr, "error running %s\n", cmd1);
            ret_val = -1;
        }
    }
    
    return ret_val;
}
