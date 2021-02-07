//Written By: Jack Petersen
//Date Created: 1/27/2021
//Email: peterja7@oregonstate.edu
//This program acts as a shell and will perform basic 
//shell commands
#include "smallsh.h"
int exitFlag = 0;
int allowBG;
struct input *getInput();
struct input *parseInput(char * buffer);
void cdCommand(struct input *currInput);
void exit(struct input * currInput);
void init(struct input *currInput);
void inputFile(struct input *currInput);
void outputFile(struct input * currInput);
void bgProcess(struct input * currInput);
char * strReplace(char * buffer, char *replace, char * with);
//void signalSetup();
void handle_SIGINT();
void handle_SIGTSTP();
int main()
{
	struct sigaction SIGINT_action = {0};
        SIGINT_action.sa_handler = SIG_DFL;
        sigfillset(&SIGINT_action.sa_mask);
        SIGINT_action.sa_flags = 0;
	struct sigaction SIGTSTP_action = {0};
        SIGTSTP_action.sa_handler = handle_SIGTSTP;
        sigfillset(&SIGTSTP_action.sa_mask);
        SIGTSTP_action.sa_flags = SA_RESTART;

        struct sigaction ignore_action = {0};
        ignore_action.sa_handler = SIG_IGN;

        sigaction(SIGINT, &ignore_action, NULL);
        sigaction(SIGTSTP, &SIGTSTP_action, NULL);
	
	int i;
	struct input * currInput = getInput();
//	signalSetup();
	for(i = 0; i <512; i++)
	{
		currInput->bgProcess[i] = '\0';
	}
	currInput->processNum = 0; 
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
					fflush(stderr);
					exit(1);
					break;
				case 0:
					sigaction(SIGTSTP, &ignore_action, NULL);
					//foreground process
					if(currInput->ampersand ==0)
					{	
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
					execStatus = execvp(currInput->commandArgc[0], currInput->commandArgc);
					if(execStatus == -1)
					{
						perror("EXEC FAILED");
						fflush(stderr);
						exit(1);
						break;
					}

					
				default:
					if(currInput->ampersand == 1)
					{
						//add background PID to array
						currInput->bgProcess[currInput->processNum] = spawnPid;
						currInput->processNum ++;
						printf("Background process PID is %d\n", spawnPid);
						fflush(stdout);
					}
						
					else
					{
						spawnPid = waitpid(spawnPid, &childStatus, 0);
					}
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
		if(allowBG == 0)
		{
			currInput->ampersand = 1;
		}
		else
		{
			currInput->ampersand = 0;
		}
		//printf("Background Proccess");
		currInput->commandArgc[i-1] = '\0';	
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

//kills all running processes and exits program
//{
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
		printf("Error with dup2\n");
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
                printf("Error with dup2\n");
		fflush(stdout);

        }
	fcntl(fout, F_SETFD, FD_CLOEXEC);
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

//this function sets up signal handlers
//Much of this code is based on Exploration: Signal Handling API from the class lecture
/*void signalSetup()
{
	//signal setup for sigint
	struct sigaction SIGINT_action = {0};
	SIGINT_action.sa_handler = SIG_DFL;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
//	sigaction(SIGINT, &SIGINT_action, NULL);
	
	//signal setup for sigstp
	struct sigaction SIGTSTP_action = {0};
	SIGTSTP_action.sa_handler = handle_SIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;

	struct sigaction ignore_action = {0};
	ignore_action.sa_handler = SIG_IGN;

	sigaction(SIGINT, &ignore_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);
	
	
 }*/
//sigint handler
void handle_SIGINT(int signo)
{
	write(STDOUT_FILENO, "CAUGHT SIGINT\n", 13);
	fflush(stdout);
	sleep(2);
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
