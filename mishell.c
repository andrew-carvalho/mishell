#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* constants */
#define MISHELL_LINE_BUFFERSIZE 1024
#define MISHELL_TOKEN_BUFFERSIZE 64
#define MISHELL_TOKEN_DELIMITERS " \t\r\n\a"

/* builtins */
int mishell_cd(char **args);
int mishell_help(char **args);
int mishell_exit(char **args);

char *builtin_str[] = {"cd", "help", "exit"};

int (*builtin_func[])(char **) = {&mishell_cd, &mishell_help, &mishell_exit};

int mishell_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }

int mishell_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "mishell: expected argumento to \"cd\"\n");
    return 1;
  }

  if (chdir(args[1]) != 0) {
    perror("mishell");
  }

  return 1;
}

int mishell_help(char **args) {
  printf("Andrew Ignacio's mishell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("Builtin commands:\n");
  for (int i = 0; i < mishell_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }
  printf("Use the man command for information on other programs.\n");
  return 1;
}

int mishell_exit(char **args) { return 0; }

/* shell functions */
int mishell_launch(char **args) {
  pid_t pid;
  pid_t wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("mishell");
    }
    exit(EXIT_FAILURE);
  }

  if (pid < 0) {
    perror("mishell");
  }

  do {
    wpid = waitpid(pid, &status, WUNTRACED);
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));

  return 1;
}

int mishell_execute(char** args) {
  if(args[0] == NULL) {
    return 1;
  }

  for(int i =0; i < mishell_num_builtins(); i++) {
    if(strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return mishell_launch(args);
}

char **mishell_split_line(char *line) {
  int bufsize = MISHELL_TOKEN_BUFFERSIZE;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "mishell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, MISHELL_TOKEN_DELIMITERS);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += MISHELL_TOKEN_BUFFERSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "mishell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, MISHELL_TOKEN_DELIMITERS);
  }

  tokens[position] = NULL;
  return tokens;
}

char *mishell_read_line() {
  int bufsize = MISHELL_LINE_BUFFERSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "mishell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();

    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    }

    buffer[position] = c;
    position++;

    if (position >= MISHELL_LINE_BUFFERSIZE) {
      bufsize += MISHELL_LINE_BUFFERSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "mishell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

void mishell_loop() {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");

    line = mishell_read_line();
    args = mishell_split_line(line);
    status = mishell_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  mishell_loop();
  return EXIT_SUCCESS;
}
