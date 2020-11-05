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
#include <fcntl.h>

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
  char *devNull = "/dev/null";  // For background commands w/o redirect
	struct Command* newCommand = malloc(sizeof(struct Command));
  // Default exitStatus and runScope for built-in commands
  //newCommand->exitStatus = 0;
  newCommand->runScope = 0;   // 0 = foreground, 1 = background
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
      // Store the file name for input redirection
      token = strtok_r(NULL, " ", &saveptr);
      newCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
      strcpy(newCommand->inputFile, token);
    } else if (strcmp(token, ">") == 0) {
      // Store the file name for output redirection
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

  // For background commands without input or output redirection specified, 
  // point input and/or output to "/dev/null"
  if (newCommand->runScope == 1) {
    if (newCommand->inputFile == NULL) {
      newCommand->inputFile = calloc(strlen(devNull) + 1, sizeof(char));
      strcpy(newCommand->inputFile, devNull);
    }

    if (newCommand->outputFile == NULL) {
      newCommand->outputFile = calloc(strlen(devNull) + 1, sizeof(char));
      strcpy(newCommand->outputFile, devNull);
    }
  }

  // execvp() requires the last element in the arg array to be NULL
  newCommand->args[newCommand->numArgs] = NULL;

	return newCommand;
}

/**
 *  This function takes a Command struct and an int as parameters. The int
 *  indicates if the shell is in foreground-only mode so we can set the
 *  run scope of the command appropriately.
 *  It opens input and output files if redirection was indicated in the command
 *  and then redirects stdin and stdout as appropriate.
 *  It then spawns a child process to run the command using an exec() function.
 */
int executeCommand(struct Command* command, int fgOnly)
{
  pid_t spawnPid;
  int inputFD, outputFD, dupResult;

  // User has forced fg-only mode, so set the runScope to match.
  if (fgOnly == 1) {
    command->runScope = 0;
  }

  // Open file for input if applicable
  if (command->inputFile != NULL) {
    inputFD = open(command->inputFile, O_RDONLY);
    if (inputFD == -1) {
      printf("cannot open %s for input\n", command->inputFile);
      fflush(stdout);
      return 1;
    }
  }

  // Open file for output if applicable
  if (command->outputFile != NULL) {
    outputFD = open(command->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outputFD == -1) {
      printf("cannot open %s for output\n", command->outputFile);
      fflush(stdout);
      return 1;
    }
  }

  // Fork the shell and store the child pid in the command struct
  spawnPid = fork();
  switch (spawnPid) {
    case -1:
      // Fork failed
      perror("Unable to fork process\n");
      fflush(stdout);
      return 1;
      break;
    case 0:
      // This is what the child is doing
      // Redirect input if applicable
      if (command->inputFile != NULL) {
        dupResult = dup2(inputFD, 0);
        if (dupResult == -1) {
          perror("Input redirect");
          fflush(stdout);
          exit(1);
        }
      }
      // Redirect output if applicable
      if (command->outputFile != NULL) {
        dupResult = dup2(outputFD, 1);
        if (dupResult == -1) {
          perror("Output redirect");
          fflush(stdout);
          exit(1);
        }
      }
      execvp(command->name, command->args);
      fflush(stdout);
      perror(command->name);
      fflush(stdout);
      break;
    default:
      if (command->runScope == 1) {
      // If running in the background, return control to user prompt
        printf("background pid is %d\n", spawnPid);
        fflush(stdout);
        command->myPid = spawnPid;
      } else {
      // Otherwise wait for process to terminate before returning control
        spawnPid = waitpid(spawnPid, &command->exitStatus, 0);
        if (WIFEXITED(command->exitStatus)) {
          return WEXITSTATUS(command->exitStatus);
        } else {
          return WTERMSIG(command->exitStatus);
        }
      }
      break;
  }

  return 0;
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
