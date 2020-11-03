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
#include <signal.h>

#define MAX_INPUT 2048

int variableExpand(char* target, int targetMax, char* source, char token, char* replStr);

int main(int argc, char const *argv[])
{
  pid_t shellPid = getpid();
  char* shellPidStr = calloc(10, sizeof(char));
  char userInput[MAX_INPUT];
  char expandedInput[MAX_INPUT];
  struct Command* myCommand = NULL;

  // Convert smallsh pid to string for use in variable expansion
  sprintf(shellPidStr, "%d", shellPid);

  while(1) {
    printf(": ");   // Display the command prompt
    fflush(stdout);
    fgets(userInput, MAX_INPUT, stdin);       // Get user input
    userInput[strlen(userInput) - 1] = '\0';  // Remove the newline character
    // Ignore blank commands and comments
    if (userInput[0] != '\0' && userInput[0] != '#') {
      if (variableExpand(expandedInput, MAX_INPUT, userInput, '$', shellPidStr)) {
        myCommand = createCommand(expandedInput);
      }
      // Handle commands
      if (strcmp(myCommand->name, "exit") == 0) {
      // Handle built-in "exit" command
        destroyCommand(myCommand);
        break;
      } else if (strcmp(myCommand->name, "cd") == 0) {
      // Handle built-in "cd" command
        if (myCommand->numArgs == 1) {
        // "cd" is entered with no arguments
          chdir(getenv("HOME"));
        } else {
          chdir(myCommand->args[1]);
        }
        destroyCommand(myCommand); 
      } else if (strcmp(myCommand->name, "status") == 0) {
        printf("Display status\n");
      } else {
        executeCommand(myCommand);
        destroyCommand(myCommand);
      }
    }
  }
  printf("Exiting smallsh.\n");

  free (shellPidStr);
  return 0;
}

/**
 *  This function takes a pointer to a target string, an int representing the
 *  maximum size of the target string, a pointer to a source string, a char,
 *  and a pointer to a replacement string. It then copies the source into the
 *  target one character at a time. If it detects two token characters in a
 *  row, it copies the replacement string into the target string instead of
 *  the double token.
 *  If the length of the target is exceeded due to this expansion, an error
 *  message is displayed and the function returns 0. If successful, the
 *  function returns 1.
 */
int variableExpand(char* target, int targetMax, char* source, char token, char* replStr)
{
  int targetLen = 0;
  int replStrLen = strlen(replStr);

  // Loop through the entire source string
  while (*source != '\0') {
    // If we've exceeded the target size, print and error and return failure
    if (targetLen >= targetMax) {
      printf("Error: Target length exceeded; cannot expand. Command failed.");
      return 0;
    }
    if (*source != token) {
    // If the source character isn't a token, copy to target and increment
      *target = *source;
      target++;
      source++;
      targetLen++;
    } else {
    // Otherwise, the current char in source is a token.
      if (*(source + 1) == token) {
      // If the next char is also a token, copy the replacement string into
      // target and advance pointers and counters
        for (int i = 0; i < replStrLen; i++) {
          *target = *replStr;
          target++;
          replStr++;
          targetLen++;
        }
        source += 2;
        replStr -= replStrLen;
      } else {
      // The next char in source is not also a token, so copy the single
      // token into target and increment
        *target = *source;
        target++;
        source++;
        targetLen++;
      }
    }
  }
  // Need to null-term target since we are doing char-by-char copy
  *target = '\0';
  return 1;
}
