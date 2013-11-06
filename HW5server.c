/* 
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
#define ERROR1 "Invalid command\n\0"
#define ERROR2 "Socket_getc EOF or error\n\0"

/* variables to hold socket descriptors */
ServerSocket welcome_socket;
Socket connect_socket;


int main(int argc, char* argv[])
{
  bool doneFlag = false;
  int i;
  int rc;
  int c;
  unsigned char tmp_name[MAX_TMP];
  unsigned char id_str[MAX_TMP];
  int id;
  int childPID, term_pid;
  int chld_status;
  char fc;
  char file_line[MAX_LINE];
  char * args[MAX_ARGS];


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

     Socket_close(welcome_socket);
     char input[MAX_LINE]={'\0'};
     memset(input,'\0',sizeof(char)*MAX_LINE);
     while(1){
      for (i = 0; i < MAX_LINE; i++) {
        c = Socket_getc(connect_socket);
        if (c == EOF) {
          remove(tmp_name);
          doneFlag = true;
          for (i=0; i<=strlen(ERROR2); i++){
            Socket_putc(ERROR2[i], connect_socket);
          }
         // printf("Socket_getc EOF or error\n"); 
          return; /* assume socket EOF ends service for this client */           
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


     fp = freopen(tmp_name, "w", stdout);
     childPID = fork();

     if (childPID == 0){
        //code for child: 

      char * token = strtok(input, " \t");

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
        
        args[count] = NULL; // the array must be null terminated for execvp() to work
        int execCode = execvp(args[0], args);
        if (execCode == -1) {
          for (i=0; i<=strlen(ERROR1); i++){
            Socket_putc(ERROR1[i], connect_socket);
          }
         // exit(0);
          //goto end;
         // printf("Error: invalid command.\n");
        }
      }

      else if (childPID == -1) {
        printf("Problem forking.\n");
      }
      
      else {
        //parent code:
        term_pid = waitpid(childPID, &chld_status, 0);
        if (doneFlag == false){
         char file_c;

         if ((tmpFP = fopen (tmp_name, "r")) == NULL) {
          fprintf (stderr, "error opening tmp file\n");
          exit (-1);
        }
        while ((fc = fgetc(tmpFP)) != EOF) {
          Socket_putc(fc, connect_socket);
        }
      }

      if (term_pid == -1) 
        perror("waitpid"); 

      else
      {
        if (WIFEXITED(chld_status)) {
          char str[] = "PID %d exited, status = %d\n";
          char str2[256];
          sprintf(str2, str, childPID, WEXITSTATUS(chld_status));
          for (i=0; i<=strlen(str2); i++){
            Socket_putc(str2[i], connect_socket);
          }
           //printf("PID %d exited, status = %d\n", childPID, WEXITSTATUS(chld_status));
        }
        else{
         printf("PID %d did not exit normally\n", childPID);
        }
     }
     


   }

   end:;
 }

 Socket_close(connect_socket);
 exit (0);
}