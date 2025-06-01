#include "shellify.h"
#include "commands.h"

void setCurrentDirectory(char *path)
{
  int ret;
  if (path == NULL)
    ret = chdir(getenv("HOME"));
  else
    ret = chdir(path);

  if (ret != 0)
    fprintf(stderr, "Error: Cannot change directory to '%s': No such file or directory\n", path ? path : "HOME");
}

void printCurrentDirectory()
{
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  printf("%s\n", cwd);
}

void setPaths(char **strs, int count)
{
  for (int i = 0; i < pathsCount; i++)
    free(paths[i]);
  free(paths);

  pathsCount = count;
  if (pathsCount == 0)
  {
    paths = NULL;
    return;
  }

  paths = malloc(pathsCount * sizeof(char *));
  for (int i = 0; i < pathsCount; i++)
    paths[i] = strdup(strs[i]);
}

void setUpRedirectionFile(char *redirectFile)
{
  int fd = open(redirectFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if (fd < 0)
  {
    fprintf(stderr, "Error: Cannot open redirection file '%s'\n", redirectFile);
    exit(1);
  }

  if (dup2(fd, STDOUT_FILENO) < 0)
  {
    fprintf(stderr, "Error: Failed to redirect output to file '%s'\n", redirectFile);
    close(fd);
    exit(1);
  }

  close(fd);
}

void runFromPath(char *command, char **args)
{
  // Check if command is from path
  if (command[0] == '/' || strchr(command, '/') != NULL)
  {
    if (access(command, X_OK) == 0)
    {
      execv(command, args);
      fprintf(stderr, "Error: Failed to execute command '%s'\n", command);
      exit(1);
    }
  }
  else // Check if command is in current directory
  {
    char *curDirPath = malloc(strlen(command) + 3);
    strcpy(curDirPath, "./");
    strcat(curDirPath, command);

    if (access(curDirPath, X_OK) == 0)
    {
      execv(curDirPath, args);
      fprintf(stderr, "Error: Failed to execute command '%s'\n", curDirPath);
      free(curDirPath);
      exit(1);
    }
    free(curDirPath);
  }

  // Try paths in PATH
  for (int i = 0; i < pathsCount; i++)
  {
    char *fullPath = malloc(strlen(paths[i]) + strlen(command) + 2);
    strcpy(fullPath, paths[i]);
    strcat(fullPath, "/");
    strcat(fullPath, command);

    // Check if exists, execute
    if (access(fullPath, X_OK) == 0)
    {
      execv(fullPath, args);

      // Execv failed
      fprintf(stderr, "Error: Failed to execute command '%s'\n", command);
    }

    free(fullPath);
  }

  // Command not found
  fprintf(stderr, "Error: Command '%s' not found in current directory or PATH\n", command);

  exit(1);
}
