#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
 
#define BUFFERSIZE 1024
#define COPYMORE 0644
 
void error(char *, char *);
int cpDir(char *source, char *destin);
int cpFile(char *source, char *destin);
int statTest(char *name); 

int main(int argc, char *argv[])
{
  if(argc != 3)
  {
    fprintf(stderr, "Failed to supply 3 args in \"cmd source dest\" format: %d args is incorrect\n", argc);
    exit(1);
  }

   char destination[PATH_MAX+1];
   strcpy(destination, argv[2]);
   char *source = argv[1];
 
   if( source[0] != '/' && destination[0] != '/' )
   {
       cpFile(source, destination);
   }
   else if( source[0] != '/' && destination[0] == '/' )
   {
      int i;
      for(i=1; i<=strlen(destination); i++)
      {
          destination[(i-1)] = destination[i];
      }
      strcat(destination, "/");
      strcat(destination, source);
      cpFile(source, destination);
  }
  else if( source[0] == '/' && destination[0] == '/' ) 
 {                             int i;
      for(i=1; i<=strlen(destination); i++)
      {
          destination[(i-1)] = destination[i];
      }
      for(i=1; i<=strlen(source); i++)
      {
          source[(i-1)] = source[i];
      }
      cpDir(source, destination);
  }
  else
  {
      fprintf(stderr, "Directory defined is outside the current working directory.\n");
      exit(1);
  }
}
 
int cpDir(char *source, char *destin)
{

    char tempDest[strlen(destin)+1];
    char tempSrc[strlen(source)+1];
    DIR *dir_ptr = NULL;
    struct dirent *direntPtr;

    strcat(destin, "/");
    strcat(source, "/");
    strcpy(tempDest, destin);
    strcpy(tempSrc, source); 
 
   if( (dir_ptr = opendir(source)) == NULL )
   {
      fprintf(stderr, "Failed to open %s to copy\n", source);
      return 0;
   }
   else
   {
      while( (direntPtr = readdir(dir_ptr)))
      {      
           
      if(statTest(direntPtr->d_name))  
      {   
              strcat(tempDest, direntPtr->d_name);        
          strcat(tempSrc, direntPtr->d_name);
          cpFile(tempSrc, tempDest);
              strcpy(tempDest, destin);
          strcpy(tempSrc, source);           
      }
      }
      closedir(dir_ptr);
      return 1;
   }
 
 
}

int statTest(char *name)
{
    struct stat fileInfo;  
    if(stat(name, &fileInfo) >=0)
    if(S_ISREG(fileInfo.st_mode))
      return 1;
    else return 0;
}

int cpFile(char *source, char *destin)
{
  int in_fd, out_fd, n_chars;
  char buf[BUFFERSIZE];
   if( (in_fd=open(source, O_RDONLY)) == -1 )
  {
    error("Failed open of", source);
  }
 
  if( (out_fd=creat(destin, COPYMORE)) == -1 )
  {
    error("Failed open of", destin);
  }
  while( (n_chars = read(in_fd, buf, BUFFERSIZE)) > 0 )
  {
    if( write(out_fd, buf, n_chars) != n_chars )
    {
      error("Error on write to", destin);
    }
 
    if( n_chars == -1 )
    {
      error("Error on read from", source);
    }
  }
    if( close(in_fd) == -1 || close(out_fd) == -1 )
    {
      error("Error closing", "");
    }
    return 1;
}

  void error(char *source1, char *source2)
  {
    fprintf(stderr, "Error: %s ", source1);
    perror(source2);
    exit(1);
  }
