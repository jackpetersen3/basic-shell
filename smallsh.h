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
#include <signal.h>

struct input{
	char *commandArgc[513];
	char *inFile;
	char *outFile;
	int *bgProcess[513];
	int processNum;
	int ampersand;
	int flag;
};


struct input *getInput();
struct input *parseInput(char * buffer);
void cdCommand(struct input *currInput);
void exitShell(struct input * currInput);
void status(int childStatus);
void init(struct input *currInput);
void inputFile(struct input *currInput);
void outputFile(struct input * currInput);
void bgProcess(struct input * currInput);
char * strReplace(char * buffer, char *replace, char * with);
//void signalSetup();
void handle_SIGINT();
void handle_SIGTSTP();
void freeAll(struct input *currInput);
