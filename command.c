/* 
 * Filename: command.c
 * Author: Aaron Ennis
 * Email: ennisa@oregonstate.edu
 * Last modified: 31 October 2020
 * Description: This is the implementation file that defines a structure
 * to contain information about a command, and some simple functions to 
 * operate on the structure.
 */

#include "command.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/** 
 * This function creates a new Command struct by parsing the command name and
 * its arguments from a string with no particular format.  
 * Parameters: A pointer to a character array
 * Return value: A pointer to a structure containing the
 */
struct Command* createCommand(char* rawData)
{
  char *token;
  char *saveptr;
	struct Command* newCommand = malloc(sizeof(struct Command));
  // Default exitStatus, runScope, and termSig for built-in commands
  newCommand->exitStatus = 0;
  newCommand->runScope = 0;
  newCommand->termSig = 0;
  // Default  file names for redirection to NULL
  newCommand->inputFile = NULL;
  newCommand->outputFile = NULL;
  newCommand->numArgs = 0;

  
  /* If the last characters in the string are '&' preceded by a space, set
   * the runScope flag to 1 to indicate this command should be run in the 
   * background and trim the '&' and trailing space from the string
   */

  if (strcmp(rawData + strlen(rawData) - 2, " &") == 0) {
    newCommand->runScope = 1;
    rawData[strlen(rawData) - 2] = '\0';
  }

  /* Get the first token, which we will assume is the command name, and copy it
   * into the Command struct's name member variable.
   */
  token = strtok_r(rawData, " ", &saveptr);
  newCommand->name = calloc(strlen(token) + 1, sizeof(char));
  strcpy(newCommand->name, token);

  /* Copy the command name into the arg array as the first element, as required
   * by execvp() and increment the counter.
   */
  newCommand->args[newCommand->numArgs] = calloc(strlen(token) + 1, sizeof(char));
  strcpy(newCommand->args[newCommand->numArgs], token);
  newCommand->numArgs++;

  /* Run through the remainder of the string. If we encounter a '<' character,
   * the next token will get copied into inputFile. If we encounter a '>'
   * character, the next token will get copied into outputFile.
   */
  do {
    token = strtok_r(NULL, " ", &saveptr);
    if (token == NULL) {
      break;
    }
    if (strcmp(token, "<") == 0) {
      // Store the input file name for redirection
      token = strtok_r(NULL, " ", &saveptr);
      newCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
      strcpy(newCommand->inputFile, token);
    } else if (strcmp(token, ">") == 0) {
      // Store the output file for redirection
      token = strtok_r(NULL, " ", &saveptr);
      newCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
      strcpy(newCommand->outputFile, token);
    } else {
      // Store the argument in the args array and increment the counter
      newCommand->args[newCommand->numArgs] = calloc(strlen(token) + 1, sizeof(char));
      strcpy(newCommand->args[newCommand->numArgs], token);
      newCommand->numArgs++;
    }
  }  while (token != NULL);

  // execvp() requires the last element in the arg array to be NULL
  newCommand->args[newCommand->numArgs] = NULL;

	return newCommand;
}

pid_t executeCommand(struct Command* command)
{
  pid_t spawnPid = fork();
  switch (spawnPid) {
    case -1:
      perror("fork()\n");
      fflush(stdout);
      exit(1);
      break;
    case 0:
      // This is what the child is doing
      execvp(command->name, command->args);
      fflush(stdout);
      perror(command->name);
      fflush(stdout);
      exit(2);
      break;
    default:
      // This is what the parent is doing
      spawnPid = waitpid(spawnPid, &command->exitStatus, 0);
      break;
  }
  return spawnPid;
}

/**
 *  This function takes a Command struct as a parameter and frees up the memory
 *  allocated to hold the members of the struct.
 */
void destroyCommand(struct Command* command) 
{
  if (command->name != NULL) {
  free(command->name);
  }
  if (command->inputFile != NULL) {
    free(command->inputFile);
  }
  if (command->outputFile != NULL) {
    free(command->outputFile);
  }
  if (command->numArgs > 0) {
    for (int i = command->numArgs; i >= 0; i--) {
      free(command->args[i]);
    }
  }
  free(command);
}
