/* All code below was written by Christelle Nieves*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_LINE_LENGTH 512
#define MAX_LINE_LENGTH_PLUS 1024
#define MAX_NUM_COMMANDS 64
#define MAX_COMMAND_LENGTH 256

void interactiveMode();
void batchMode(char *);
void parse();
void execute();
void resetInputLine();
void resetAllCommands();
void resetCurrentCommand();

// Global variables
char inputLine[MAX_LINE_LENGTH_PLUS];
char *allCommands[MAX_NUM_COMMANDS];
char *currentCommand[MAX_COMMAND_LENGTH];
int exitFlag = 0;
int batchModeFlag = 0;
char *token = NULL;

// This function clears the input line string
void resetInputLine()
{
    memset(inputLine, 0, MAX_LINE_LENGTH);
}

// This function clears the allCommands array
void resetAllCommands()
{
    memset(allCommands, 0, MAX_NUM_COMMANDS);
}

// This function clears the currentCommand array
void resetCurrentCommand()
{
    memset(currentCommand, 0, MAX_COMMAND_LENGTH);
}

// This function parses the input and separates it into tokens according to the delimiter
// Then it parses those tokens into separate commands and calls the execute function on them
void parse()
{
    int i, j, charFlag, index;

    // Check for too many characters
    if (strlen(inputLine) > MAX_LINE_LENGTH)
    {
        perror("Error: Command too long. Retry");
        resetInputLine();
        return;
    }

    // Check if first character is space/null/carriage return
    if (isspace(inputLine[0]) || inputLine[0] == 0 || inputLine[0] == 13)
    {
        charFlag = 0;
        index = 0;

        // Loop through the input string and check if it contains any commands
        while (isspace(inputLine[index]))
        {
            // If we encounter a non-whitespace character, trigger the flag
            if (!isspace(inputLine[index + 1]) && inputLine[index + 1] != 0)
            { 
                charFlag = 1;
            }
            index++;
        }
        // If we did not encounter any non-whitespace characters, we will not continue parsing this line
        if (charFlag == 0)
        {
            return;
        }
        // If the input did contain a command, trim the leading whitespace so we can continue parsing the string
        else if (index != 0)
        {
            int c = 0;
            while (inputLine[c + index] != '\0')
            {
                inputLine[c] = inputLine[c + index];
                c++;
            }
            inputLine[c] = '\0';
        }
    }

    // Split line by the delimiter
    token = strtok(inputLine, ";");

    // Trigger exit flag if we find a quit command
    if (strcmp(token, "quit") == 0)
    {
        exitFlag = 1;
    }

    i = 0;
    // Loop to add all commands to the allCommands array
    while (token != NULL)
    {
        if (strcmp(token, " ") != 0)
        {
            allCommands[i] = token;
            ++i;
        }
        token = strtok(NULL, ";");
    }

    allCommands[i] = NULL; // Add null terminator after the last command

    // Loop through all commands and parse/add to the current command array
    for (i = 0; allCommands[i]; i++)
    {
        j = 0;
        token = strtok(allCommands[i], " \t\n");

        while (token != NULL)
        {
            if (strcmp(token, "quit") == 0)
            {
                exitFlag = 1;
            }

            currentCommand[j++] = token;
            token = strtok(NULL, " \t\n");
        }

        currentCommand[j] = NULL;

        // Execute the current command and reset the array
        execute();
        resetCurrentCommand();
    }

    if (exitFlag)
    {
        return;
    }

    resetInputLine();
    resetAllCommands();
}

// This function executes a command
void execute()
{
    int pid = fork();
    int status;

    if (pid == 0)
    {
        // Execute the command
        execvp(currentCommand[0], currentCommand);
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        // Wait for all processes to finish
        while (wait(&status) == pid)
        {
            if (status == 0)
            {
                printf("PID %d exited with status %d\n", pid, status);
            }
        }
    }
}

// This function runs the shell in batch mode using a file name that is passed in as a string
// Batch mode reads commands from a file and does not display a prompt to the screen
void batchMode(char *fileName)
{
    FILE *fp = fopen(fileName, "r");

    if (fp != NULL)
    {
        // Get input from the file and call the parse function on each line
        while (fgets(inputLine, sizeof(inputLine), fp) != NULL)
        {
            printf("Retrieved line of length %d :\n+%s\n", (int)strlen(inputLine), inputLine);
            parse();

            if (exitFlag)
            {
                exit(EXIT_SUCCESS);
            }
        }
    }
    else
    {
        perror("Error: File does not exist");
    }

    exit(0);
}

// This function runs the shell in interactive mode
// Interactive mode displays a prompt to the screen and takes in user input from the command line
void interactiveMode()
{
    printf("Shell--->");

    // Get input from the command line and call the parse function
    while (fgets(inputLine, sizeof(inputLine), stdin) != NULL)
    {
        parse();

        if (exitFlag)
        {
            exit(EXIT_SUCCESS);
        }

        printf("Shell--->");
    }

    exit(0);
}

int main(int argc, char *argv[])
{
    resetInputLine();
    resetAllCommands();
    resetCurrentCommand();

    // Check the number of arguments that were passed into the  program
    if (argc == 2)
    { // If we have 2 arguments the shell will run in batch mode
        batchModeFlag = 1;
        char *fileName = argv[1];
        printf("FILE IS: %s, argv[1] is %s\n", fileName, argv[1]);
        batchMode(fileName);
    }
    else if (argc == 1)
    { // If we have 1 argument the shell will run in interactive mode
        interactiveMode();
    }
    else
    { // Any other number of arguments will trigger an error
        perror("Error: Wrong number of commands");
    }

    return 0;
}