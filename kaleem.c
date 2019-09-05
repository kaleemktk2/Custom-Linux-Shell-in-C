#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <signal.h>
#include <setjmp.h>
#include <readline/history.h>
#define clear() printf("\033[H\033[J") // for clearing the screen
#define MAXCOM 1000
#define MAXLIST 100

jmp_buf env;			//used for sigjump

//List_Functions I used 
int exit_flag=0;
char input[MAXCOM];
int takeinput(char* str, char* printit);
char **get_input(char *);
void init_shell();
int cd(char *);
int other_command(char **command);	//if Not in /bin then its other cmd
void refresh(int sigint){		
					//CTRL+C refresh the terminal......
	printf(" refresh \n");
	siglongjmp(env,10);
}
int main(){

	int pid_start = fork();
	if(pid_start == 0){
		init_shell();		//child welcomes u....
		exit(0);
	}

    	char **command;    
	char cwd[PATH_MAX];	// holds the PWD
	char cli_line[50]="";   
	char user_name[10]="";
	pid_t child_pid;
    	int stat_loc;
	struct sigaction newact;
	newact.sa_handler = refresh;
	newact.sa_flags = 0;
	sigaction(SIGINT,&newact,NULL);		// CTRL+C refresh dont kill !
	
	using_history();			// starting history
    	while (1){       
		getlogin_r(user_name,10);	// get username
		getcwd(cwd,sizeof(cwd));	// get PWD
		sprintf(cli_line,"User_Name Sifat & %s:~%s$ ",user_name,cwd);
		sigsetjmp(env,10);
		if(takeinput(input,cli_line))	// return 0 if user enters a cmd
			continue;
		
		if(input == NULL){		// check for ctrl+D(NULL input)
			printf("\n");
			break;
			exit(0);
		}


		int count =0;
		char input2[MAXCOM];
		strcpy(input2,input);
                for(count=0;input2[count]!='\0';count++){}	// size of cmd
                
		int pipe_flag=0,pipe_index=0;
                
		for(int i=0; i<count; i++){		// check for pipes....

			if(input2[i]=='|'){
                                pipe_flag  = 1;
				pipe_index = i;		// Note the | pipe index..
                                break;
                        }
                }
                char pipe_commands[2][20];
                if(pipe_flag){
		int head_pid=fork();
		if(head_pid==0){
			strncpy(pipe_commands[0],input2,pipe_index);		//LHS of pipe
			strncpy(pipe_commands[1],input2+pipe_index+1,count);	// RHS of pipe
			
			const char s[2]=" ";	//seperator.....
			
			char **cmd1,**cmd2;
			cmd1 = get_input(pipe_commands[0]);
			cmd2 = get_input(pipe_commands[1]);
			
			int fd_pipe[2];
			if(pipe(fd_pipe) == -1){
				perror("Pipe");
				exit(EXIT_FAILURE);
			}
			int pidd = fork();
			if(pidd == -1){
				perror("Fork");
				exit(EXIT_FAILURE);
			}
			if(pidd == 0){
				dup2(fd_pipe[1],STDOUT_FILENO);
				//close pipe for reading
				close(fd_pipe[0]);
				execvp(cmd1[0],cmd1);
				exit(EXIT_SUCCESS);
			}
			else{
				dup2(fd_pipe[0],STDIN_FILENO);
				// close pipe for writting
				close(fd_pipe[1]);
				execvp(cmd2[0],cmd2);
			}
			
                }
		usleep(10000);
		continue;
		}
		

		command = get_input(input);	// Tokenize the input cmd....
		child_pid = fork();
		if (child_pid < 0){        
		    	perror("Fork failed");
	    		exit(1);
		}        
		if (child_pid == 0){

	    		/* Never returns if the call is successful */            
			if (execvp(command[0], command) < 0){           
				other_command(command);
				printf("other command \n");
				exit(1);
	    		}
		} 
		else{
			if (strcmp(command[0], "cd") == 0){	// CD cmd should be done by parent
				if (cd(command[1]) < 0) {
					perror(command[1]);            	
				}
			}

		    	waitpid(child_pid, &stat_loc, WUNTRACED);
		}
    	}
	return 0;
}
char **get_input(char *input) {
    	char **command = malloc(8 * sizeof(char *));
	if (command == NULL){
		perror("malloc failed");
		exit(1);
	}
    	char *separator = " ";
	char *parsed;
    	int index = 0;
	parsed = strtok(input, separator);
    	while (parsed != NULL) {
	    	command[index] = parsed;
		index++;
		parsed = strtok(NULL, separator);
    	}
	command[index] = NULL;
    	return command;
}
int other_command(char **command){
		if (strcmp(command[0], "cd") == 0){       
		    	if (cd(command[1]) < 0) {
				perror(command[1]);            
			}
		}
		if (strcmp(command[0], "exit") == 0){   
		       printf(" exitting terminal ...... \n");
		       kill(getppid(),9);
		  				//KILL my father and  Me!
		} 
}
int cd(char *path){
	printf("changing path to %s\n",path);
	return chdir(path);
}
void init_shell()
{
    clear();
    printf("\n\n\n\n\n\n\n\t\t WELCOME 2 KTK & SIFAT SHELL.............");
    char* username = getenv("USER");
    printf("\n\n\nUSER is: @%s", username);
	sleep(2);
    printf("\nLOADING .................\n");
    sleep(1);
    clear();
    exit(0);
}
// Function to take input 
int takeinput(char* str, char* printit){
    char *buf;
    buf = readline(printit);
    if (strlen(buf) != 0){
    	    add_history(buf);    // add the cmd to history
	    strcpy(str,buf);
	    return 0;
    } 
    else{       
	    return 1;
    }
}


