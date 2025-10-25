
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


void parse(std::vector<Token> &tokens)
{
    // Initialize the first simple command for this new command
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);

    // Given a list of tokens, the task is to parse them and fill the Command structure
    for (size_t i = 0; i < tokens.size(); ++i) 
    {
        Token token = tokens[i];

        switch (token.type)
        {
            // Both are strings to be added as arguments
            case TOKEN_COMMAND:
            case TOKEN_ARGUMENT:
            {
                // Get the argument using string duplication
                char *arg = strdup(token.value.c_str());
                Command::_currentSimpleCommand->insertArgument(arg);
                break;
            }

            // Output redirection: next token has to be an output file (file name)
            case TOKEN_REDIRECT: // >
            {
                if (i + 1 < tokens.size() && tokens[i+1].type == TOKEN_ARGUMENT) 
                {

                    // Add the filename to the command's output file.
                    Command::_currentCommand._outFile = strdup(tokens[i+1].value.c_str());
                    
                    // Skip the file name token
                    i++;
                } 
                else // Error: no file name provided 
                {
                    fprintf(stderr, "syntax error near `>'\n");

                    // Clean up the current command and stop parsing
                    Command::_currentCommand.clear(); 
                    return; // Stop parsing this line
                }
                break;
            }

            // Append output redirection: next token has to be an output file (file name)
            // Just like TOKEN_REDIRECT but set the append flag
            case TOKEN_APPEND: // >>
            {
                if (i + 1 < tokens.size() && tokens[i+1].type == TOKEN_ARGUMENT) 
                {
                    Command::_currentCommand._outFile = strdup(tokens[i+1].value.c_str());
                    Command::_currentCommand._append = 1;

                    i++; // Skip the filename
                }
                else // Error
                {
                    fprintf(stderr, "syntax error near `>'\n");
                    Command::_currentCommand.clear(); 
                    return;
                }
                break;
            }

            // Input redirection: next token is the input file
            case TOKEN_INPUT: // <
            {
                if (i + 1 < tokens.size() && tokens[i+1].type == TOKEN_ARGUMENT) 
                {
                    // Add the filename to the command's input file
                    Command::_currentCommand._inputFile = strdup(tokens[i+1].value.c_str());
                    i++;
                } 
                else // Error
                {
                     fprintf(stderr, "syntax error near `<'\n");
                     Command::_currentCommand.clear();
                     return;
                }
                break;
            }

            // Error redirection: next token is the error file
            case TOKEN_ERROR: // 2>
            {
                if (i + 1 < tokens.size() && tokens[i+1].type == TOKEN_ARGUMENT) 
                {
                    // Add the filename to the command's error file
                    Command::_currentCommand._errFile = strdup(tokens[i+1].value.c_str());
                    i++;
                } 
                else 
                {
                    fprintf(stderr, "syntax error near `2>'\n");
                    Command::_currentCommand.clear();
                    return;
                }
                break;
            }

            // Redirect all output and error
            // Just like TOKEN_ERROR but also set the _out_error flag
            case TOKEN_REDIRECT_AND_ERROR: // >>&
            {
                if (i + 1 < tokens.size() && tokens[i+1].type == TOKEN_ARGUMENT) {
                    Command::_currentCommand._errFile = strdup(tokens[i+1].value.c_str());
                    Command::_currentCommand._out_error = 1; 
                    i++;
                } else {
                    fprintf(stderr, "syntax error near `>>&'\n");
                    Command::_currentCommand.clear();
                    return;
                }
                break;
            }

            // Pipe means a new simple command
            case TOKEN_PIPE: // |
            {
                Command::_currentSimpleCommand = new SimpleCommand();
                Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);

                // This means that the next tokens will get added to the new simple command structure

                break;
            }

            case TOKEN_BACKGROUND:
            {
                // Only set the background flag
                Command::_currentCommand._background = 1;
                break;
            }

            case TOKEN_EOF:
            {
                // end of input;
                break;
            }

            default:
            {    
                // ignore unknown tokens
                break;
            }
        }
    }
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;

    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;

    // Initialize the rest of the flags
    _append = 0;
    _out_error = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    // If the command is empty, just return to the prompt
    if (_numberOfSimpleCommands == 0 || _simpleCommands[0]->_numberOfArguments == 0) 
    {
        clear();
        return;
    }

    print();

    // Quick check for built-in commands
    // Exit
    if (strcmp(_simpleCommands[0]->_arguments[0], "exit") == 0) 
    {
        printf("Exiting...\n");
        clear();
        exit(0);
    }

    // Change Directory
    if (strcmp(_simpleCommands[0]->_arguments[0], "cd") == 0) 
    {
        char *dir;
        if (_simpleCommands[0]->_numberOfArguments > 1) 
        {
            // Get dir from argument
            dir = _simpleCommands[0]->_arguments[1];
        } 
        else // No argument given
        {
            // Change to home directory
            dir = getenv("HOME");
            if (dir == NULL) 
            {
                perror("getenv(HOME)");
                dir = "."; // Current directory as fallback
            }
        }
        
        // Printing the directory change to pass test case
        printf("Changing to directory '%s'\n", dir);

        if (chdir(dir) < 0) // No such file or directory
        {
            perror("cd");
        } 
        else 
        {
            char cwd[1024];

            // Get current working directory 
            if (getcwd(cwd, sizeof(cwd)) != NULL) // Success
            {
                printf("You are now in %s\n", cwd);
            }
        }
        
        clear(); // Clear the 'cd' command
        return;  // Return to prompt
    }

    // Start of execution logic

    // Default file descriptors
    int default_in = dup(STDIN_FILENO);
    int default_out = dup(STDOUT_FILENO);
    int default_err = dup(STDERR_FILENO);

    pid_t last_pid; // To wait for the last process in the pipeline

    // Current input file descriptor (start with keyboard input)
    int input_fd = default_in;
    
    int pipe_fd[2]; // Variable for the pipe


    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        // Get first simple command
        SimpleCommand *cmd = _simpleCommands[i];

        // Setup default output/error
        dup2(default_out, STDOUT_FILENO);
        dup2(default_err, STDERR_FILENO);

        // Save current_input_fd since input_fd will be modified
        int current_input_fd = input_fd;

        int output_fd; // File descriptor for the output of the command

        // If it's the last command, redirect output to file or terminal
        if (i == _numberOfSimpleCommands - 1) 
        {
            output_fd = default_out;
        } 
        else // If not last command, create a pipe
        {
            // Check for pipe failure
            if (pipe(pipe_fd) < 0) 
            {
                perror("Pipe error");
                exit(1);
            }

            // Set output to write end of pipe
            output_fd = pipe_fd[1]; // This command writes to the pipe
            input_fd = pipe_fd[0];  // The next command reads from the pipe
        }

        // Check for file input on the first command before fork so that it doesn't overwrite input_fd
        // if (i == 0 && _inputFile) {
        //     int fd = open(_inputFile, O_RDONLY);

        //     // Failed to open the file
        //     if (fd < 0) {
        //         perror("open(inputFile)");
        //         exit(1); 
        //     }

        //     input_fd = fd; // The new input is the file
        // }

        // Fork a child process for each simple command
        pid_t pid = fork();

        if (pid < 0) 
        { 
            // perror prints a system error message followed by the argument string
            perror("Fork failed");
            clear();
            return;
        }

        // Run child process which will execute the simple command
        if (pid == 0) // Child process
        {
            // Rewire input/output of child process to appropriate fds
            if (current_input_fd != default_in) {
                dup2(current_input_fd, STDIN_FILENO);
                close(current_input_fd);
            }
            if (output_fd != default_out) {
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }

            // Output file redirection only applies to the last command
            if (i == _numberOfSimpleCommands - 1) // Last command check
            {
                // Check for Output Redirection (>, >>)
                if (_outFile) 
                {
                    int fd;

                    if (_append) // This means we have >> (append)
                    {
                        fd = open(_outFile, O_CREAT | O_WRONLY | O_APPEND, 0666);
                    } 
                    else // This means we have > (overwrite)
                    { 
                        fd = open(_outFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                    }
                    if (fd < 0) 
                    {
                        perror("open(outFile)");
                        exit(1); // Exit child
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
            }
            
            // Input file redirection only applies to the first command (Double check)
            // To be deleted 
            if (i == 0) // First command check
            {
                // Check for Input Redirection (<)
                if (_inputFile) 
                {
                    // Verify that input redirection is handled
                    int fd = open(_inputFile, O_RDONLY);
                    if (fd < 0) 
                    {
                        perror("open(inputFile)");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
            }


            // Error file redirection check for last command
            if (i == _numberOfSimpleCommands - 1) // Last command check
            {
                // Check for Error Redirection (2>, >>&)
                if (_errFile) 
                {
                    int fd;

                    if (_out_error) 
                    {
                        // Appending (>>&)
                        fd = open(_errFile, O_CREAT | O_WRONLY | O_APPEND, 0666);
                        
                        dup2(fd, STDOUT_FILENO);
                    } 
                    else 
                    {
                        // Overwriting (2>)
                        fd = open(_errFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                    }

                    if (fd < 0) // File error
                    {
                        perror("open(errFile)");
                        exit(1);
                    }
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                }
            }

            // Close default FDs in the child
            close(default_in);
            close(default_out);
            close(default_err);

            // Child must close the read end of the next pipe
            if (i < _numberOfSimpleCommands - 1) {
                close(input_fd); // 'input_fd' holds the read end of the next pipe
            }

            // Finally execute commands with execvp
            // Takes command name and array of arguments, first argument being the command itself
            execvp(cmd->_arguments[0], cmd->_arguments);
            
            // If execvp returns, it means there was an error
            perror("execvp");
            exit(1);
        }
        else // Parent process
        {
            // Save the PID of the last-forked child
            last_pid = pid;

            // Parent must close its ends of the pipe so that the shell doesn't wait
            // Also for child process to receive EOF
            if (current_input_fd != default_in) 
            {
                close(current_input_fd);
            }
            if (output_fd != default_out) 
            {
                close(output_fd);
            }
        }
        
        // The input for the next loop iteration is the read end of the pipe we just created
        if (i < _numberOfSimpleCommands - 1) // If not the last command
        {
            input_fd = pipe_fd[0];
        } 
        else 
        {
            input_fd = default_in;
        }        
    }

    // Done with all simple commands and handling the pipeline

    // Restore the shell's default I/O after finishing everything
    dup2(default_in, STDIN_FILENO);
    dup2(default_out, STDOUT_FILENO);
    dup2(default_err, STDERR_FILENO);
    close(default_in);
    close(default_out);
    close(default_err);

    // Only skip if background execution is set
    if (!_background) {
        int status; 

        // Waits for last child process to finish
        waitpid(last_pid, &status, 0);
    }

    // Finally clear the command to prepare for the next one
    clear();
}

void Command::prompt()
{
    // Modified the prompt function to continuously read, parse, and execute commands
    std::string input;

    while (true)
    {
        // Print the prompt
        printf("myshell>");
        fflush(stdout);

        // Read a line of input
        if (!std::getline(std::cin, input)) {
            // Handle (Ctrl+D) to exit the shell
            printf("\nExiting...\n");
            exit(0);
        }

        // Tokenize the input
        std::vector<Token> tokens = tokenize(input);

        // Parse the tokens into the _currentCommand
        parse(tokens);

        // Execute the _currentCommand
        Command::_currentCommand.execute();
    }
}

// Signal handler for SIGCHLD to delete processes that have ended
void onChildExit(int sig) 
{
    while (waitpid(-1, NULL, WNOHANG) > 0);

    // Create log file
    int log_fd = open("log.txt", O_CREAT | O_WRONLY | O_APPEND, 0666);

    if (log_fd < 0) // Error opening log file
    {
        perror("open(log.txt)");
        return; 
    }

    // Write the log message
    const char *msg = "A child process terminated.\n";

    // Append message to log file
    write(log_fd, msg, strlen(msg));

    // Close the log file.
    close(log_fd);
}

void setup() 
{
    // Ignore SIGINT in the shell, which is generated by Ctrl+C
    signal(SIGINT, SIG_IGN);

    // Call onChildExit() when a child process ends
    signal(SIGCHLD, onChildExit);
}

// Global current command
Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    setup();
    Command::_currentCommand.prompt();
    return 0;
}