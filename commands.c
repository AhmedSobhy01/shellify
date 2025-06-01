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
    fprintf(stderr, "Invalid path!\n");
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
    fprintf(stderr, "An error has occurred!\n");
    exit(1);
  }

  if (dup2(fd, STDOUT_FILENO) < 0)
  {
    fprintf(stderr, "An error has occurred!\n");
    close(fd);
    exit(1);
  }

  close(fd);
}

void runFromPath(char *command, char **args)
{
  for (int i = 0; i < pathsCount; i++)
  {
    char *fullPath = malloc(strlen(paths[i]) + strlen(args[0]) + 2);
    strcpy(fullPath, paths[i]);
    strcat(fullPath, "/");
    strcat(fullPath, args[0]);

    // Check if exists, execute
    if (access(fullPath, X_OK) == 0)
    {
      execv(fullPath, args);

      // Execv failed
      fprintf(stderr, "An error has occurred!\n");
    }

    free(fullPath);
  }

  // Command not found
  fprintf(stderr, "An error has occurred!\n");

  exit(1);
}
