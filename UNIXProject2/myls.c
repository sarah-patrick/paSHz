/*******************************************************************/
/* Name: Christina Haynes-Zaragoza	Date: 04/23/17		   */
/* Class: IntroUnix 			Assignment: project2	   */
/* Comment: Got alot of help coding myls from the website  */
/*	    www.annrich.com/cs590/notes/homework1/myls.txt	   */
/*******************************************************************/
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<time.h>
#include<pwd.h>
#include<grp.h>

#define TRUE 1
#define FALSE 0
#define MAX_FCOUNT 8192

#define SEC_PER_YEAR 60 * 60 * 24 * 365

/* *************************************** */
/* * Type: Stuct                         * */
/* * Name: file                          * */
/* * Description: An array structure     * */
/* *              that will hold file    * */ 
/* * 		  files to be sorted.    * */
/* **************************************** */
typedef struct z_file
{
	char  name[1000]; //files 
	struct stat buf; // inode for the file 
} file;
//typedef struct file FILE; // suppose to be some kind of shortcut*/ 

/* *********************** FUNCTION PROTOTYPES ****************** */
int compare_fileNames();
int list_permissions();
int print_long();
int list_file();
int myls();
int print_file(char *, char *, struct stat, int, int); 
/* ********************** MAIN ********************************** */
int main(int argc, char **argv)
{
	int showsize = FALSE; // -s flag 
	int showlong = FALSE; // -l flag 
	char c; // current option
	int incrament; // loop var
	while((c = getopt(argc, argv, ":sl")) != -1) // checking arguments
	{
	 	switch(c)
		{
			case 's':
				 showsize = TRUE;
				 break; 
			case 'l': 
				 showlong = TRUE; 
				 break;
			case '?': 
				fprintf(stderr, "Ivalidoption -%c\n", optopt);
				 return 1; 
				 break;
		}   
	}
	
	if(argc == optind) /* If no arguments then use . */
		myls(".", showsize,showlong); 
	else
	{
		for(incrament = optind; incrament < argc; incrament++)
		{
			printf("\n");
			myls(argv[incrament], showsize, showlong);
		}
	}; 	
return 0;
}

/**************************************************/
/* Function Name: myls                            */
/* Description: will take file information into   */
/* 		data struct and generate a "ls"	  */
/*		type output. 			  */
/**************************************************/

