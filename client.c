/* John Haskell
This program is a client that takes in user input on stdin and sends it
to a server via a socket. The server handles processing this input as a 
shell command. The result of the shell command's execution is received
by this client along with a reply line from the server.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "Socket.h"
#include "ToUpper.h" /* definitions common to client and server */

 int main(int argc, char* argv[])
 {
  int i, c, rc, fc; // for indexing and holding characters
  int count = 0; // used for iterating over what was typed on stdin

  char line_data[MAX_LINE]; // a buffer for holding characters from stdin

  /* variable to hold socket descriptor */
  Socket connect_socket;

  if (argc < 3)
  {
    printf("No host and port\n");
    return (-1);
  }

  /* connect to the server at the specified host and port;
   * blocks the process until the connection is accepted
   * by the server; creates a new data transfer socket.
   */
   connect_socket = Socket_new(argv[1], atoi(argv[2]));
   if (connect_socket < 0)
   {
     printf("Failed to connect to server\n");
     return (-1);
   }

  printf("%% "); // print initial prompt

  /* get a string from stdin up to and including 
   * a newline or to the maximim input line size.  
   * Continue getting strings from stdin until EOF.
   */ 



   while ((fgets(line_data, sizeof(line_data), stdin) != NULL))
   {
      count = strlen(line_data) + 1; /* count includes '\0' */

      /* send the characters of the input line to the server
       * using the data transfer socket.
       */
       for (i = 0; i < count; i++)
       {
         c = line_data[i];
         rc = Socket_putc(c, connect_socket);
         if (rc == EOF)
         {
           printf("Socket_putc EOF or error\n");             
           Socket_close(connect_socket);
              exit (-1);  /* assume socket problem is fatal, end client */
         }
       }


      /* receive the converted characters for the string from
       * the server using the data transfer socket.
       */
       while (1){
        fc = Socket_getc(connect_socket);
        printf("%c", fc);
        if (fc == '\0') {
          break;
        }
      }

       printf("%% "); // print prompt for next command

     } /* end of while loop; at EOF */

       Socket_close(connect_socket);
       exit(0);
     }
