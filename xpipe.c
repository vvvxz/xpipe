#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Usage: xpipe <cmd> | <receiving cmd> & <receiving cmd> &...\n");
		exit(0);
	}

	// Determine how many args are part of the first command.
	int num_args = -1;
	// Start checking at 3
	// Don't check the last arg, if we haven't found it by then, exit.
	for (int i = 2; i < argc; i++) {
	  if (memcmp(argv[i], "!", 2) == 0) {
			num_args = i - 2;
		}
	}
	if (num_args == -1) {
    printf("Usage: xpipe <cmd> ! <receiving cmd> , <receiving cmd> ,...\n");
    exit(0);
	}

	int pipefds[2];
	if (pipe(pipefds)) {
    perror("Failed to create pipe");
    exit(1);
  }

  int rc = fork();
  if (rc < 0) {
		perror("fork failed");
		exit(1);
  }

  if (rc == 0) {
    // Child Process
    char* args[num_args + 2];
    args[num_args + 1] = NULL;
    // No need to save num_args as this is the child process
    while (num_args > -1) {
      args[num_args] = strdup(argv[num_args + 1]);
      num_args--;
    }
    dup2(pipefds[1], STDOUT_FILENO);
    execvp(args[0], args);
  } else {
    // Parent Process
    // Read data with exponential sizes until EOF
    char* child_output = NULL;
    size_t output_size = 20;
    size_t data_read = 0;
    do {
      output_size *= 2;
      child_output = realloc(child_output, output_size);
      data_read += read(pipefds[0], child_output + data_read, output_size - data_read);
    } while (data_read == output_size);

    // Output should handle /n and /0 chars.

    printf("%s", child_output);

    // TODO: send to the next processes.
  }

	return 0;
}
