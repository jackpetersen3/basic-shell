//Written By: Jack Petersen
//Date Created: 1/27/2021
//Email: peterja7@oregonstate.edu
//This program acts as a shell and will perform basic 
//shell commands
#include "smallsh.h"
int exitFlag = 0;
struct input *getInput();
struct input *parseInput(char * buffer);
void cdCommand(struct input *currInput);
void init(struct input *currInput);
void inputFile(struct input *currInput);
void outputFile(struct input * currInput);
char * strReplace(char * buffer, char *replace, char * with);

int main()
{
	struct input * currInput = getInput();
	   
	while(exitFlag != 1)
	{
		//check to see if user input was blank or a comment
		if(currInput->flag == 1)
		{
			currInput->flag = 0;
			init(currInput);
			getInput();
		}
		//user entered cd
		else if(strcmp(currInput->commandArgc[0], "cd") == 0)
		{
			cdCommand(currInput);
		}
		//kill background proccesses and exit program
		else if(strcmp(currInput->commandArgc[0], "exit") == 0)
		{
			exit(1);
		}
		//fork a new procceses and exec() the commands
		else
		{
			pid_t spawnPid = -5;
			int childStatus;
			int execStatus;
			spawnPid = fork();
			switch(spawnPid)
			{
				case -1:
					perror("fork() failed");
					exit(1);
					break;
				case 0:
					//check for both input and putput redirection
					if(currInput->inFile != NULL && currInput->outFile != NULL)
			                {
                        			inputFile(currInput);
                        			outputFile(currInput);
                        		}
					//check for input redirection
		                        else if(currInput->inFile != NULL)
		                        {
                		                inputFile(currInput);
                        		}
					//check for output redirection
					else if(currInput->outFile != NULL)
		                        {
                		                outputFile(currInput);
                        		}
					
					execStatus = execvp(currInput->commandArgc[0], currInput->commandArgc);
					if(execStatus == -1)
					{
						perror("EXEC FAILED");
						exit(1);
						break;
					}

					
				default:
					spawnPid = waitpid(spawnPid, &childStatus, 0);
				//	printf("PARENT(%d): child(%d)\n", getpid(), spawnPid);
			}
		}
		//get Input
		init(currInput);
		currInput = getInput();
	}
	return 0;
}


//This function parses through the input line
struct input *parseInput(char * buffer)
{
	int pid;
	char myPid[30];
	int i = 1;
        struct input *currInput = malloc(sizeof(struct input));
        char *savePtr = NULL;
	//get command
	if(strstr(buffer, "$$") != NULL)
	{
		pid = getpid();
		sprintf(myPid, "%d", pid);
		buffer = strReplace(buffer,"$$", myPid);
	}
	
        char * token = strtok_r(buffer," ", &savePtr);
	currInput->commandArgc[0] = strdup(token);
	token = strtok_r(NULL, " \n", &savePtr);
//	printf("%s", token);
	while(token != '\0')
	{
		if(strcmp(token, "<") == 0)
		{
			token = strtok_r(NULL, " \n", &savePtr);
			currInput->inFile = calloc(strlen(token)+1, sizeof(char));
			strcpy(currInput->inFile, token);
		}
		//check if next token is > symbol
                else if(strcmp(token, ">") == 0)
                {
                        token = strtok_r(NULL, " \n", &savePtr);
                        currInput->outFile = calloc(strlen(token)+1, sizeof(char));
                        strcpy(currInput->outFile, token);
                }
		//==================================================================
		////testing only
		//==================================================================
		else if(strcmp(token, "exit") == 0)
		{
			exitFlag = 1;
		}
		//=================================================================
		else
		{
			currInput->commandArgc[i] = strdup(token);
			i++;
		}
		token = strtok_r(NULL, " \n", &savePtr);
	

	}
//	printf("%s", currInput->commandArgc[i-1]);
	
	//check if last argument is &
	if(strcmp(currInput->commandArgc[i-1], "&") == 0)
	{
		currInput->ampersand = 1;
		printf("Background Proccess");
	}

        return currInput;
}

//gets input
struct input *getInput()
{
	int pid = getpid();
        char *buffer = NULL;
        size_t len = 0;
        ssize_t nRead = 0;
        printf(": ");
	fflush(stdout);
        nRead = getline(&buffer, &len, stdin);
	if(strlen(buffer) == 1)
	{
		buffer[0] = '#';
		
	}
	else
	{
		buffer[nRead -1] = '\0';
	}
	struct input *currLine = parseInput(buffer);
	if(buffer[0] == '#')
        {
                currLine->flag =1;
        }
	free(buffer);
	return currLine;

}
// change directory command implementation
void cdCommand(struct input * currInput)
{
	if(currInput->commandArgc[1] == NULL)
	{
		chdir(getenv("HOME"));
		printf("hello");
	}
	else
	{
		chdir(currInput->commandArgc[1]);
		printf("goodbye");
		
	}
}
// initialize all elements of the command and argument array to NULL
void init(struct input * currInput)
{
	int i;
	for(i = 0; i < 513; i++)
	{
		currInput->commandArgc[i] = NULL;
		i++;
	}

	currInput->inFile = NULL;
	currInput->outFile = NULL;
	currInput->flag = 0;
	currInput->ampersand = 0;

}

//this program handes file input redirection
void inputFile(struct input * currInput)
{
	int infile = 0;
	//open file for reading
	infile = open(currInput->inFile, O_RDONLY);
	if(infile == -1)
	{
		printf("Cannot open file %s for reading\n", currInput->inFile);
	}
	dup2(infile,STDIN_FILENO);
	fcntl(infile, F_SETFD, FD_CLOEXEC);
}

void outputFile(struct input * currInput)
{
	int dupErr = 0;
        int outfile = 0;
	
	outfile = open(currInput->outFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
        if(outfile == -1)
        {
                printf("Cannot open file %s for writing\n", currInput->outFile);
        }
        dupErr = dup2(outfile,STDOUT_FILENO);
	if(dupErr == -1)
	{
		printf("dup2 error");
	}
	fcntl(outfile, F_SETFD, FD_CLOEXEC);
}

//this function replaces all instances of $$ with the pid of the current proccess
//===============================================================================
//FIX BEFORE TURNING IN
//===============================================================================
char * strReplace(char * buffer, char *replace, char * with)
{
	char * result;
	int i, count = 0;
	int len = strlen(replace);
	int pidLen = strlen(with);

	for(i = 0; buffer[i] != '\0'; i++)
	{
		if(strstr(&buffer[i], replace) == &buffer[i])
		{
			count++;
			i += len-1;
		}
	}
			
	result = (char*)malloc(i + count * (pidLen - len) + 1);
	i = 0;
	while(*buffer)
	{
		if(strstr(buffer,replace) == buffer)
		{
			strcpy(&result[i], with);
			i+= pidLen;
			buffer += len;
		}
		else
		{
			result[i++] = *buffer++;
		}
	}
	result[i] = '\0';
	return result;
}		 
