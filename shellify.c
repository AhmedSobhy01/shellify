#include "shellify.h"
#include "utils.h"
#include "commands.h"

char **paths = NULL;
int pathsCount = 0;

char *readInput(size_t *bufferSize);
char *checkForRedirection(char **args, int *argsCount);
int handleBuiltInCommands(char **args, int argsCount);
void executeCommand(char **args);
int processCommand(char *buffer, size_t bufferSize);
void exitShellify(char *buffer, FILE *input);
void freeSubCommands(char ***subCommands, int numSubCommands, int *subCommandArgCount, int *subCommandArgSize);
void signalHandler(int signum);

int main(int argc, char *argv[])
{
  // Default path
  char *defaultPath[] = {"/bin"};
  setPaths(defaultPath, 1);

  size_t bufferSize = 0;
  char *buffer;

  // Main loop
  while (1)
  {
    buffer = readInput(&bufferSize);
    processCommand(buffer, bufferSize);
    free(buffer);
  }

  return 0;
}

// Read user input from stdin
char *readInput(size_t *bufferSize)
{
  char *buffer = NULL;
  ssize_t inputSize;

  printf("shellify> ");
  inputSize = getline(&buffer, bufferSize, stdin);

  if (inputSize == -1)
  {
    free(buffer);
    exit(0);
  }

  if (buffer[inputSize - 1] == '\n')
    buffer[inputSize - 1] = '\0';

  return buffer;
}

// Check for redirection file
char *checkForRedirection(char **args, int *argsCount)
{
  char *redirectFile = NULL;

  for (int i = 0; i < *argsCount; i++)
  {
    if (strcmp(args[i], ">") == 0)
    {
      if (i + 1 < *argsCount)
      {
        redirectFile = args[i + 1];
        args[i] = NULL;
        *argsCount = i;
      }
      else
      {
        fprintf(stderr, "An error has occurred!\n");
        return NULL;
      }
      break;
    }
  }

  return redirectFile;
}

// Handle built-in commands
int handleBuiltInCommands(char **args, int argsCount)
{
  if (args[0] == NULL)
    return 1;

  if (strcmp(args[0], "exit") == 0)
  {
    if (argsCount > 1)
    {
      fprintf(stderr, "An error has occurred!\n");
      return 1;
    }
    exit(0);
  }
  else if (strcmp(args[0], "cd") == 0)
  {
    if (argsCount > 2)
    {
      fprintf(stderr, "An error has occurred!\n");
      return 1;
    }
    setCurrentDirectory(argsCount > 1 ? args[1] : NULL);
    return 1;
  }
  else if (strcmp(args[0], "pwd") == 0)
  {
    printCurrentDirectory();
    return 1;
  }
  else if (strcmp(args[0], "path") == 0)
  {
    setPaths(args + 1, argsCount - 1);
    return 1;
  }

  return 0; // Not a built-in command
}

// Execute external command
void executeCommand(char **args)
{
  pid_t pid = fork();

  if (pid == 0)
  {
    // Child process
    runFromPath(args[0], args);
  }
  else if (pid > 0)
  {
    // Parent process
    int status;
    waitpid(pid, &status, 0);
  }
  else
  {
    // Fork failed
    fprintf(stderr, "An error has occurred!\n");
  }
}

// Process a command from the input buffer
int processCommand(char *buffer, size_t bufferSize)
{
  int argsCount;
  char **args = parseArguments(buffer, &bufferSize, &argsCount);

  if (args[0] == NULL)
  {
    free(args);
    return 1;
  }

  // Check for redirection
  char *redirectFile = checkForRedirection(args, &argsCount);

  // Handle built-in commands
  if (handleBuiltInCommands(args, argsCount))
  {
    free(args);
    return 1;
  }

  pid_t pid = fork();

  if (pid == 0)
  {
    // Child process
    if (redirectFile != NULL)
      setUpRedirectionFile(redirectFile);

    runFromPath(args[0], args);
  }
  else if (pid > 0)
  {
    // Parent process
    int status;
    waitpid(pid, &status, 0);
  }
  else
  {
    // Fork failed
    fprintf(stderr, "An error has occurred!\n");
  }

  free(args);
  return 1;
}

// Exit the shell and clean up
void exitShellify(char *buffer, FILE *input)
{
  free(buffer);

  if (input != stdin)
    fclose(input);

  for (int i = 0; i < pathsCount; i++)
    free(paths[i]);
  free(paths);

  exit(0);
}

// Clean up for subcommands data
void freeSubCommands(char ***subCommands, int numSubCommands, int *subCommandArgCount, int *subCommandArgSize)
{
  for (int i = 0; i < numSubCommands; i++)
    free(subCommands[i]);

  free(subCommands);
  free(subCommandArgCount);
  free(subCommandArgSize);
}

// Signal handler for SIGINT and SIGTERM
void signalHandler(int signum)
{
  if (currentRunningPGroup != -1)
  {
    killpg(currentRunningPGroup, signum);
  }
  else
  {
    printf("\nshellify> ");
    fflush(stdout);
  }
}