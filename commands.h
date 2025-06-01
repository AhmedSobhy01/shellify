#ifndef COMMANDS_H
#define COMMANDS_H

void setCurrentDirectory(char *path);
void printCurrentDirectory();
void setPaths(char **strs, int count);
void runFromPath(char *command, char **args);
void setUpRedirectionFile(char *redirectFile);

#endif
