// k_stdio.h -- Defines the interface for monitor.h
//              From JamesM's kernel development tutorials.

#ifndef K_STDIO
#define K_STDIO

#include "common.h"

/*String Manipulation*/

/*Write a single character out to the screen.*/
void k_putChar(char c);

/*Clear the screen to all black.*/
void k_clear();

/*Output a null-terminated ASCII string to the monitor.*/
void k_printf(char *c, ...);

/*Convert an integer to a char array ex: 123 to [1,2,3]*/
void k_intToChar(int integer, char *numbersInChar);

/*Convert char array with numer ex:[1,2,3] to ASCII [49, 50, 51]*/
void k_numbersToAsciInChar(char *asciInChar, int integerLength);

/*Compares two stings, return 0 if same and 1 if different*/
int k_strcmp(char *stringOne, char *stringTwo);

/*Gets the length of a string*/
int k_strlen(char *string);

/*A substring function where start and end are inclusive*/
void k_strchop(char *stringIn, char *stringOut, int start, int end);

/*copies one char array to another char array*/
void k_strcpy(char *input, char *output);



/*Other functions*/

/*Function that shifts the cursor depending on the shift amount*/
void shiftCursor(int shiftAmount);

/*Set the charCount back to 0*/
void resetCharCount(void);

/*gets input char pointer and detroys it*/
void destroyCharPointer(char *pointer);

/*gets or sets the starting cursor_y of a shell input*/
void startingCursorY(void);

/*returns the cursor_x value*/
int getCursorXValue(void);

/*finds out how many elements there are in a char array*/
int k_elemInCharArray(char *array);

#endif // K_STDIO
