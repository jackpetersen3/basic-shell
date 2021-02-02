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
		else if(strncmp(currInput->commandArgc[0], "cd", 2) == 0)
		{
			cdCommand(currInput);
		}
		else
		{
			pid_t spawnPid = -5;
			int childStatus;
			int execStatus;
			spawnPid = fork();
			switch(spawnPid)
			{
				case -1:
					printf("fork Failed");
					perror("fork() failed");
					exit(1);
					break;
				case 0:
					
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
	int i = 1;
        struct input *currInput = malloc(sizeof(struct input));
        char *savePtr = NULL;
	//get command
        char * token = strtok_r(buffer," ", &savePtr);
	currInput->commandArgc[0] = strdup(token);
//	printf("%s", token);
	while(token != '\0')
	{
		if(strcmp(token, "<") == 0)
		{
			token = strtok_r(NULL, " ", &savePtr);
			currInput->inFile = calloc(strlen(token)+1, sizeof(char));
			strcpy(currInput->inFile, token);
		}
		//check if next token is > symbol
                else if(strcmp(token, ">") == 0)
                {
                        token = strtok_r(NULL, " ", &savePtr);
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
		token = strtok_r(NULL, " ", &savePtr);

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
//	fflush(stdout);
        nRead = getline(&buffer, &len, stdin);
	if(strlen(buffer) == 1)
	{
		getInput();
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
