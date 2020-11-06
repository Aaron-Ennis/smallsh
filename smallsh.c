/* 
 * Filename: smallsh.c
 * Author: Aaron Ennis
 * Email: ennisa@oregonstate.edu
 * Last modified: 5 November 2020
 * Description: This is the main implementation file for a Linux shell program
 * that implements a small subset of features akin to a more full-featured
 * shell like bash or csh. The purpose of this exercise is to demonstrate some
 * aspects of the UNIX/Linux process API, signals and signal handling and I/O
 * redirection. 
 */

#include "linkedList.h"
#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_INPUT 2048

int variableExpand(char* target, int targetMax, char* source, char token, char* replStr);
void cleanUpBeforeExit(struct LinkedList* commands);
void handle_SIGTSTP(int sigNum);

int main(int argc, char const *argv[])
{
  pid_t shellPid = getpid();
  pid_t bgPid;
  char* shellPidStr = calloc(10, sizeof(char));
  char userInput[MAX_INPUT];
  char expandedInput[MAX_INPUT];
  struct Command* myCommand = NULL;
  struct Command* bgCommand = NULL;
  // We'll need this to ignore signals under certain circumstances
  struct sigaction ignore = {{0}};
  // We'll use this to 
  struct sigaction toggleFGOnly = {{0}};

  // Use a linked list to keep track of processes running in the background,
  // and initialize an iterator for it.
  struct LinkedList* bgCommands = linkedListCreate();

  int fgOnly = 0;   // Keep track of foreground only mode
  int lastFgStatus = 0; // Keep track of the status of the last fg command

  // Convert smallsh pid to string for use in variable expansion
  sprintf(shellPidStr, "%d", shellPid);

  // Register a signal handler to ignore SIGINT/ctrl-c by default
  // We will set custom behavior for this signal for foreground commands
  ignore.sa_handler = SIG_IGN;
  sigfillset(&ignore.sa_mask);
  ignore.sa_flags = 0;
  sigaction(SIGINT, &ignore, NULL);

  // Register a custom signal handler for SIGTSTP to enter/exit foreground
  // only command mode.
  toggleFGOnly.sa_handler = handle_SIGTSTP;
  sigfillset(&toggleFGOnly.sa_mask);
  toggleFGOnly.sa_flags = SA_RESTART;
  sigaction(SIGTSTP, &toggleFGOnly, NULL);
  

  while(1) {
    
    printf(": ");   // Display the command prompt
    fflush(stdout);
    fgets(userInput, MAX_INPUT, stdin);       // Get user input
    userInput[strlen(userInput) - 1] = '\0';  // Remove the newline character
      
    // Keep processing the commands as long as a comment or a blank line is
    // entered. Otherwise, just loop back and display the prompt.
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
      // Handle built-in "status" command
        if (lastFgStatus > 1) {
          printf("terminated by signal %d\n", lastFgStatus);
          fflush(stdout);
        } else {
          printf("exit value %d\n", lastFgStatus);
          fflush(stdout);
        }
      } else {
        if (myCommand->runScope == 1 && fgOnly == 0) {
        // Keep track of the command since it's going to run in the background
          linkedListAddFront(bgCommands, myCommand);
          executeCommand(myCommand, fgOnly);
        } else {
        // Otherwise, run it and destroy it immediately as a foreground process
        lastFgStatus = executeCommand(myCommand, fgOnly);
        destroyCommand(myCommand);
        }
      }
    }
    // Iterate through the list of background commands.
    struct Iterator* iterator = createIterator(bgCommands);
    while(iteratorHasNext(iterator)) {
      bgCommand = iteratorNext(iterator);
      bgPid = waitpid(bgCommand->myPid, &bgCommand->exitStatus, WNOHANG);
      // If the return value for waitpid() is 0, the pid has not terminated.
      // Otherwise, the pid itself will return if it has terminated...
      if (bgPid != 0) {
        if (WIFEXITED(bgCommand->exitStatus)) {
            printf("background pid %d is done: exit value %d\n", bgCommand->myPid, bgCommand->exitStatus);
            fflush(stdout);
            // Free this command and remove it from the list
            destroyCommand(bgCommand);
            iteratorRemove(iterator);    
          } else {
            printf("background pid %d is done: terminated by signal %d\n", bgCommand->myPid, bgCommand->exitStatus);
            fflush(stdout);
            // Free this command and remove it from the list
            destroyCommand(bgCommand);
            iteratorRemove(iterator);
          }
      }
    }
    iteratorDestroy(iterator);  // Remove the iterator for next loop
    fflush(stdin);
  }

  cleanUpBeforeExit(bgCommands);

  free (shellPidStr);
  return 0;
}

void handle_SIGTSTP(int sigNum) {
  char* message = "\nEntering foreground-only mode (& is now ignored)\n";
  write(STDOUT_FILENO, message, 50);
  fflush(stdout);
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




/**
 *  This function takes a LinkedList struct as a parameter and performs
 *  all of the necessary memory cleanup. It should be called just before the
 *  program exits.
 */
void cleanUpBeforeExit(struct LinkedList* commands) 
{
  struct Iterator* iterator = createIterator(commands);
  struct Command* currentCommand = NULL;

  // Destroy/free all of the command structs before we destroy the LinkedList
  while (iteratorHasNext(iterator)) {
    currentCommand = iteratorNext(iterator);
    destroyCommand(currentCommand);
  }

  // Destroy/free the iterator we created for this function
  iteratorDestroy(iterator);
  linkedListDestroy(commands);
}
