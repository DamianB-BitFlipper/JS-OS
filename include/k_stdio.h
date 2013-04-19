/*
 * k_stdio.h
 * 
 * Copyright 2013 JS <js@duck-squirell>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#ifndef K_STDIO
#define K_STDIO

#include <system.h>

// A few defines to make life a little easier
#define BLACK          0
#define DARK_BLUE      1
#define DARK_GREEN     2
#define DARK_CYAN      3
#define DARK_RED       4
#define DARK_MAGENTA   5
#define BROWN          6
#define LIGHT_GREY     7
#define DARK_GREY      8
#define LIGHT_BLUE     9
#define LIGHT_GREEN    10
#define LIGHT_CYAN     11
#define LIGHT_RED      12
#define LIGHT_MAGENTA  13
#define YELLOW         14
#define WHITE          15

/*a few simple defines*/

/*String Manipulation*/

/*Write a single character out to the screen.*/
void k_putChar(char c);

/*Clear the screen to all black*/
void k_clear();

/*Save screen to char array*/
void k_save();

/*Restore screen from char array*/
void k_restore();

/*Output a null-terminated ASCII string to the monitor*/
void k_printf(char *c, ...);

/*k_printf function where developer can set location as to where to print*/
void k_setprintf(int x, int y, char *text);

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

/*sorts an integer array fpr lowest to highest*/
void k_sortArray(int *array, int count);


/*Other functions*/

/*Function that shifts the cursor depending on the shift amount*/
void shiftCursor(int shiftAmount);

/*functions that have a normal movement for the cursor depending on the value of "movement"
 * -1 is either left if in normalHCursor or down if in normalVCursor
 * 1 is either right if in normalHCursor or up if in normalVCursor*/
void normalHCursor(int movement);
void normalVCursor(int movement);

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

/*function that delets a line on the screen*/
void deleteLine(int yValue); // if yValue < 0, then deletes line of cursor_y

/*function that sets the cursor_y minimum and maximum value*/
void setScreenYMinMax(int yMinimun, int yMaximum); // if arg is negative, yMin/yMax will not change


#endif // K_STDIO
