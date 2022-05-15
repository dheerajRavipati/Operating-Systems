// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports four arguments

int main()
{

  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;                                         
                                                           
    char *working_string  = strdup( command_string );                

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    // Now print the tokenized input as a debug check
    //Condition to check if any input is given.
    //If a blank line is entered, the shell will clear the
    if (!(token[0] == NULL))
    {
      // If provided input is exit or quit, program exits
      if ((strcmp(token[0], "exit") == 0) || (strcmp(token[0], "quit") == 0))
      {
        free(head_ptr);
        return 0;
      }
      //Condition to accept "cd" command.
      //If cd command is entered, it will call chdir function
      //which can be used to change directory
      else if (strcmp(token[0], "cd") == 0)
      {
        int status = chdir(token[1]);
        if (status == -1)
        {
          printf("No Such file or directory found %s. \n", token[1]);
          return 0;
        }
        else
        {
          return 0;
        }
      }
      else if (token[0][0] == '!')
      {
        int i;
        char n[MAX_COMMAND_SIZE];
      }
      else if (strcmp(token[0], "pidhistory") == 0)
      {
        printf("pidhistory");
      }
      //If some shell command, other than above derived commands, is entered by the user
      else
      {
        int pid = fork();
        if (pid == 0)
        {
          if (execvp(token[0], token) == -1)
          {
            printf("%s: Command not found.\n", token[0]);
            exit(0);
          }
        }
        else
        {
          int status;
          wait(&status);
        }
      }
    }
    free(head_ptr);
  }

  return 0;
  // e2520ca2-76f3-11ec-90d6-0242ac120003
}
