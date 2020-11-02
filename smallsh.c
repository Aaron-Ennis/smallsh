/* 
 * Filename: smallsh.c
 * Author: Aaron Ennis
 * Email: ennisa@oregonstate.edu
 * Last modified: 28 October 2020
 * Description: This is the main implementation file for a Linux shell program
 * that implements a small subset of features akin to a more full-featured
 * shell like bash or csh. The purpose of this exercise is to demonstrate some
 * aspects of the UNIX/Linux process API, signals and signal handling and I/O
 * redirection. 
 */

#include "command.h"
#include "linkedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_INPUT 2048


int main(int argc, char const *argv[])
{
  char* shellPid = malloc(sizeof(pid_t));
  char userInput[MAX_INPUT];
  struct Command* myCommand = NULL;
  // Convert smallsh pid to string for use in variable expansion
  sprintf(shellPid, "%d", getpid());
  // Display a command prompt until the "exit" command is detected
  printf("Shell is running under pid %s", shellPid);
  do {
    printf(": ");
    fgets(userInput, MAX_INPUT, stdin);
    // Remove the newline character
    userInput[strlen(userInput) - 1] = '\0';
    // Ignore blank commands and comments
    if (userInput[0] != '\0' && userInput[0] != '#') {
      myCommand = createCommand(userInput);
    }
  } while (myCommand == NULL || strcmp(myCommand->name, "exit") != 0);

  
  destroyCommand(myCommand);
  free (shellPid);
  return 0;
}
