#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>

#include "my_shell.h"

// TODO: ls_command() 
void ls_command(char **command, int size){
	printf("\n");
	char command2[10];
	snprintf(command2, sizeof command2, "%s %s", command[0], command[1]);
	if(command[1] == NULL){
		system("ls");
	}
	else{
		if(command[1] != NULL){
			system(command2);
		}
		else{
			printf(" [*] Error: invalid command.\n");
		}
	}
	printf("\n");
}

// TODO: cd function
void cd_command(char **command, int size){
	char directory[100];
	char first[200];
	getcwd(first, sizeof(first));
	char *third = "/";
	char second[200];
	snprintf(second, sizeof second, "%s%s%s", first, third, command[1]);
	//Changing the directory to the previous directory before it.
	if(strcmp(command[1], "..") == 0){	
		if(chdir("..") < 0){
			printf("Cant change\n");
		}
	}//end of if
	else{
		if(chdir(second) < 0){
			printf(" [*] Invalid directory.\n");
			exit(1);
		}
	}
	//Print the current working directory.
	printf("%s\n",getcwd(directory,100));
}
// Function to show the current working directory. 
void pwd_command(){
	char current_directory[1024];
	chdir("/current_directory");
	// Find the current working directory of the user.
	getcwd(current_directory, sizeof(current_directory));
	printf("Current location: %s\n", current_directory);
}
// TODO: cat function
void cat_command(char *filename, int argc){

	int ch, command;

	// [*] using a for-loop helps to add a break
	for(int i = 0; i < argc; i++){

		// open the file to read - USE O_RDONLY!
		command = open(filename, O_RDONLY);

		// check to make sure user entered in a file
		if(command < 0){
			// this will print the specific type of error and leave the function
			perror(" [*] Error: ");
			break;

		}
		// read each character from the file - one by one
		while(read (command, &ch, 1)){  
			write(STDOUT_FILENO, &ch, 1); 
		}

		close(command);
	}
}
// TODO: function to catch CLT-C
void signal_handler(int signo){
	if (signo == SIGINT) {
		// print warning
		fprintf(stdout, "\n [*] signal_handler caught CTL-C \n");
		fprintf(stdout, " [-->] To exit program, type -> exit <- or hit CLT-D\n");
		fprintf(stdout, "       To get back to the shell, press enter. [<--]\n");
		// flush statement
		fflush(stdout);
	}
}
// TODO: function to catch CLT-C
void proc_signal_handler(int signo){
	if (signo == SIGINT) {
		// print warning
		fprintf(stdout, "\n [*] proc_signal_handler caught CTL-C \n");
		// flush statement
		fflush(stdout);
		signal(SIGINT, proc_signal_handler);
	}
}

// TODO: help command
void help_command(){
	printf("\n");
	printf("***************************************************************\n");
	printf("*   							      *\n");
	printf("*   Shell Commands to Try:                                    *\n");
	printf("*   							      *\n");
	printf("*   	1. ls                                                 *\n"); 
	printf("*   	    - Examples:                                       *\n"); 
	printf("*   	        ls -a     ls -s                               *\n"); 
	printf("*   	        ls -r     ls -t                               *\n"); 
	printf("*   	        ls -l     ls -ltr                             *\n"); 
	printf("*   	        ls ..     ls ~                                *\n"); 
	printf("*   	2. pwd                                                *\n");
	printf("*   	3. cd                                                 *\n");
	printf("*   	4. cat                                                *\n");
	printf("*   	5. exit                                               *\n");
	printf("*   							      *\n");
	printf("***************************************************************\n");
	printf("\n");
}
// TODO: exit function
void exit_command(){
	printf("\n");
	printf("***************************************************************\n");
	printf("*                                                             *\n");
	printf("*                THANKS FOR USING THE SHELL!                  *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");
	exit(0);
}

