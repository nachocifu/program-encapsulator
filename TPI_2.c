#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define ERROR_EXIT 1
#define BUFFER_SIZE 100

int pipes[2][2];

#define READ_FD  0
#define WRITE_FD 1
#define PARENT_WRITE_PIPE  0
#define PARENT_READ_PIPE   1 

#define PARENT_READ_FD  ( pipes[PARENT_READ_PIPE][READ_FD]   )
#define PARENT_WRITE_FD ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )

#define CHILD_READ_FD   ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
#define CHILD_WRITE_FD  ( pipes[PARENT_READ_PIPE][WRITE_FD]  )

int main(int argc, char const *argv[]) {
	// This program only recives 1 argument. None is not valid.
	if (argc != 2){
		printf("Only 1 argument pls.\n");
		return ERROR_EXIT;
	}

	// Copy the command string to memory.
	int command_lenght = strlen(argv[1]) * sizeof(char);
	char * input_command = malloc(command_lenght);
	memcpy(input_command, argv[1], command_lenght);
	
	// First lets read from std input.
	char * buffer = calloc(BUFFER_SIZE - 1, sizeof(char));
	int buffer_index = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1) - 1;

	//Lets open 2 pipes. One for reading and one for writing.
	if (pipe(pipes[0]) || pipe(pipes[1])) {
		printf("Error opening pipes.\n");
		return ERROR_EXIT;
	}

	int pid = fork();
	if (pid == 0) {
		// This is the child.

		// Lets configure the pipes.
		close(STDOUT_FILENO);
		close(STDIN_FILENO);
		dup2(CHILD_READ_FD, STDIN_FILENO); // This conects the output of the pipe form parent to child to the childs STDIN.
		dup2(CHILD_WRITE_FD, STDOUT_FILENO); // Same but the other way.
		close(PARENT_READ_FD);
		close(PARENT_WRITE_FD);

		// Run the command.
		system(input_command);

	} else {
		// This is the parent.

		// Lets configure the pipes.
		close(CHILD_WRITE_FD);
		close(CHILD_READ_FD);

		// Write the command to the child.
		write(PARENT_WRITE_FD, buffer, buffer_index);
		close(PARENT_WRITE_FD);

		// Read the response from the child.
		char * child_response = calloc(BUFFER_SIZE - 1, sizeof(char));
		int child_response_lenght = read(PARENT_READ_FD, child_response, BUFFER_SIZE - 1) - 1;

		// Calculate parity of input.
		unsigned char buffer_parity = 0;
		for (int i = 0; i < buffer_index; i++)
			buffer_parity ^= buffer[i];

		// Calculate parity of output.
		unsigned char output_parity = 0;
		for (int i = 0; i < child_response_lenght; i++)
			output_parity ^= child_response[i];


		// Printing.
		printf("%s\n", child_response);
		fprintf(stderr, "in  parity: 0x%02X\n", buffer_parity);
		fprintf(stderr, "out parity: 0x%02X\n", output_parity);
	}

	return 0;
}