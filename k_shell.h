#ifndef SHELL_H
#define SHELL_H

#include "common.h"

/*adds a " ->" indent*/
void addShellIndent();

/*gets string input from previous line*/
void getTypedText(int charCount, int startingYPos, int cursor_y, char *c);

/*reads inputs for shell*/
void executeInput(char *input, char *arguements);

/*gets the input and spits it into command and arguments*/
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


#endif
