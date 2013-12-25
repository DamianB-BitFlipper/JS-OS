/*
 * k_shell.h
 * 
 * Copyright 2013 JS-OS <js@duck-squirell>
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

#ifndef SHELL_H
#define SHELL_H

#include <system.h>

#define LS_DIR_COLOR          "%Cbk%clbl"   //black background with light blue text
#define LS_BLKDEV_COLOR       "%Cbk%cg"   //black background with light blue text
#define LS_DEFAULT_COLOR      "%Cbk%cw"    //black background with white text

/*adds a " ->" indent*/
void addShellIndent();

/*prints the greeting message*/
void greeting_message();

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
void printInputBuffer(s32int direction); //value of 1 meaning up and -1 meaning down the array

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
 * in a path char *args
 * dirCount, and fileCount should be unused [constant] integers*/
void dirFilePathCount(char *args, u32int *dirCount, u32int *fileCount);

/*compares two file name, smarter than strcmp since it supports "*"
 * to represent any character */
int compareFileName(char *testName, char *fileName);

/*returns the next option (flag)*/
int getopt(int argIndex, int nArgs, char **args, const char *optString);

/*for multitaksing, gets the args allong with a process thread in a task*/
void get_task_args(char *arguements);

/*checks if the user want the shell command to be multasking enabled,
 * that is checked by looking to see if the user put an '&' at the end of the input*/
u8int is_enable_multitask(char *arguements);

#endif
