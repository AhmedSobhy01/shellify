#include "shellify.h"
#include "utils.h"
#include "commands.h"

char **paths = NULL;
int pathsCount = 0;
pid_t currentRunningPGroup = -1;

char *readInput(size_t *bufferSize);
char *checkForRedirection(char **args, int *argsCount);
int isBuiltInCommand(const char *command);
int handleBuiltInCommands(char **args, int argsCount);
int processCommand(char *buffer, size_t bufferSize);
void exitShellify(char *buffer, FILE *input);
void freeSubCommands(char ***subCommands, int numSubCommands, int *subCommandArgCount, int *subCommandArgSize);
void signalHandler(int signum);
int executeCommands(char ***subCommands, int numSubCommands, char *redirectFile);

int main(int argc, char *argv[])
{
  // Default path
  char *defaultPath[] = {"/bin"};
  setPaths(defaultPath, 1);

  size_t bufferSize = 0;
  char *buffer;

  // Signal handlers for SIGINT and SIGTERM
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  // Signal handlers for job control signals
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);

  // Main loop
  while (1)
  {
    buffer = readInput(&bufferSize);

    if (buffer == NULL)
    {
      printf("\n");
      exitShellify(buffer, stdin);
    }

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
    return NULL;
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
        fprintf(stderr, "Error: Redirection '>' requires a filename\n");
        return NULL;
      }
      break;
    }
  }

  return redirectFile;
}

// Check if a command is a built-in command
int isBuiltInCommand(const char *command)
{
  return (strcmp(command, "exit") == 0 || strcmp(command, "cd") == 0 || strcmp(command, "pwd") == 0 || strcmp(command, "path") == 0);
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
      fprintf(stderr, "Error: 'exit' command takes no arguments\n");
      return 1;
    }
    exit(0);
  }
  else if (strcmp(args[0], "cd") == 0)
  {
    if (argsCount > 2)
    {
      fprintf(stderr, "Error: 'cd' command takes at most one argument\n");
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

  // Divide command into subcommands by pipes
  int numSubCommands = 1;
  for (int i = 0; i < argsCount; i++)
    if (strcmp(args[i], "|") == 0)
      numSubCommands++;

  char ***subCommands = malloc(numSubCommands * sizeof(char **));
  int *subCommandArgCount = malloc(numSubCommands * sizeof(int));
  int *subCommandArgSize = malloc(numSubCommands * sizeof(int));

  for (int i = 0; i < numSubCommands; i++)
  {
    subCommandArgSize[i] = 10;
    subCommandArgCount[i] = 0;
    subCommands[i] = malloc(subCommandArgSize[i] * sizeof(char *));
  }

  // Split arguments into subCommands by pipes
  int subCommandIndex = 0;
  for (int i = 0; i < argsCount; i++)
  {
    if (strcmp(args[i], "|") == 0)
    {
      subCommands[subCommandIndex][subCommandArgCount[subCommandIndex]] = NULL;
      subCommandIndex++;
    }
    else
    {
      if (subCommandArgCount[subCommandIndex] >= subCommandArgSize[subCommandIndex])
      {
        subCommandArgSize[subCommandIndex] *= 2;
        subCommands[subCommandIndex] = realloc(subCommands[subCommandIndex], subCommandArgSize[subCommandIndex] * sizeof(char *));
      }

      subCommands[subCommandIndex][subCommandArgCount[subCommandIndex]++] = args[i];
    }
  }

  subCommands[subCommandIndex][subCommandArgCount[subCommandIndex]] = NULL;

  // Check for redirection file in the last command
  char *redirectFile = NULL;
  if (subCommandArgCount[numSubCommands - 1] >= 2 && strcmp(subCommands[numSubCommands - 1][subCommandArgCount[numSubCommands - 1] - 2], ">") == 0)
  {
    redirectFile = subCommands[numSubCommands - 1][subCommandArgCount[numSubCommands - 1] - 1];
    subCommands[numSubCommands - 1][subCommandArgCount[numSubCommands - 1] - 2] = NULL;
    subCommandArgCount[numSubCommands - 1] -= 2;
  }

  // One built in command
  if (numSubCommands == 1 && handleBuiltInCommands(subCommands[0], subCommandArgCount[0]))
  {
    freeSubCommands(subCommands, numSubCommands, subCommandArgCount, subCommandArgSize);
    free(args);
    return 1;
  }

  // First command is a built in command
  if (numSubCommands > 1 && isBuiltInCommand(subCommands[0][0]))
  {
    fprintf(stderr, "Error: Built-in commands cannot be used with pipes\n");
    freeSubCommands(subCommands, numSubCommands, subCommandArgCount, subCommandArgSize);
    free(args);
    return 1;
  }

  executeCommands(subCommands, numSubCommands, redirectFile);

  // Cleanup
  freeSubCommands(subCommands, numSubCommands, subCommandArgCount, subCommandArgSize);
  free(args);
  return 1;
}

// Execute commands with pipes
int executeCommands(char ***subCommands, int numSubCommands, char *redirectFile)
{
  // Create pipes for each subcommand
  int numPipes = numSubCommands - 1;
  int pipes[2 * numPipes];
  pid_t pids[numSubCommands];

  for (int i = 0; i < numPipes; i++)
  {
    if (pipe(pipes + i * 2) < 0)
    {
      fprintf(stderr, "Error: Failed to create pipe\n");
      return 0;
    }
  }

  for (int i = 0; i < numSubCommands; i++)
  {
    pid_t pid = fork();

    if (pid == -1)
    {
      fprintf(stderr, "Error: Failed to create child process\n");
      return 0;
    }

    if (pid == 0)
    {
      // Child process

      // Redirect stdin if not the first command
      if (i > 0)
      {
        if (dup2(pipes[(i - 1) * 2], STDIN_FILENO) < 0)
        {
          fprintf(stderr, "Error: Failed to redirect standard input\n");
          exit(1);
        }
      }

      // Redirect stdout if not the last command
      if (i < numSubCommands - 1)
      {
        if (dup2(pipes[i * 2 + 1], STDOUT_FILENO) < 0)
        {
          fprintf(stderr, "Error: Failed to redirect standard output\n");
          exit(1);
        }
      }

      // Redirection File
      if (i == numSubCommands - 1 && redirectFile != NULL)
        setUpRedirectionFile(redirectFile);

      for (int j = 0; j < 2 * numPipes; j++)
        close(pipes[j]);

      setpgid(0, 0);

      runFromPath(subCommands[i][0], subCommands[i]);

      exit(1);
    }
    else
    {
      // Parent process
      pids[i] = pid;

      // Make first child group leader
      if (i == 0)
      {
        currentRunningPGroup = pid;
        setpgid(pid, currentRunningPGroup);

        // Give terminal control to the first child
        if (tcsetpgrp(STDIN_FILENO, currentRunningPGroup) < 0)
        {
          perror("tcsetpgrp (parent → child)");
        }
      }
      else
      {
        setpgid(pid, currentRunningPGroup);
      }
    }
  }

  // Close all pipes in the parent
  for (int i = 0; i < 2 * numPipes; i++)
    close(pipes[i]);

  // Wait for all child processes
  for (int i = 0; i < numSubCommands; i++)
  {
    int status;
    if (waitpid(pids[i], &status, 0) < 0)
      perror("waitpid");
  }

  // Restore terminal control
  if (tcsetpgrp(STDIN_FILENO, getpid()) < 0)
    perror("tcsetpgrp");

  currentRunningPGroup = -1;

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