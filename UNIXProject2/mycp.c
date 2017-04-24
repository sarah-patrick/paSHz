#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define TRUE 1
#define FALSE 0

int checkDir(const char * path);
int cpFile(const char * source, const char * destination);
void getName(char * buffer, char * name);
void cpDir(const char * source, const char * destination);
int fileToFile(const char* source, const char* destination);

int main(int argc, char **argv)
{
    	int recursive = FALSE; // -R flag 
	char c;
	if (argc < 3 || argc > 4)
	{
		printf("ERROR: Fromat cmd ./mycp SRC DEST || ./mycp -Rflag SRCDIR DESTDIR\n");
		exit(1);
	}    
	if (argc==3)
	{
		int returnValue = fileToFile(argv[1],argv[2]);
		//	printf("Invalidoption");
		return returnValue;
	}   
	if (argc==4)
	{
		while((c = getopt(argc, argv, ":R")) != -1) 
		{
			switch(c)
			{
				case 'R':
				recursive = TRUE;
				break; 
			case '?': 
				fprintf(stderr, "Invalidoption -%c\n", optopt);
				return 1; 
				break;
			}   
		}
        	if (recursive == TRUE)
		{              
			cpDir(argv[2], argv[3]);
        	}
	}
    return 0;
}

int fileToFile(const char* source, const char* destination)
{
    	char buffer[1024];
    	int files[2];
    	ssize_t count;
    	files[0] = open(source, O_RDONLY);
    	if (files[0] == -1)
	{
        	printf("ERROR: You do not have permission to open file, or nonexistent choice.\n");
        	return -1;
    	}
	files[1] = open(destination, O_WRONLY | O_CREAT , 0644);    
    	if (files[1] == -1)
    	{
        	close(files[0]);
        	return -1;
    	}    
    	while ((count = read(files[0], buffer, sizeof(buffer))) != 0)
        	write(files[1], buffer, count);   
    	return 0;
}

void cpDir(const char * source, const char * destination)
{
   	 char sourceBuffer[BUFSIZ],  bufferDestination[BUFSIZ];
   	 char name[BUFSIZ];
   	 int flag;
   	 flag = checkDir(source);
    
   	 strcpy(sourceBuffer, source);
   	 strcpy(bufferDestination, destination);
    
    	if(flag == 0) 
    	{
        	getName(sourceBuffer, name);
        	strcat(bufferDestination, "/");
        	strcat(bufferDestination, name);
        	cpFile(sourceBuffer, bufferDestination);
        	return;
    	}
   	else if(flag == 1)
        {
            	getName(sourceBuffer, name);             
           	strcat(bufferDestination, "/");
            	strcat(bufferDestination, name);         
            	if(strcmp(name, ".")==0 || strcmp(name, "..")==0 )
		{
                	return;
        	}
            	struct stat old;            
            	if(stat(source, &old) == -1)
		{
                	printf("mkdir(%s), stat(%s) error!\n", bufferDestination, sourceBuffer);
                	return;
            	}            
            	mkdir(bufferDestination, old.st_mode | O_CREAT);
            	chmod(bufferDestination, old.st_mode);          
            	int fileTwo;
            	if( (fileTwo = creat(bufferDestination, old.st_mode)) == -1)
		{
                	mkdir(destination, old.st_mode | O_CREAT);
                	DIR * pdir;
                	struct dirent * pdirent;                
                	pdir = opendir(sourceBuffer);
                	while(TRUE) 
			{
                    		pdirent = readdir(pdir) ;
                    		if(pdirent == NULL)
                        		break;
                		else
				{
                       			strcpy(sourceBuffer, source);
                       			strcat(sourceBuffer, "/");
                       			strcat(sourceBuffer, pdirent->d_name);
                       			cpDir(sourceBuffer, destination);
                		}
			}
                	closedir(pdir);
		}
        	else
		{
               		DIR * pdir;
                	struct dirent * pdirent;
                	pdir = opendir(sourceBuffer);
                	while(TRUE) 
			{
                		pdirent = readdir(pdir) ;
                    		if(pdirent == NULL)
                        		break;
                		else
				{
                        		strcpy(sourceBuffer, source);
                        		strcat(sourceBuffer, "/");
                        		strcat(sourceBuffer, pdirent->d_name);
                        		cpDir(sourceBuffer, bufferDestination);
                    		}
                	}
                	closedir(pdir);
        	}
            	return;
        }
        else
           	return;
}

int checkDir(const char * path)
{
	struct stat buffer;    if(stat(path, &buffer) == -1)
	{
		printf("ERROR: dir(%s), status(%s) missmatch.\n", path, path);
		exit(1);
	}
	if((S_IFMT & buffer.st_mode) == S_IFDIR)
	{
        	return 1;
	}
	else
		return 0;
}

int cpFile(const char * source, const char * destination)
{
	int fileOne, fileTwo, n;
	char bf[BUFSIZ];
	struct stat old;
	if(stat(source, &old) == -1)
	{
		printf("ERROR: mycp error on stat:(%s)\n", source);
		return 0;
	}
	if( (fileOne = open(source, O_RDONLY)) == -1)
	{
		printf("ERROR: mycp failed on open:(%s)\n", source);
        	return 0;
	}
	if( (fileTwo = creat(destination, old.st_mode)) == -1)
	{
		printf("ERROR: mycp failed on create:(%s)\n", destination);
		close(fileOne);
		return 0;
	}
	if(fchmod(fileTwo, old.st_mode) == -1)
	{
		printf("ERROR: mycp failed on fchmod:(%s)\n", destination);
		return 0;
	}
	while((n = read(fileOne, bf, BUFSIZ)) > 0)
	{
       		if(write(fileTwo, bf, n) != n)
		{
            	printf("ERROR: mycp failed on write:(%s)\n", destination);
            	close(fileOne);
            	close(fileTwo);
            	return 0;
        	}
    	}
    	close(fileOne);
    	close(fileTwo);
    	return 1;
}

void getName(char * buffer, char * name)
{
    	int i, n, j;
    	n = strlen(buffer);
    	for(i = n - 1; i >=0 ; i--)
	{
        	if(buffer[i]=='/')
		{
            	break;
        	}
    	}
    	for(i++, j = 0; i < n; i++, j++)
        	name[j] = buffer[i];
    	name[j] = '\0';
}
