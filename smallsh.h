#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

struct input{
	char *commandArgc[513];
	char *inFile;
	char * outFile;
	int * bgProcess[513];
	int processNum;
	int ampersand;
	int flag;
	int allowBG;
};