int myls(char *dir, int showsize, int showlong)
{
	int i;				/* counter variable     */
	DIR *dirptr;  		/* pointer to directory */
	struct dirent *dirstct;	/* directory struct */
	struct stat sbuf;       /* stst struct*/
	FILE *f; 		/* temp ptr to structf*/
	FILE **filename; 	/* array of file names*/
	int filecounters;       /* counter to keep track of array*/
	/*int incrament2; 	 second incramentaor for loops*/
	int total = 0;		/* keep count of the num of blocks n dir*/
	char fbuf[1000];	/* filename buffer*/
	
	if(lstat(dir,&sbuf) < 0) /* check file type*/
	{
		perror("stat call failed");
		_exit(1);
	}
	
	if(!S_ISDIR(sbuf.st_mode)) /* if its not a directory simply print*/
	{
		print_file (dir, NULL, sbuf, showsize, showlong);
		return(TRUE); 
	}
	/*update the name array*/
	filename = (FILE **) malloc(sizeof(FILE *) * MAX_FCOUNT + 1);
	filecounters = 0;
	
	if(filename == NULL) 
	{
		fprintf(stderr, "Cannot allocate filename array\n");
		return(FALSE);
	}
	if((dirptr = opendir(dir)) == NULL) /*open file directory*/ 
	{
		fprintf(stderr,"Cannot allocate filename array\n");
		return(FALSE);
	} 
	
	while((dirstct = readdir(dirptr)) != NULL) /*read directory and print results*/
	{
		if(dirstct->d_name[0] == '.')
			continue;
		sprintf(fbuf, "%s/%s", dir, dirstct->d_name);

		if(lstat(fbuf, &sbuf) < 0)
		{
			perror("stat call failed");
			return(FALSE);
		}
		if((f = (FILE *)malloc (sizeof(FILE *))) == NULL)
		{
			fprintf(stderr, "cannot malloc filename %s\n", 
				dirstct->d_name);
			return(FALSE);
		}
		strcpy(f->name, dirstct->d_name);
		f->buf = sbuf;
		filename[filecounters++] = f;
		
		total += sbuf.st_blocks;

		if(filecounters > MAX_FCOUNT)
		{
			fprintf(stderr, "MAX_FCOUNT of %d exceeded\n", 
				MAX_FCOUNT);
			return(FALSE);
		}	
	}
	closedir(dirptr);
	qsort(filename,filecounters,sizeof(FILE *), cmp_name);
	if(strcmp(dir, ".") != 0)
		printf("%s: \n", dir);
	if(showlong)
		printf("total %d\n", total);
	for(i = 0; i < filecounters; i++)
	{
		print_file(filename[i]->name,dir,filename[i]->buf, showsize,
			   showlong);
		free(filename[i]);
	}
	free(filename);
}
/***********************************************************************/
/* Function Name: print_file                                            */
/*								       */
/* Description: ls style ouput of files				       */ 
/***********************************************************************/
int print_file(char *file, char *dir, struct stat buf, int showsize, int showlong)
{
	if(showsize)
		printf("%4ld",buf.st_blocks);
	if(showlong)
		print_long(file, dir,buf);
	else
		printf("%s\n", file);
} 
/**************************************************************************/
/* Function Name: print_long 	                                          */
/*                                                                        */
/* Description: display file name in the -l form (ls -l) 		  */
/**************************************************************************/
int print_long(char *name, char *dir, struct stat buf)
{
	int num; 	/*# of bytes in readlink*/
	char perm[16];	/*Permissions in string(-rwxrwxrwx)*/

	char temp[256]; /*temp filename*/
	char link[256]; /*symbolic link traget*/

	char owner[32];
	char group[32];
	struct passwd *pw;
	struct group *gr;

	char mtime[32];
	time_t now;
	struct tm *tm, *tnow;
	
	if(S_ISLNK(buf.st_mode))
	{
		if(dir != NULL)
		sprintf(temp, "%s/%s", dir, name);
		else 
			strcpy(temp,name);
		if((num = readlink(temp,link,256))>=0)
		{
			link[num] = 0;
			sprintf(name,"%s->%s",name, link);
		}
	}
	list_permissions(perm, buf.st_mode);
	if((pw = getpwuid(buf.st_uid))== NULL)
		sprintf(owner, "%d", buf.st_uid);
	else
		strcpy(owner, pw->pw_name);
	if((gr = getgrgid(buf.st_gid)) == NULL)
		sprintf(group, "%d", buf.st_gid);
	else
		strcpy(group, gr->gr_name);

	now = time(0);
	tnow = localtime(&now);
	
	tm = localtime(&(buf.st_mtime));
	
	if(buf.st_mtime < now - SEC_PER_YEAR)
		strftime(mtime, 32,"%b %d %Y", tm);
	else
		strftime(mtime, 32, "%b %d %H:%M", tm);

	if(S_ISCHR(buf.st_mode)|| S_ISBLK(buf.st_mode))
	{
		printf("%s %lud %-8s %-9s %3d, %3d %s %s\n", perm, buf.st_nlink, 
			   owner, group,major(buf.st_rdev), minor(buf.st_rdev),mtime,name);
	}
	else 
	{
		printf("%s %lud %-8s %-9s %7ld %s %s\n", perm, buf.st_nlink, owner,
			   group, buf.st_size, mtime, name);
	}
}
/*************************************************************************/
/* Function Name: list_permissions					 */
/*      								 */
/*Description: the development of permissions string for the -l format   */
/*************************************************************************/
int list_permissions(char *perm, mode_t m)
{
	char ftype; 
	if(S_ISREG(m))
		ftype = '-';
	else if(S_ISDIR(m))
		ftype = 'd';
	else if(S_ISLNK(m))
		ftype = 'l';
	else if(S_ISFIFO(m))
		ftype = 'p';
	else if(S_ISBLK(m))
		ftype = 'b';
	else if(S_ISCHR(m))
		ftype = 'c';
	else if(S_ISSOCK(m))
		ftype = 's';
	sprintf(perm, "%c%c%c%c%c%c%c%c%c%c", ftype,
		(m & S_IRUSR) ? 'r' : '-', 
		(m & S_IWUSR) ? 'w' : '-',
		(m & S_IXUSR) ? 'x' : '-', 
		(m & S_IRGRP) ? 'r' : '-', 
		(m & S_IWGRP) ? 'w' : '-', 
		(m & S_IXGRP) ? 'x' : '-', 
		(m & S_IROTH) ? 'r' : '-',
		(m & S_IWOTH) ? 'w' : '-', 
		(m & S_IXOTH) ? 'x' : '-');
	if(m & S_ISUID)
		perm[3] = (m & S_IXUSR) ? 's': 'S';
	if(m & S_ISGID)
		perm[6] = (m & S_IXGRP) ? 's': 'S'; 
	if(m & S_ISVTX) 
		perm[9] = (m & S_IXOTH) ? 't' : 'T';
	return TRUE;  
}
/************************************************************/
/* Function Name: compare_fileNames			    */
/*							    */
/* Description: comparison of two file entries 		    */
/************************************************************/
int compare_fileNames(file **a, file **b)
{
	file *one, *two; 
	
	one = *a;
	two = *b; 
	
	return (strcmp(one->name, two->name));
}

