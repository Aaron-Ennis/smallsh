/* 
 * Filename: command.h
 * Author: Aaron Ennis
 * Email: ennisa@oregonstate.edu
 * Last modified: 31 October 2020
 * Description: This is the declaration/interface file that defines a structure
 * to contain information about a command, and some simple functions to
 * operate on the structure.
 */

#include <sys/types.h>
#include <unistd.h>
#ifndef COMMAND_H
#define COMMAND_H

// Define the command struct
struct Command
{
  char* name;
  char* args[512];  // We're only supporting up to 512 arguments
  char* inputFile;
  char* outputFile;
  int numArgs;
  int exitStatus;
  int termSig;
  int runScope; // 0 = foreground, 1 = background
};

struct Command* createCommand(char* rawData);
void destroyCommand(struct Command* command);
int executeCommand(struct Command* command);

#endif