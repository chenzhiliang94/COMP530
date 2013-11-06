/* 
 * A server program that provides the service of converting a string
 * to upper case.  It is implemented using the "daemon-service" 
 * model.  In this model, multiple clients can be serviced
 * concurrently by the service.  The server main process is 
 * a simple loop that accepts incoming socket connections, and for 
 * each new connection established, uses fork() to create a child 
 * process that is a new instance of the service process.  This child 
 * process will provide the service for the single client program that 
 * established the socket connection.  
 *
 * Each new instance of the server process accepts input strings from 
 * its client program, converts the characters to upper case and returns 
 * the converted string back to the client.
 *
 * Since the main process (the daemon) is intended to be continuously
 * available, it has no defined termination condition and must be
 * terminated by an external action (Ctrl-C or kill command).
 *
 * The server has one parameter: the port number that will be used
 * for the "welcoming" socket where the client makes a connection.
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

#define MAX_TMP 100

/* variables to hold socket descriptors */
  ServerSocket welcome_socket;
  Socket connect_socket;

  /* variable names for file "handles" */
  FILE *tmpFP;
  FILE *fp;

  unsigned char tmp_name[MAX_TMP];

void runCommand(void);

int main(int argc, char* argv[])
{
  pid_t spid, term_pid; /* pid_t is typedef for Linux process ID */ 
  int chld_status;
  bool forever = true;
  int i;
  int rc;
  
  unsigned char id_str[MAX_TMP];
  int id;

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

     id = (int) getpid();
  sprintf(id_str, "%d", id);
  strcpy(tmp_name,"tmp");
  strcat(tmp_name, id_str); 

  fp = freopen(tmp_name, "w", stdout); 


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

     runCommand();
     Socket_close(connect_socket);
     exit (0);
}

void runCommand(void) {
  int i;
  int childPID;
  int childSTAT;
  char c, rc;
  char input[MAX_LINE];
  char * args[MAX_ARGS];

  while(1){
    for (i = 0; i < MAX_LINE; i++) {
      c = Socket_getc(connect_socket);
      if (c == EOF) {
        printf("Socket_getc EOF or error\n"); 
        return; /* assume socket EOF ends service for this client */           
      }
      else {
        if (c == '\n') {
          input[i] = '\0';
          break;
        }
        input[i] = c;
      }
    }

       /* be sure the string is terminated if max size reached */
      if (i == MAX_LINE) input[i-1] = '\0';
      
      childPID = fork();

      if (childPID == 0){
        //code for child: 

        char * token = strtok(input, " "); // tokenize the input line
        args[0] = token; // set first "argument" to the command itself
      
        int count = 1;
        // loop to fill args[] with the remaining arguments (if any)
        while (token != NULL) {
          token = strtok(NULL, " ");
          args[count] = token;
          count++;
          if (count > (MAX_ARGS+2)){
            // handle it
          }
        }
        
        args[count - 1] = NULL; // the array must be null terminated for execvp() to work
        int execCode = execvp(args[0], args);
        if (execCode == -1) {
          printf("Error: invalid command.\n");
        }
        exit(0);
      }

      else if (childPID == -1) {
        printf("Problem forking.\n");
      }
      
      else {
        //parent code:
        waitpid(0, &childSTAT, WNOHANG|WUNTRACED);
        if (WIFEXITED(childSTAT) == true) {
          WEXITSTATUS(childSTAT);
        }
        printf("sup\n");
        char file_c;
        tmpFP = fopen(tmp_name, "r");

        // while (read(tmpFP, &file_c, sizeof(char)) != 0) {
        //   rc = Socket_putc(file_c, connect_socket);
        // }
        // close(tmpFP);
      }

    
  }
  
}