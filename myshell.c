
#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

// #define CMD_LEN 1024
#define ARGS_LEN 10 // Max Argument length
#define MAX_CMDS 10 // Max number of commands

int parseInput(char *str,char *commands[])
{
	/* This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).*/
    char* tmp=str;
    int i;
    //parsing &&
    for (i = 0; i < MAX_CMDS; i++) 
    {
        commands[i] = strsep(&str, "&&");
        if (commands[i] == NULL)
        {
            break;
        }
        if(strlen(commands[i])==0)
        {
            i--;
        }
    }
    if(commands[1] != NULL)
    {
        return 1;
    }
    str=tmp;

    //parsing ##
    for (i = 0; i < MAX_CMDS; i++)
    {
        commands[i] = strsep(&str, "##");
        if (commands[i] == NULL)
        {
           break;
        }
        if(strlen(commands[i]) == 0)
        {
            i--;
        }
    }
    if(commands[1] != NULL)
    {
        return 2;
    }
    str=tmp;

    //parsing >
    for (i = 0; i < MAX_CMDS; i++)
    {
        commands[i] = strsep(&str, ">");
        if (commands[i] == NULL)
        {
           break;
        }
        if(strlen(commands[i]) == 0)
        {
            i--;
        }
    }
    if(commands[1] != NULL)
    {
        return 3;
    }
    return 0;
}

void executeCommand(char *cmd)
{
	// This function will fork a new process to execute a command
    int i;
    char *args[ARGS_LEN];
    for (i = 0; i < ARGS_LEN; i++) 
    {
        args[i] = strsep(&cmd, " ");
        if (args[i] == NULL)
        {
            break;
        }
    }

    if(strcmp(args[0],"cd")==0)
    {
        // Change working directory to mentioned directory
        if(chdir(args[1])<0)
        {
            printf("Shell: Incorrect command\n"); // If chdir fails
        }
    }
    else
    {
        int pid=fork(); // forking a child process
        if(pid<0) // fork unsuccessful
        {
          printf("Forking failed");
        }
        else if(pid==0) // fork successful 
        {
            // Signal handling
			// Restoring default behaviour for Ctrl-C and Ctrl-Z
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP,SIG_DFL);
            if(execvp(args[0],args)<0) // incorrect command
            {
                printf("Shell: Incorrect command\n");
                exit(0);
            }
        }
        else // parent process
        {
            // parent process should wait till child process completes
            int rc_wait = waitpid(pid, NULL, WUNTRACED);
        }
    }
}

void executeParallelCommands(char *commands[])
{
	// This function will run multiple commands in parallel

    int i=0;
    int j;
    char *args[ARGS_LEN];
    while(commands[i]!=NULL)
    {
        for (j= 0; j < ARGS_LEN; j++) 
        {
           args[j] = strsep(&commands[i], " ");
           if (args[j] == NULL)
           {
              break;
           }
           if(strlen(args[j]) == 0)
           {
               j--;
           }
        }
        if(strcmp(args[0],"cd")==0)
        {
            if(chdir(args[1])<0)
            {
                printf("Shell: Incorrect command\n");
            }
        }
        else
        {
            int pid=fork(); // Forking
            if(pid<0) // If fork fails
            {
               printf("forking failed");
            }
            else if(pid==0) // Fork succeeds
            {
                // Signal handling
			    // Restoring the default behaviour for Ctrl-C and Ctrl-Z signal
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP,SIG_DFL);
                //Executing command
                if(execvp(args[0],args)<0)
                {
                    printf("Shell: Incorrect command\n");
                    exit(0);
                }
            }
        }
        i++;
    }
    while(wait(NULL)>0);
}

void executeSequentialCommands(char *commands[])
{	
	// This function will run multiple commands in parallel
    int i=0;
    int j;
    char *args[ARGS_LEN];

    // Checking all words in command entered in terminal
    while(commands[i]!=NULL)
    {
        for (j= 0; j < ARGS_LEN; j++) 
        {
           args[j] = strsep(&commands[i], " ");
           if (args[j] == NULL)
           {
              break;
           }
           if(strlen(args[j]) == 0)
           {
               j--;
           }
        }
        if(strcmp(args[0],"cd")==0)
        {
            if(chdir(args[1])<0)
            {
                printf("Shell: Incorrect command\n");
            }
        }
        else
        {
            int pid=fork();
            if(pid<0)
            {
               printf("forking failed");
            }
            else if(pid==0)
            {
               signal(SIGINT, SIG_DFL);
               signal(SIGTSTP,SIG_DFL);
               if(execvp(args[0],args)<0)
               {
                  printf("Shell: Incorrect command\n");
                  exit(0);
               }
            }
            else
            {
               wait(NULL);
            }
        }
        i++;
    }
}

void executeCommandRedirection(char *commands[])
{
	// This function will run a single command with output redirected to an output file specificed by user
    char *args[ARGS_LEN];
    if(commands[1][0]==' ')
    {
        commands[1]=commands[1]+1;
    }
    int j;
    for (j= 0; j < ARGS_LEN; j++) 
    {
        args[j] = strsep(&commands[0], " ");
        if (args[j] == NULL)
        {
            break;
        }
        if(strlen(args[j]) == 0)
        {
            j--;
        }
    }
    int pid=fork(); // forking the child
    if(pid<0) // fork unsuccessful
    {
        printf("forking failed");
    }
    else if(pid==0) // fork successful - child
    {
        // redirecting stdout
        close(STDOUT_FILENO);
        // Restoring default behaviour for Ctrl-C and Ctrl-Z
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP,SIG_DFL);
		open(commands[1], O_CREAT | O_WRONLY);
		if(execvp(args[0], args)<0) // executing command
        {
            printf("Shell: Incorrect command\n");
            exit(0);
        }
    }
    else
    {
        // Waiting till parent process completes
        int rc_wait = waitpid(pid, NULL, WUNTRACED);
    }
}

int main()
{
	// Initial declarations
    char inputString[1024];
    char *ptr=inputString;
    size_t size=1024;
    char *commands[MAX_CMDS];
    char dir[1024]; // Variable to store current working directory
    signal(SIGINT, SIG_IGN); // Ignore signal interrupt (Ctrl+C)
    signal(SIGTSTP,SIG_IGN); // Ignore signal of suspending execution (Ctrl+Z)

    // This loop will keep your shell running until user exits.
	while(1)
    {
		// Print the prompt in format - currentWorkingDirectory$
        getcwd(dir,sizeof(dir)); //function that return the current working directory and store it in the dir variable
        printf("%s$",dir); // prints the current directory 

        // accept input with 'getline()'
        getline(&ptr,&size,stdin);
        
        int len=strlen(inputString);
        inputString[len-1]='\0'; //removing \n which is read from the command prompt

        if(strcmp(inputString,"exit")==0)  // When user uses exit command.
        {
            printf("Exiting shell...\n");
            exit(0);
        }
		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		int result=parseInput(inputString,commands); 	// To keep track of command to be executed
		if(result==1)
        {
            // This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
			executeParallelCommands(commands);		
        }
		else if(result==2)
        {
            // This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
			executeSequentialCommands(commands);	
        }
		else if(result==3)
        {
            // This function is invoked when user wants redirect output of a single command to and output file specificed by user
			executeCommandRedirection(commands);	
        }
		else
        {
            // This function is invoked when user wants to run a single commands
			executeCommand(commands[0]);		    
        }
	}
	return 0;
}