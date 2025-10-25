
#ifndef command_h
#define command_h

struct SimpleCommand
{
	int _numberOfAvailableArguments;

	// Actual number of arguments
	int _numberOfArguments; 

	// Array of strings representing arguments
	char **_arguments;

	// Constructor
	SimpleCommand();

	// Insert an argument into _arguments array
	void insertArgument(char *argument);
};

struct Command
{
	int _numberOfAvailableSimpleCommands;
	int _numberOfSimpleCommands;

	// Array of simple command pointers
	SimpleCommand **_simpleCommands;

	char *_outFile;    // For > redirection/overwrite and >> append
	char *_inputFile;  // For < redirection
	char *_errFile;    // For 2> redirection
	int  _out_error;   // For 2>&1 redirection
	int _background;   // For & background
	int _append;       // For >> append

	// To be implemented
	void prompt();
	void print();
	void execute();
	void clear();

	// Constructor
	Command();

	// Insert a simple command into _simpleCommands array
	void insertSimpleCommand(SimpleCommand *simpleCommand);

	// Static variables for the current command being processed
	static Command _currentCommand;
	static SimpleCommand *_currentSimpleCommand;
};

#endif
