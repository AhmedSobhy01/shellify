#ifndef SHELLIFY_H
#define SHELLIFY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

extern char **paths;
extern int pathsCount;

char *readInput(size_t *bufferSize);
int processCommand(char *buffer, size_t bufferSize);
int handleBuiltInCommands(char **args, int argsCount);
char *checkForRedirection(char **args, int *argsCount);
void executeCommand(char **args);

#endif