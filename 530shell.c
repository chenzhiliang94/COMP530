/*
 ============================================================================
 Name        : 530shell.c
 Author      : John Haskell
 Description : A simple shell. The user is given a prompt and can type in 
		commands to be executed. To do this, the shell forks itself
		and the child process parses the command and uses execvp()
		appropriately.
 ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define MAX 200 // max number of characters in input line
#define MAX_ARGS 10 // max number of arguments in a command 

int main() {
	int childPID;
	char input[MAX];
	int length = 0; // length of the input string; used to check input is not too long
	
	printf("%% "); // print initial prompt

	// begin main loop
	while ((fgets(input, 9999, stdin)) != NULL) {
		
		length = strlen(input); 

		// check to see if the input string is too long
		// if it is, jump to the end of the loop
		if (length > MAX) {
			printf("This shell can only process commands fewer than 200 characters.\nPlease enter a shorter command.\n");
			printf("%% ");
			goto end;
			}

		//get rid of the newline character at the end of the input string:		
		input[length - 1] = ' ';

		char * args[MAX_ARGS]; // array to store arguments that will be passed to execvp()

		childPID = fork(); // fork the shell, creating a child process

		if (childPID == 0) {
			//code for child follows
			int execCode = 0; // used to find execvp errors
			char * token = strtok(input, " "); // tokenize the input line
			args[0] = token; // set first "argument" to the command itself
			int count = 1;

			// loop to fill args[] with the remaining arguments (if any)
			while (token != NULL) {
				token = strtok(NULL, " ");
				args[count] = token;
				count++;
				if (count > (MAX_ARGS+2)){
					printf("This shell can only process commands with 10 or fewer arguments.\nPlease use less arguments.\n");
					printf("%% ");
					goto end;
					}
			}

			args[count - 1] = NULL; // the array must be null terminated for execvp() to work

			// check to make sure execvp() worked correctly
			execCode = execvp(args[0], args);
			if (execCode == -1) {
				printf("Error: invalid command.\n");
				}
			printf("%% ");
		}

		// check to make sure fork() ran correctly
		else if (childPID == -1) {
			printf("There was a problem forking the child process.\n");
			printf("%% ");
			}

		else {
			//code for parent
			printf("%% ");
			wait();
		}
		end:; // label for previous goto statements
	}
	return 0;
}