// Implement the ablity to pipe one command to another -> example: ls | grep toy | grep cup
// Implement the ability to run a command in the background
// Implement signal handlers for Clt-D and Clt-C
void piped(char **command, int size, int index, bool good, int a, int b){
	int fd[2]; //initilize the file descriptor
	int status = pipe(fd);//put descriptor in the pipe
	if (status == -1){//if status is -1
		fprintf(stderr, "Pipe Failed");//pipe has failed
	}//end of if

	pid_t process1 = fork();//get process1 to write stuff to the first end of the pipe
	if (process1 < 0){//if the process is less than 0
		fprintf(stderr, "fork failed");//fork has failed
	}//end of if

	if(process1 == 0){//makes the child process
		//child process
		dup2(fd[1], 1);//make a copy of a file descriptor
		close(fd[0]);//close the reading end
		close(fd[1]);//close the writing end since we already duplicated it
		if(index == 1){//if the location of "|" at one.
			execlp(command[0], command[0], NULL);//execute these commands
		}//end of if
		else if(index == 2){//if the location of "|" at two
			execlp(command[0], command[0], command[1],  NULL);//execute these commands
		}//end of else if
	}//end of if
	pid_t process2 = fork(); //second fork for the second command half of the command that I am going to parse in

	if(process2 < 0){//if the process is less than 0
		fprintf(stderr, "fork failed");//fork is going to failed
	}//end of if
	if(process2 == 0){//if it is zero than it is child
		//child process
		if(good == false){//if there is a ">" or "<" symbol in the command array then dont do this
			dup2(fd[0], 0);//make duplicate of reading
			close(fd[0]);//close the main descriptor since we have a duplicate one
			close(fd[1]);//close the other end as well
			if(index == 1 && size == 3){//if the "|" at 1 and size 3
				execlp(command[2], command[2], NULL);//execute these commands by reading the read end of the pipe.
			}//end of if
			else if(index == 1 && size == 4){//if the "|" at 1 and size 4
				execlp(command[2], command[2], command[3], NULL);//execute these commands by reading the read end of the pipe.
			}//end of else if
			else if(index == 2 && size == 4){//if the "|" at 2 and size is 4
				execlp(command[3], command[3], NULL);//execute these commands by reading the read end of the pipe.
			}//end of else if
			else if(index == 2 && size == 5){//if the "|" at 2 and size is 5
				execlp(command[3], command[3], command[4], NULL);//execute these commands by reading the read end of the pipe.
			}//end of else if
		}//end of if
		else{//begin else statement if there is no "<" or ">" sign
			if(a > 0 && b > 0){//if both "<" and ">" show up do this
				printf("This command is not possible with this shell\n");//our shell is not designed to handled these commands
				exit(1);//exit(1)
			}//end of if
			else if(a > 0){//if ">" exist then do this
				int p = open(command[size - 1], O_CREAT | O_WRONLY | O_APPEND, 0644);//make a file to put data in
				dup2(fd[0], 0);//duplicate the descriptor to handle the read part
				close(fd[0]);//close the main descriptor reading end 
				close(fd[1]);//close the main descriptor writing end
				dup2(p, 1);//make another duplication of write to write in a file
				if(index == 1 && size == 5){//if the "|" at 1 and size is 5
					execlp(command[2], command[2], NULL);//execute these commands
				}//end of if
				else if(index == 1 && size == 6){//if the "|" at 1 and size is 6
					execlp(command[2], command[2], command[3], NULL);//then execute these commands
				}//end of else if
				else if(index == 2 && size == 6){//if the "|" at 2 and size is 6
					execlp(command[3], command[3], NULL);//execute the commands
				}//end of else if
				else if(index == 2 && size == 7){//if the "|" at 2 and size is 7
					execlp(command[3], command[3], command[4], NULL);//execute these commands
				}//end of elseif
				close(p);//close the descriptor to write to a file since by now that should be done

			}//end of else if
			else if(b > 0){//if "<" exist then do this
				int p = open(command[size - 1], O_RDONLY, 0644);//make a file to read data from
				if(p < 0){//if p is less than zero
					fprintf(stderr, "Redirect output failed\n");//file failed
					exit(1);//exit the program
				}//end of if	
				dup2(fd[0], 0);//make a duplicate file descriptor with read end
				close(fd[1]);//close the main one we dont need it
				close(fd[0]);//close this one too
				dup2(p, 0);//duplicate another one with read properties
				if(index == 1 && size == 5){//if the "|" is at 1 and size is 5
					execlp(command[2], command[2], NULL);//execute these commands by reading the read end of the pipe.
				}//end of if
				else if(index == 1 && size == 6){//if the "|" is at 1 and size is 6
					execlp(command[2], command[2], command[3], NULL);//execute these commands by reading the read end of the pipe.
				}//end of else if
				else if(index == 2 && size == 6){//if the "|" is at 2 and size is 6
					execlp(command[3], command[3], NULL);//execute these commands by reading the read end of the pipe.
				}//end of else if
				else if(index == 2 && size == 7){//if the "|" is at 2 and size is 7
					execlp(command[3], command[3], command[4], NULL);//execute these commands by reading the read end of the pipe.
				}//end of else if
				close(p);//close the descriptor to read since by now the file content should be read

			}//end of else if


		}//end of else
	}//end of of
	//close the file descriptors in the main process
	close(fd[0]);//close the read descriptor of the main process
	close(fd[1]);//close the write descriptor of the main process
	wait(NULL);//wait for the first child
	wait(NULL);//wait for the second child

}//end of piped

