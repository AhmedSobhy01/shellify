#include "shellify.h"
#include "utils.h"

char **parseArguments(char *buffer, size_t *bufferSize, int *argsCount)
{
  (*argsCount) = 0;
  int argsSize = 10;
  char **args = malloc(argsSize * sizeof(char *));
  char *token = strtok(buffer, " ");

  while (token != NULL)
  {
    if ((*argsCount) >= argsSize)
    {
      argsSize *= 2;
      args = realloc(args, argsSize * sizeof(char *));
    }

    // Token starts with quote
    if (token[0] == '"' || token[0] == '\'')
    {
      char *quotedArg = malloc((*bufferSize) * sizeof(char));
      char quote = token[0];
      int tokenLen = strlen(token);

      // Token ends with quote
      if (tokenLen > 1 && token[tokenLen - 1] == quote)
      {
        strncpy(quotedArg, token + 1, tokenLen - 2);
        quotedArg[tokenLen - 2] = '\0';
      }
      else
      {
        strcpy(quotedArg, token + 1);

        // Continue until we find the closing quote
        while ((token = strtok(NULL, " ")) != NULL)
        {
          strcat(quotedArg, " ");

          if (token[strlen(token) - 1] == quote)
          {
            strncat(quotedArg, token, strlen(token) - 1);
            break;
          }

          strcat(quotedArg, token);
        }
      }

      args[(*argsCount)++] = strdup(quotedArg);

      free(quotedArg);
    }
    else
    {
      args[(*argsCount)++] = strdup(token);
    }

    token = strtok(NULL, " ");
  }

  args[(*argsCount)] = NULL;

  return args;
}
