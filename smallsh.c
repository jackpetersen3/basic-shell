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
		//get Input
		init(currInput);
		currInput = getInput();
	}
}


//This function parses through the input line
struct input *parseInput(char * buffer)
{
	int i = 1;
        if(strcmp(buffer, " ") == 0)
        {
                getInput();
        }
        struct input *currInput = malloc(sizeof(struct input));
        char *savePtr = NULL;
	//get command
        char * token = strtok_r(buffer, " ", &savePtr);
  	currInput->commandArgc[0] = strdup(token);
	token = strtok_r(NULL, " ", &savePtr);
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
		else if(strcmp(currInput->commandArgc[0], "exit") == 0)
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
	printf("%s", currInput->commandArgc[i-1]);
	
	//check if last argument is &
	if(strcmp(currInput->commandArgc[i-1], "&\n") == 0)
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
	struct input *currLine = parseInput(buffer);

	if(buffer[0] == '#')
	{
		currLine->flag = 1;
	}
	else if(strlen(buffer) == 1)
	{
		printf("BLANK LINE");
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
}