//This function is used just print the lines we input in shell
//Those lines are stored in an array
void print(char **command, int size){
	for(int i = 0; i < size; i++){
		printf("%s\n", command[i]);
	}//end of for
}//end of print

//This function is used to check if any symbols exist
//If exist it returns the index of that symbol
int check(char **command, int size, char *symbol){
	int index= 0;
	for(int i = 0; i < size; i++){
		char *check_for_data;
		// here is where we check
		check_for_data = strpbrk(symbol, command[i]);

		//if the string contains a |, then mark as true
		if(check_for_data){
			index = i;
		}//end of if
	}//end of for
	return index;
}//end of check

//This function is used to do redirections of input and output
void redirection(char **command, int size, int indexa, int indexb){
	if(indexa > 0){//if the ">" exist do this
		int fd = open(command[size - 1], O_CREAT | O_WRONLY | O_APPEND, 0644);//create a file to write into
		if(fd < 0){//if the fd is less than 0
			fprintf(stderr, "redirect output failed\n");//file failed to be made
			exit(1);//exit
		}//end of if
		int test = dup2(fd, 1);//make dup 2 with wrting features
		if(test < 0){//if the dup is less than 0
			fprintf(stderr, "Dup Failed\n");//print dup failed
			exit(1);//exit the program
		}//end of if
		if(indexa == 2){//if the index of ">" at 2
			execlp(command[0], command[0], command[1], NULL);//execute the first commands
		}//end of if
		else if(indexa == 1){//if the index of ">" at 1
			execlp(command[0], command[0], NULL);//execute these commands
		}//end of else if
		close(fd);//close the descriptor
	}//end of if
	if(indexb > 0){//if the "<" exist do this
		int fd = open(command[size - 1], O_RDONLY, 0644);//File to read from
		if(fd < 0){//if the fd is less than 0
			fprintf(stderr, "redirect output failed\n");//The redirection failed
			exit(1);//exit right now
		}//end of if
		int test = dup2(fd, 0);//make dup2 with read properties
		if(test < 0){//if the dup is less than 0
			fprintf(stderr, "Dup failed\n");//print dup failed
			exit(1);//exit
		}//end of if
		if(indexb == 2){//if the index of "<" at 2
			execlp(command[0], command[0], command[1], NULL);//execute in commands in this pattern
		}//end of if
		else if(indexb == 1){//if the idex of "<" at 1
			execlp(command[0], command[0], NULL);//execute in this manner
		}//end of else if
		close(fd);//close the fd since we are now done

	}//end of if

}//end of redirection

void heart(){
	printf("\n");
	printf("    *****     *****\n");
	printf("   *******   *******\n");
	printf("  ********* *********\n");
	printf("  *******************\n");
	printf("   *****************\n");
	printf("    ***************\n");
	printf("     *************\n");
	printf("      ***********\n");
	printf("       *********\n");
	printf("        *******\n");
	printf("         *****\n");
	printf("          ***\n");
	printf("           *\n");
	printf("\n");
}
