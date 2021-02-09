//Written By: Jack Petersen
//Date Created: 1/27/2021
//Email: peterja7@oregonstate.edu
//This program acts as a shell and will perform basic 
//shell commands
#include "smallsh.h"
//globals
int exitStatus = 0;
int allowBG;
int main()
{
	//Set up signal handlers
	//based on code from Explortation: Signal Handling API
	struct sigaction SIGINT_action = {0};
        SIGINT_action.sa_handler = handle_SIGINT;
        sigfillset(&SIGINT_action.sa_mask);
        SIGINT_action.sa_flags = SA_RESTART;
	struct sigaction SIGTSTP_action = {0};
        SIGTSTP_action.sa_handler = handle_SIGTSTP;
        sigfillset(&SIGTSTP_action.sa_mask);
        SIGTSTP_action.sa_flags = SA_RESTART;

 	//call signal handlers       
        sigaction(SIGINT, &SIGINT_action, NULL);
        sigaction(SIGTSTP, &SIGTSTP_action, NULL);
	
	int i;
	int childStatus = 0;
	int execStatus = 0;
	struct input * currInput = getInput();
	//initialze background pid array
	for(i = 0; i <199; i++)
	{
		currInput->bgProcess[i] = -5;
	}
	currInput->processNum = 0; 
	while(exitStatus != 1)
	{
		//check to see if user input was blank or a comment
		if(currInput->flag == 1)
		{
			currInput->flag = 0;
			init(currInput);
			getInput();
		}
		//user entered cd
		else if(strcmp(currInput->args[0], "cd") == 0)
		{
			cdCommand(currInput);
		}
		//user enetered status
		else if(strcmp(currInput->args[0], "status") == 0)
		{
			status(childStatus);
		}
		//kill background proccesses and exit program
		else if(strcmp(currInput->args[0], "exit") == 0)
		{
			exitShell(currInput);
		}
		//fork a new procceses and exec() the commands
		else
		{
			pid_t spawnPid = -5;
			spawnPid = fork();
			switch(spawnPid)
			{
				case -1:
					perror("fork() error");
					fflush(stderr);
					exit(1);
					break;
				case 0:
						
					//foreground process
					if(currInput->isBG ==0)
					{
						signal(SIGINT,SIG_DFL);
						
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

					}

					//background process
					else
					{
						bgProcess(currInput);
						if(currInput->inFile != NULL && currInput->outFile != NULL)
                                                {
                                                        inputFile(currInput);
                                                        outputFile(currInput);
                                                }
						
						else if(currInput->inFile != NULL)
                                                {
                                                        inputFile(currInput);
                                                }

						else if(currInput->outFile != NULL)
                                                {
                                                        outputFile(currInput);
                                                }
						
					}
					//Execute commands
					execStatus = execvp(currInput->args[0], currInput->args);
					if(execStatus == -1)
					{
						perror("error");
						fflush(stderr);
						exit(1);
						break;
					}
				//is parent
					
				default:
					//is running in background
					if(currInput->isBG == 1)
					{
						//add background PID to array
						currInput->bgProcess[currInput->processNum] = spawnPid;
						currInput->processNum ++;
						printf("background process PID is %d\n", spawnPid);
						fflush(stdout);
                                        }
						
					else
					{
                                                spawnPid = waitpid(spawnPid, &childStatus, 0);
                                        }			
			}
			
		}
                                               
		//check for exiting background processes and report pid and status if they exit
		pid_t bgPID;
                int bgStatus;
                int j;
                for(j = 0;j < 199; j++)
                {
                	bgPID = waitpid(currInput->bgProcess[j], &bgStatus, WNOHANG);
                        if(bgPID != -1 && bgPID != 0)
                        {
                        	if(WIFEXITED(bgStatus))
                                {
                                	printf("background pid %d is done: exit status %d\n", bgPID, WEXITSTATUS(bgStatus));
                                        fflush(stdout);
                                        currInput->bgProcess[j] = 0;
                                }
				else if(WIFSIGNALED(bgStatus))
				{
					printf("background pid %d is done: terminated by signal %d\n", bgPID, WTERMSIG(bgStatus));
				}
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
	currInput->args[0] = strdup(token);
	token = strtok_r(NULL, " \n", &savePtr);
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
		
		else
		{
			currInput->args[i] = strdup(token);
			i++;
		}
		token = strtok_r(NULL, " \n", &savePtr);
	
	}

	
	//check if last argument is &
	if(strcmp(currInput->args[i-1], "&") == 0)
	{
		if(allowBG == 0)
		{
			currInput->isBG = 1;
		}
		else
		{
			currInput->isBG = 0;
		}
		currInput->args[i-1] = '\0';	
	}

        return currInput;
}

//gets input
struct input *getInput()
{
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
//free memory
void freeAll(struct input * currInput)
{
	struct input *temp = currInput;
	free(temp->inFile);
	free(temp->outFile);
	free(temp);
}
// change directory command implementation
void cdCommand(struct input * currInput)
{
	if(currInput->args[1] == NULL)

	{
		chdir(getenv("HOME"));
	}
	else
	{
		chdir(currInput->args[1]);
		
	}
}

//kills all running processes and exits program
void exitShell(struct input * currInput)
{
	int i;
	for(i = 0; i < 199; i++)
	{
		if(currInput->bgProcess[i] != 0)
		{
			kill(*currInput->bgProcess[i], SIGTERM);
		}
	}
	exitStatus = 0;
	exit(0);
}
//displays status or terminating signal of the last foreground process ran by the shell
void status(int childStatus)
{
	//if childStatus has not been set
	if(childStatus == 0)
	{
		printf("Exit Status: 0\n");
		fflush(stdout);
	}

	else if(WIFEXITED(childStatus) != 0)
	{
		printf("Exit Status: %d\n", WEXITSTATUS(childStatus));
		fflush(stdout);
	}
	else if(WIFSIGNALED(childStatus) != 0)
	{
		printf("Process terminated pid signal %d\n", WTERMSIG(childStatus));
		fflush(stdout);
	}
}
	
	 		
// initialize all elements of the command and argument array to NULL as well as other vars
void init(struct input * currInput)
{
	int i;
	for(i = 0; i < 513; i++)
	{
		currInput->args[i] = NULL;
		i++;
	}

	currInput->inFile = NULL;
	currInput->outFile = NULL;
	currInput->flag = 0;
	currInput->isBG = 0;

}

//this function handles file input redirection
void inputFile(struct input * currInput)
{
	int infile = 0;
	//open file for reading
	infile = open(currInput->inFile, O_RDONLY);
	if(infile == -1)
	{
		printf("Cannot open file %s for reading\n", currInput->inFile);
		fflush(stdout);
	}
	dup2(infile,STDIN_FILENO);
	fcntl(infile, F_SETFD, FD_CLOEXEC);
}
//this function handles foreground outfput file redirection
void outputFile(struct input * currInput)
{
	int dupErr = 0;
        int outfile = 0;
	
	outfile = open(currInput->outFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
        if(outfile == -1)
        {
                printf("Cannot open file %s for writing\n", currInput->outFile);
		fflush(stdout);
        }
        dupErr = dup2(outfile,STDOUT_FILENO);
	if(dupErr == -1)
	{
		printf("dup2 error");
		fflush(stdout);
	}
	fcntl(outfile, F_SETFD, FD_CLOEXEC);
}

//this function handles background input redirection
void bgProcess(struct input * currInput)
{
	int dupErr = 0;
	int fin = 0;
	int fout = 0;
	//open /dev/null for reading
	fin = open("/dev/null", O_RDONLY);
	if(fin == -1)
	{
		printf("Cannot open /dev/null for input\n");
		fflush(stdout);

	}
	dupErr = dup2(fin, STDIN_FILENO);
	if(dupErr == -1)
	{
		printf("Error pid dup2\n");
		fflush(stdout);

	}
	 fcntl(fin, F_SETFD, FD_CLOEXEC);
	//open /dev/null for writing
	fout = open("/dev/null",  O_WRONLY | O_CREAT | O_TRUNC, 0640);
	if(fout == -1)
        {
                printf("Cannot open /dev/null for input\n");
		fflush(stdout);

        }
        dupErr = dup2(fout, STDOUT_FILENO);
        if(dupErr == -1)
        {
                printf("Error pid dup2\n");
		fflush(stdout);

        }
	fcntl(fout, F_SETFD, FD_CLOEXEC);
}

//this function replaces all instances of $$ pid the pid of the current proccess
//Sources: https://www.tutorialspoint.com/c-program-to-replace-a-word-in-a-text-by-another-given-word
char * strReplace(char * buffer, char *replace, char * pid)
{
	char * result;
	int i = 0;
	int count = 0;
	int pidLen = strlen(pid);

	for(i = 0; buffer[i] != '\0'; i++)
	{
		if(strstr(&buffer[i], replace) != NULL)
		{
			count++;
			i++;
		}
	}
			
	result = malloc(i + count * (pidLen - 2) + 1 *sizeof(char));
	i = 0;
	while(*buffer)
	{
		if(strstr(buffer,replace) == buffer)
		{
			strcpy(&result[i], pid);
			i+= pidLen;
			buffer += 2;
		}
		else
		{
			result[i++] = *buffer++;
		}
	}
	result[i] = '\0';
	return result;
}	

//sigint handler
void handle_SIGINT(int signo)
{
	//ignore sigint unless otherwise specified
	signal(SIGINT, SIG_IGN);

}
//sigstp handler
void handle_SIGTSTP(int signo)
{
	//background is currently allowed
	if(allowBG == 0)
	{
		allowBG = 1;
		char * message = "\nEntering foreground only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, 50);
		fflush(stdout);
		write(STDOUT_FILENO, ": \n", 3);
		fflush(stdout);
	}
		
	// background is currently not allowed
	else
	{
		allowBG = 0;
		char * message = "\nExiting foreground only mode\n";
		write(STDOUT_FILENO, message, 31);
		fflush(stdout);
		write(STDOUT_FILENO, ": \n", 3);
                fflush(stdout);

	}
}	
