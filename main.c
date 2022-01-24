#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include "my_shell.h"
#define BUFFER_SIZE 100         // the max size of the input containing the user's commands
#define COMMAND_WORD_SIZE 10    // the max number of words on the command line
#define DELIMETERS " \t"

// static variable to hold the user's name
static char *username;
static int counter = 1;

/* variable to keep track of the number of
   words in the command line when running  */
static int num_words = 0;

// static char arrays
static char line[BUFFER_SIZE]; // 100
static char *command_words[COMMAND_WORD_SIZE]; // 10

// function to print an error if command cannot be found
void command_not_found(char *command){
	fprintf(stderr, " [*] Command not found: %s\n", command);
	exit(1);
}

// function to read the number of words from argv -> example "hi gusty" = 2
int get_command_words() {
	// Collect words on line into cmd_words
	num_words = 0;
	char *p;
	// strtok() returns pointer to word on line
	p = strtok(line, DELIMETERS); 
	while (p != NULL) {                         
		// [!] p has address in line, e.g., &line[0]
		// cmd_words[] points to words on the "line" array
		command_words[num_words] = p;
		// count the words on the line
		num_words++;
		// get the nexr word on the line
		p = strtok(NULL, DELIMETERS);
	}
	// 0 marks end of words in cmd_words
	command_words[num_words] = NULL;
	// return num of words found
	return num_words;                           
}

// ----------------------------------------------------------------------------------
int main(int argc, char **argv){
	printf("***************************************************************\n");
	printf("*                                                             *\n");
	printf("*                         MY SHELL                            *\n");
	printf("*                                                             *\n");
	printf("***************************************************************\n");

	// get user's name from their terminal
	username = getenv("USER");
	printf("\nUSER is: @%s\n", username);

	// show time and date
	time_t current_time = time(NULL);
	struct tm time = *localtime(&current_time);
	printf("DATE/TIME: %d-%02d-%02d - %02d:%02d:%02d\n", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
	printf("FOR SHELL COMMANDS, TYPE -> HELP <- TO SEE THE LIST OF COMMANDS\n");
	printf("\n");
	printf("***************************************************************\n");
	printf("***************************************************************\n\n");

	// backgroundrun set to 1 after % ./loop &
	int num = 1;
	// set a boolean to false to indicicate piping
	bool piping = false;
	bool redirect = false;

	// [*] MAKE SURE TO KEEP THIS HERE!
	// used to catch CLT-C when the program runs
	signal(SIGINT, signal_handler);
	while(1) {
		// set the line to zero before each use!
		memset(line, 0, BUFFER_SIZE);   
		// print the user's name and update the shell number
		fprintf(stdout, "%s@DESKTOP - %d %% ", username, counter);
		//update counter
		counter++;
		// flush the prompt to the screen for the user
		fflush(stdout);  

		// if the input is CTL-D, then exit the shell
		if (fgets(line, BUFFER_SIZE, stdin) == 0){   
			exit_command();
		}

		// trim lf from line
		line[strcspn(line, "\n")] = '\0';

		// is the user does not enter input, then continue with the loop
		if(!get_command_words()){
			continue;
		}	       

		//testing(command_words, num_words);
		// exit the program if the user types "exit"
		if(strcmp(command_words[0], "exit") == 0){
			exit_command();
		}


		if(fork() == 0) {
			//to check for pipe
			int index = 0;
			for(int i = 0; i < num_words; i++){
				const char symbols[] = "|";
				char *check_for_pipe;
				// here is where we check
				check_for_pipe = strpbrk(symbols, command_words[i]);

				// if the string contains a |, then mark as true
				if(check_for_pipe){
					piping = true;
					index = i;
				}//end of if
			}//end of for
			int a = check(command_words, num_words, ">");
			int b = check(command_words, num_words, "<");
			if(a > 0 || b > 0){
				redirect = true;
			}

			// catch CLT-C
			signal(SIGINT, proc_signal_handler);
			// print the shell commands to the screen
			//print(command_words, num_words);	
			if(strcmp(command_words[0], "help") == 0){
				help_command();
				break;
			}
			else if(redirect == true && piping == false){
				redirection(command_words, num_words, a, b);
				break;
			}
			else if(piping == true){
				piped(command_words, num_words, index, redirect, a , b);
				break;
			}
			else if(strcmp(command_words[0], "ls") == 0){
				ls_command(command_words, num_words);
				break;
			}
			else if(strcmp(command_words[0], "pwd") == 0){
				pwd_command();
				break;
			}
			else if(strcmp(command_words[0], "cd") == 0){
				cd_command(command_words, num_words);
				break;
			}
			else if(strcmp(command_words[0], "cat") == 0){
				cat_command(command_words[1], argc);
				break;
			}
			else if(strcmp(command_words[0], "Gusty") == 0 || strcmp(command_words[0], "gusty") == 0){
				heart();
				break;
			}
			else{
				printf(" [*] Invalid command!\n");
				break;
			}
		}

		// call wait
		wait(NULL);
		// command number, blank lines do not increment num
		num++;  
	}
}
