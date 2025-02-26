// Name: Grace Bertozzi V01012576

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 100
#define MAX_NUM_TOKENS 8
#define MAX_COMMANDS 4


/*
1. read user input
2. parse commands into arguments
3. create pipes
4. fork & execute child processes
5. close pipes and wait for child processes to execute

USEFUL:

int pipe(int pipefd[2]);
    creates a pipe and fills the fd array with two file descriptors
    pipe acts like buffer

    pipefd[2]: This is an array of two integers (file descriptors):
    pipefd[0]: The read-end of the pipe (for reading data).
    pipefd[1]: The write-end of the pipe (for writing data).

    returns 0 on success, else -1

ssize_t read(int fd, void *buf, size_t count);
    fd- file descriptor to read from
    buf- points to where to store input
    count- max number of bytes/chars to read

    returns number of bytes read if successful, else -1 and sets errno

ssize_t write(int fd, const void *buf, size_t count);
    fd: The file descriptor to write to (e.g., 1 for stdout, 2 for stderr).
    buf: A pointer to the data to write.
    count: The number of bytes to write.

    returns number of bytes written if successful, else -1 and sets errno

char *strncpy(char *dest, const char *src, size_t n);
    // copy a specified number of characters from one string to another.
    dest: A pointer to the destination string.
    src: A pointer to the source string.
    n: The maximum number of characters to copy.

    returns pointer to destination string dest

void *memset(void *s, int c, size_t n);
    // fill a block of memory with a specific value.
    s: A pointer to the block of memory to fill.
    c: The value to set (converted to an unsigned char).    
    n: The number of bytes to set.

    returns pointer to memory block s

char *strtok(char *str, const char *delim);
    // tokenizing (splitting) a string based on a given delimiter
    str: The string to be tokenized. On the first call, pass the original string. 
    For subsequent calls, pass NULL to continue tokenizing the same string.
    delim: A string containing delimiter characters that define where the string should be split.

    returns a pointer to the next token found in the string, or NULL when no more tokens are found
*/

// prototypes
void readInput(char input[MAX_COMMANDS][MAX_INPUT_SIZE], char *commands[MAX_COMMANDS][MAX_NUM_TOKENS + 1], int *line_count);
void tokenizeInput(char *input_line, char **command);
void create_pipes(int fds[MAX_COMMANDS - 1][2], int line_count);
void execute_kids(char *commands[MAX_COMMANDS][MAX_NUM_TOKENS + 1], int fds[MAX_COMMANDS - 1][2], int command_count);
void close_pipes(int command_count, int fds[MAX_COMMANDS - 1][2]);

int my_strlen(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// reads all input and calls tokenization helper function on each line of input
void readInput(char input[MAX_COMMANDS][MAX_INPUT_SIZE], char *commands[MAX_COMMANDS][MAX_NUM_TOKENS + 1], int *line_count) {
    int bytes_read;

    while (*line_count < MAX_COMMANDS) {
        bytes_read = read(0, input[*line_count], MAX_INPUT_SIZE - 1); // store each line in input

        if (bytes_read > 1) { // tokenize input
            input[*line_count][bytes_read - 1] = '\0'; // null terminate each line
            tokenizeInput(input[*line_count], commands[*line_count]);

            (*line_count) ++; // increment value of int through pointer
            
            /*// TODO: remove debugging
            //printf("line: %d\n", *line_count);
            //printf("Length of line: %d\n",  my_strlen(input[*line_count]));

            write(1, "You entered: ", 13);
            write(1, input[*line_count], bytes_read - 1); // Print user input
            write(1, "\n", 1);
            */
        }
        else if (bytes_read <= 1) { // empty line entered
            // write(1, "Empty line.\n", 10); // TODO: remove debugging
            break;
        }
    }
}

// tokenizes single line of input
// NOTE: input and command should already pointing at current row in repspective arrays
void tokenizeInput(char *input_line, char **command) {
    if (!input_line) {
        return;
    }
   
    int token_count = 0;
    char *token;

    token = strtok(input_line, " "); // tokenize first based on spaces
    while (token != NULL && token_count < MAX_NUM_TOKENS) {
        if (my_strlen(token) > 0) { 
            command[token_count++] = token; // post increment operation
        }
        token = strtok(NULL, " "); // increment strtok
    }
    command[token_count] = NULL; // null terminate end of command array

    /*
    TODO: remove debugging
    for (int i = 0; i < token_count; i++) {
        printf("token[%d]: %s\n", i, command[i]);
        //printf("count: %d\n", i);

    }*/
}

// pipe() will populate a row in fds for each command
// NOTE: fds[i][0] = read end, fds[i][1] = write end
void create_pipes(int fds[MAX_COMMANDS - 1][2], int command_count) {

    for (int i = 0; i < command_count - 1; i++ ) {
        if (pipe(fds[i]) == - 1) { // creates pipe for each row/command
            write(2, "\nError creating pipe.\n", 30); // if error creating
            exit(1);
        } 
    }
}

void execute_kids(char *commands[MAX_COMMANDS][MAX_NUM_TOKENS + 1], int fds[MAX_COMMANDS - 1][2], int command_count) {
   
    for (int i = 0; i < command_count; i ++) {
        pid_t pid = fork(); // create

        if (pid == 0) { // kids enter

            if (i > 0) {
                dup2(fds[i - 1][0], 0); // redirect input for all but first kid
            }
            if (i < command_count - 1) {
                dup2(fds[i][1], 1); // redirect output for all but last kid
            }
            for (int j = 0; j < command_count - 1; j++) {
                close(fds[j][0]);
                close(fds[j][1]);
            }
            execvp(commands[i][0], commands[i]); // execute current comand i.e. overwrite memory space
            exit(1); // if exec fails
        }
    }
    //printf("All commands executed. Parent process exiting.\n"); // TODO: remove debugging
}

// close pipe fds open in the parent process
void close_pipes(int command_count, int fds[MAX_COMMANDS - 1][2]) {
    for (int i = 0; i < command_count - 1; i++) {
        close(fds[i][0]); // close read end
        close(fds[i][1]); // close write end
    }
}

int main() {
    char input[MAX_COMMANDS][MAX_INPUT_SIZE]; // static 2d array
    char *commands[MAX_COMMANDS][MAX_NUM_TOKENS + 1]; // 2d array of pointers
    int command_count = 0; // track lines/commands entered

    readInput(input, commands, &command_count); // read user input and call helper function to tokenize

    if (command_count > 1) {
        int fds[command_count - 1][2]; // create array to hold each end of pipe for all but last command
        create_pipes(fds, command_count);
        execute_kids(commands, fds, command_count);
        close_pipes(command_count, fds);
    } else if (command_count == 1) { // If there is only one command, execute it without pipes
        execvp(commands[0][0], commands[0]);
    }

    return 0;
}
