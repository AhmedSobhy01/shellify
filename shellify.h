#ifndef SHELLIFY_H
#define SHELLIFY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <termios.h>
#include <signal.h>

extern char **paths;
extern int pathsCount;
extern pid_t currentRunningPGroup;

char *readInput(size_t *bufferSize);
int processCommand(char *buffer, size_t bufferSize);
int handleBuiltInCommands(char **args, int argsCount);
void executeCommand(char **args);
void freeSubCommands(char ***subCommands, int numSubCommands, int *subCommandArgCount, int *subCommandArgSize);
int executeCommands(char ***subCommands, int numSubCommands, char *redirectFile);

#endif