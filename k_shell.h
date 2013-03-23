#ifndef SHELL_H
#define SHELL_H

#include "common.h"

/*adds a " ->" indent*/
void addShellIndent();

/*gets string input from previous line*/
void getTypedText(int charCount, int startingYPos, int cursor_y, char *c);

/*reads inputs for shell*/
void executeInput(char *input, char *arguements);

/*gets the input and spits it into command (written in char *input string) and arguments*/
void formatInput(char *input, char *arguements);

/*function that removes trailing space if there is one*/
int removeTrailingSpaces(char *string);

/*saves input char array in shell to array so it can be accessed later*/
void saveInputToBuffer(char *input);

/*prints the input buffer in an array for shell to access previous inputs*/
void printInputBuffer(int direction); //value of 1 meaning up and -1 meaning down the array

/*function that turns on or off if addShellIndent() should print the indent or not*/
void turnShellIndentOnOff(int onOrOff);

/*function that turns on or off if input should be taken when typing*/
void turnShellInputOnOff(int onOrOff);

/*sets the functions to a pointer for when the arrow keys are pressed*/
void arrowKeyFunction(char *callOrWrite, char *keys, void (*func)(int));

/*returns the number of args in the char *args*/
int countArgs(char *args);

/*given a string of args and outputs a char *args[numberOfArgs] to access each argument individually
 *also returns the number of arguments*/
int getArgs(char *args, char **output);

/*gets input char *args that is a file path, cd's to that path and return
 * the path of dirs and the file in the input path */
int cdFormatArgs(char *args, char *dirPath, char *filePath);

/*gets the size of the directory names and file names
 * in a path char *args, dirCount, and fileCount should
 * be unused integers, blank*/
void dirFilePathCount(char *args, int *dirCount, int *fileCount);

#endif
