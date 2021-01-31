//Written By: Jack Petersen
//Date Created: 1/27/2021
//Email: peterja7@oregonstate.edu
//This program acts as a shell and will perform basic 
//shell commands
#include "smallsh.h"
int exitFlag = 0;
struct input *getInput();
struct input *parseInput(char * buffer);
int main()
{
	struct input * currInput = getInput();
    
	while(strcmp(currInput->command, "exit") != 0)
	{
		currInput = getInput();
//		printf("%s", currInput->command);	
	}
}


//This function parses through the input line
struct input *parseInput(char * buffer)
{
	int i = 0;
        if(strcmp(buffer, " ") == 0)
        {
                getInput();
        }
        struct input *currInput = malloc(sizeof(struct input));
        char *savePtr = NULL;
	//get command
        char * token = strtok_r(buffer, " ", &savePtr);
	currInput->command = calloc(strlen(token)+1, sizeof(char));
        strcpy(currInput->command, token);
	token = strtok_r(NULL, " ", &savePtr);
//	printf("%s", currInput->command);
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
		else
		{
			currInput->argc[i] = token;
			i++;
		}
		token = strtok_r(NULL, " ", &savePtr);

	}
	printf("%s", currInput->inFile);
	printf("\n");
	printf("%s", currInput->argc[i]);



        return currInput;
}

//gets input
struct input *getInput()
{
        char *buffer = NULL;
        size_t len = 0;
        ssize_t nRead = 0;
        printf(": ");
        nRead = getline(&buffer, &len, stdin);
	if(buffer[0] == '#')
	{
		getInput();
	}
        struct input *currLine = parseInput(buffer);
        return currLine;

}

