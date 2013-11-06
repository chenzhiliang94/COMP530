/* John Haskell
This program is a remote shell. It accepts a single connection from a client
and reads characters sent by the client over a socket. This program will
then fork-- The child parses the characters it received into tokens that
are then passed to execvp() in order to run the associated command. The resulting
output is redirected into a temporary file named using the PID of this process.
The parent waits until the child is done, then reads through the temp file, sending
its contents back to the client through the socket. Then the parent sends a reply
line indicating the exit status of the child to the client.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Socket.h"
#include "ToUpper.h" /* definitions shared by client and server */

#define MAX_TMP 100 // buffer size for temp file name 
//Possible errors that may need to be sent to client:
#define ERROR1 "Invalid command\n\0" 
#define ERROR2 "Problem forking\n\0"

/* variables to hold socket descriptors */
ServerSocket welcome_socket;
Socket connect_socket;


int main(int argc, char* argv[])
{
bool doneFlag = false; // used for signaling when the server has received EOF
int i; // for indexing
int rc; // char descriptor
int c; // char descriptor

// The following variables help build the temp file name:
unsigned char tmp_name[MAX_TMP];
unsigned char id_str[MAX_TMP];
int id;

int childPID, term_pid, chld_status; // for checking fork() and child exit status
char fc;
char * args[MAX_ARGS]; // array to store arguments
char input[MAX_LINE]={'\0'}; // buffer to store input from client


  /* variable names for file "handles" */
FILE *tmpFP;
FILE *fp;

if (argc < 2)
{
  printf("No port specified\n");
  return (-1);
}

/* create a "welcoming" socket at the specified port */
welcome_socket = ServerSocket_new(atoi(argv[1]));
if (welcome_socket < 0)
{
  printf("Failed new server socket\n");
  return (-1);
}

// Form the string to name the temp file:
id = (int) getpid();
sprintf(id_str, "%d", id);
strcpy(tmp_name,"tmp");
strcat(tmp_name, id_str); 



   /* accept an incoming client connection; blocks the
    * process until a connection attempt by a client.
    * creates a new data transfer socket.
    */
    connect_socket = ServerSocket_accept(welcome_socket);
    if (connect_socket < 0)
    {
     printf("Failed accept on server socket\n");
     exit (-1);
   }

   Socket_close(welcome_socket); // welcome socket no longer needed
   
   //memset(input,'\0',sizeof(char)*MAX_LINE);
   while(1){ // begin forever loop of reading from client and sending output
    // read from client:
    for (i = 0; i < MAX_LINE; i++) {
      c = Socket_getc(connect_socket);
      if (c == EOF) {
        // handle closing operations:
        remove(tmp_name);
        doneFlag = true;
        for (i=0; i<=strlen(ERROR2); i++){
          Socket_putc(ERROR2[i], connect_socket);
        }
        return; /* assume socket EOF ends service for this client */           
      }
      if (input[0] == ' ' || input[0] == '\t' || input[0] == '\n') {
        break;
      }
      else {
        if (c == '\0') {
          input[i] = '\0';
          break;
        }
        if (c == '\n') {
          i--;
        }
        else{       
         input[i] = c;
       }
     }
   }

     /* be sure the string is terminated if max size reached */
   if (i == MAX_LINE) input[i-1] = '\0';

   // Redirect stdout to the temp file:
   fp = freopen(tmp_name, "w", stdout);

   childPID = fork(); // fork to handle command execution

   if (childPID == 0){
    //code for child: 

    char * token = strtok(input, " \t");

      args[0] = token; // set first "argument" to the command itself

      int count = 1;
      // loop to fill args[] with the remaining arguments (if any)
      while (token != NULL) {
        token = strtok(NULL, " \t");
        
        args[count] = token;
        count++;
      }
      
      args[count] = NULL; // the array must be null terminated for execvp() to work
      int execCode = execvp(args[0], args);
      if (execCode == -1) {
        // If there was a problem, send appropriate error message to client:
        for (i=0; i<=strlen(ERROR1); i++){
          Socket_putc(ERROR1[i], connect_socket);
        }
      }
    }

    else if (childPID == -1) {
      // If there was a problem, send appropriate error message to client:
      for (i=0; i<=strlen(ERROR2); i++){
        Socket_putc(ERROR2[i], connect_socket);
      }
    }
    
    else {
      //parent code:
      term_pid = waitpid(childPID, &chld_status, 0); //waits for child to finish

      if (doneFlag == false){ // will only attempt to open temp file if EOF has not been received
       char file_c;

       if ((tmpFP = fopen (tmp_name, "r")) == NULL) {
        exit (-1);
      }

      //read contents of temp file and send to client:
      while ((fc = fgetc(tmpFP)) != EOF) {
        Socket_putc(fc, connect_socket);
      }
    }

    if (term_pid == -1) 
      perror("waitpid"); // in case there was a problem waiting

    else
    {
      if (WIFEXITED(chld_status)) {
        //form a string as a reply line to send to the client:
        char str[] = "PID %d exited, status = %d\n";
        char str2[256];
        sprintf(str2, str, childPID, WEXITSTATUS(chld_status));
        for (i=0; i<=strlen(str2); i++){
          Socket_putc(str2[i], connect_socket);
        }
         
      }
      else{
        //form a string as a reply line to send to the client:
        char str[] = "PID %d did not exit normally\n";
        char str2[256];
        sprintf(str2, str, childPID, WEXITSTATUS(chld_status));
        for (i=0; i<=strlen(str2); i++){
          Socket_putc(str2[i], connect_socket);
        }

     }
   }
   


 }
}
//Done:
Socket_close(connect_socket);
exit (0);
}