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
#include <signal.h>

// Define the command struct
struct Command
{
  char* name;
  char* args[512];
  char* inputFile;
  char* outputFile;
  int numArgs;
  int exitStatus;
  pid_t myPid;
  int runScope; // 0 = foreground, 1 = background
};

struct Command* createCommand(char* rawData);
void destroyCommand(struct Command* command);
int executeCommand(struct Command* command, int fgOnly);

#endif