/*
 * k_shell.c
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

#include <system.h>

u8int shellIndentOn = ON, shellInput = ON;

//The currently running task.
extern volatile task_t *current_task;

extern char *path; //out string for the path from root to the current directory

void turnShellIndentOnOff(int onOrOff)
{
  if(onOrOff == ON) //switch value of shellIndentOn
  {
    shellIndentOn = ON;
  }else if(onOrOff == OFF)
  {
    shellIndentOn = OFF;
  }
}

void turnShellInputOnOff(int onOrOff)
{
  if(onOrOff == 1) //switch value of shellIndentOn
  {
    shellInput = ON;
  }else if(onOrOff == 0)
  {
    shellInput = OFF;
  }
}

void addShellIndent()
{

  if(shellIndentOn == ON) //if shell indent should be printed or not
  {
    int cursor_xValue = getCursorXValue();


    if(cursor_xValue == 0) //if starting cursor is at 0, new line is already there
    {
      /*deletes any chrachter on the line cursor_y, since parameter is negative, function defaults
       * we delete characters since if there is some junk, we remove it to make it look clean */
      deleteLine(-1); 
      
      k_printf("[%cg%s]%cw\n->", path);
    }else{ //if starting cursor is not at 0, new line is not present
      k_printf("\n[%cg%s]%cw\n->", path);
    }

    startingCursorY(); //sets the starting cursor_y position to a veriable

    resetCharCount(); //sets char count back to zero
  }
}

void greeting_message()
{
  //print our nice animal friend
  program_ascii("-little_skverl");
  k_printf("\t-meet our little friend, the skverl (squirrel)\n");
  k_printf("Type \"%cghelp%cw\" for a summury of all of the commands\n");
}

void getTypedText(int charCount, int startingYPos, int cursor_y, char *c)
{
  if(shellInput == ON)
  {
    //~ int spacesRemoved = removeTrailingSpaces(c); //removes trailing space

    //~ char c[charCount];
    u32int x, maxCursor_x = charCount + 2; //the max value the cursor was at, +2 because "->" is 2 characters

    //~ k_printf("\n%d\n", lineCount);

    u16int *video_memory = (u16int*)0xb8000;

    // The background colour is black (0), the foreground is white (15).
    u8int backColour = 0;
    u8int foreColour = 15;

    // The attribute byte is made up of two nibbles - the lower being the
    // foreground colour, and the upper the background colour.
    u8int  attributeByte = (backColour << 4) | (foreColour & 0x0F);
    // The attribute byte is the top 8 bits of the word we have to send to the
    // VGA board.
    u16int attribute = attributeByte << 8;
    u16int *location;

    c += charCount;

    //~ for(x = charCount; x > 0; x--)
    for(x = 0; x < charCount + 1; x++)
    {
      location = video_memory + (startingYPos * 80 + maxCursor_x - x);
      //~ location = video_memory + (cursor_y*80 + charCount - x);
      *c = *location | attribute;

      //~ k_putChar(*c);

      c--;
    }

    //to compensate for the extra c-- on the last loop
    c++;
    *(c + charCount) = 0; //add the \000 at the end

    //~ k_printf("\nstartingY=%d\n", startingYPos);
  }

}

void dirFilePathCount(char *args, u32int *dirCount, u32int *fileCount)
{
  u32int a, length = strlen(args);
  s32int count = -1;

  //a == the position of the very last "/" after finnished with for loop
  for(a = 0; a < length; a++)
  {
    if(*(args + a) == '/')
    {

      count = a;
    }
  }

  if(count != -1) //if count was changed (in char *args, there is atleast one "/")
  {
    u32int intitialDir = currentDir_inode;

    char dirs[count + 2]; //+2 to count because count starts at 0 for the first character and for \000 at the end
    memcpy(dirs, args, count + 1); //+1 to count because count starts at 0 for the first character
    *(dirs + count + 1) = 0; //add \000 at the end

    program_cd(dirs);

    fs_node_t *testDir;
    
    //test if the rest of the args is a dir or file
    testDir = finddir_fs(&root_nodes[currentDir_inode], args + count + 1); 

    if(testDir != 0 && testDir->flags == FS_DIRECTORY) //if testDir is a directory
    {
      //set the dirCount to the whole arg len
      *dirCount = length;

      //if everything is a directory, then there are no file names, set length to 0
      *fileCount = 0;
    }else if(testDir != 0 && testDir->flags == FS_FILE) //if testDir is a file
    {
      //the dirCount is everything before the final '/'
      *dirCount = count + 1;

      //the rest of the arg string is the filecount
      *fileCount = length - (count + 1);
    }else if(!testDir) //there is no testDir location (nothing exists with that name)
    {
      //the dirCount is everything before the final '/'      
      *dirCount = count + 1;

      //assume the rest is a file the user wants to create
      *fileCount = length - (count + 1);
    }

    //return the current directory to the initialDir
    setCurrentDir(&root_nodes[intitialDir]);

    return;
  }else{
    fs_node_t *testDir;

    testDir = finddir_fs(&root_nodes[currentDir_inode], args);

    if(testDir != 0 && testDir->flags == FS_DIRECTORY) //if testDir is a directory
    {
      *dirCount = length;
      //~ *fileCount = -1;
      *fileCount = 0;
    }else if(testDir != 0 && testDir->flags == FS_FILE) //if testDir is a file
    {
      //~ *dirCount = -1;
      *dirCount = 0;
      *fileCount = length;
    }else if(testDir == 0) //there is no testDir location, it must be a file
    {
      *dirCount = 0;
      //~ *fileCount = 0;
      *fileCount = length;
      return;
    }

    return;
  }
}

int compareFileName(char *testName, char *fileName)
{
  u32int length = strlen(testName);
  u32int fileLen = strlen(fileName);

  s32int i, s = 0, charsAfter, loop = FALSE, c;

  for(i = 0; i < length; i++)
  {
    //if the current letter is an "*" or we want to loop
    if(*(testName + i) == '*' || loop == TRUE)
    {
      //if we did not want to loop, increase i since the current char is a "*", and we want to know the next char
      if(loop == FALSE)
      {
        i++;
      }

      //reset the value of loop to default
      loop = FALSE;

      //increment s to offset to the character after the "*"
      while(*(testName + i) != *(fileName + s) && s <= fileLen)
      {
        s++;
      }

      if(s > fileLen)
      {
        return 1;
      }

      /*this segment checks how many charaters there are after the current
       * "*" to the next "*" or end of name */
      charsAfter = 0;
      while(*(testName + i + charsAfter) != '*' && charsAfter + i < length)
      {
        charsAfter++;
      }

      /*this checks if the contents of testName from i (offset) to the next "*" or end of name
       * correspond the same with fileName + s (offset) */
      for(c = 0; c <= charsAfter; c++)
      {
        /*if the contents of the two names differ and the current testName + offset is not an "*" */
        if(*(testName + i + c) != *(fileName + s + c) && *(testName + i + c) != '*')
        {
          /*we want to loop and check fileName's later contents if they correspond */
          loop = TRUE;

          /*if we reached the end of fileName, exit */
          if(*(fileName + s + c) == 0)
          {
            return 1;
          }

          /*if we reached the end of our testName, then decrement i, which
           * will get incremented by our for loop, ie: balance i out to
           * keep it constant */
          if(*(testName + i + c) == 0)
          {
            i--;
          }

          break;
        }
      }

    }

    /*if the fileNames differ and we have not told the loop to continue, exit */
    if(*(testName + i) != *(fileName + s) && loop == FALSE)
    {
      return 1;
    }else{
      s++;
    }

  }

  //we survived the for loop, return true (only survives if the fileNames are "same")
  return 0;

}

int getopt(int argIndex, int nArgs, char **args, const char *optString)
{
  /*if our argIndex excceds the number of Args, remember,
   * argIndex starts from 0, so if they are equal, that means
   * we already parced all of the args */
  if(argIndex >= nArgs)
  {
    return -1;
  }
    
  int optLen = strlen(optString);

  if(*(args[argIndex]) != '-') //if the arg start off without the propper notation
  {
    //exit without complaint
    return 0;
  }

  //static int increase = 1;
  char singleArg = *(args[argIndex] + 1); //+1 to skip over the "-" in the front of args

  int i;
  for(i = 0; i < optLen; i++)
  {
    //if the arg we are looking for is in our optString
    if(singleArg == *(optString + i))
    {
      return singleArg;

    }
  }

  //if we did not exit in the above for loop, return an error, there was no proper arg
  //error!
  return -1;
}

int cdFormatArgs(char *args, char *dirPath, char *filePath)
{
  u32int i, length = strlen(args);
  s32int count = -1;

  for(i = 0; i < length; i++)
  {
    /* using the following, after it is done executing, the integer
     * count will be equeal to the number of character before and
     * including the last "/" in the destination string of input */
    if(*(args + i) == '/') //for every "/" in the dest of input, increment count to i
    {
      count = i;
    }
  }

  //if count is still -1, then all of args is the filePath and none of it is dirPath
  if(count == -1)
  {
    //set count to be the length of args (i) -1 becuase i starts from 0
    count = i - 1;

    fs_node_t *isDir;
    isDir = finddir_fs(&root_nodes[currentDir_inode], args);

    if(!isDir) //no such entry exists
    {
      //error!
      return 1;
    }else if(isDir != 0 && isDir->flags == FS_FILE) //the input is a file, cannot be cd'ed to
    {
      *(dirPath) = 0; //set dirPath to null
      strcpy(filePath, isDir->name);
      *(filePath + strlen(isDir->name)) = 0; //add \000 to the end

      return 0;
    }else if(isDir != 0 && isDir->flags == FS_DIRECTORY) //the input is just a directory
    {
      u32int w = program_cd(args);
      
      //there is no filePath, set it to 0
      *(filePath) = 0;
      return w;
    }
  }

  //~ u32int intitialDir = currentDir_inode;

  char *dirs;

  //+2 to count because count starts at 0 for the first character and for \000 at the end
  dirs = (char*)kmalloc(count + 2);
  memcpy(dirs, args, count + 1); //+1 to count because count starts at 0 for the first character
  *(dirs + count + 1) = 0; //add \000 at the end

  u32int work = program_cd(dirs);

  fs_node_t *isDir;
  isDir = finddir_fs(&root_nodes[currentDir_inode], args + count + 1); //test if the rest of the path is a directory

  if(isDir != 0 && isDir->flags == FS_DIRECTORY) //the rest is a directory
  {
    //cd to the directory left out without a ending "/"
    work = program_cd(args + count + 1);

    count = length - 1;

  }

  //copy directory content in args to dirPath
  memcpy(dirPath, args, count + 1);
  *(dirPath + count + 1) = 0; //add \000 to the end

  //copy file content in args to filePath
  memcpy(filePath, args + count + 1, length - count - 1);
  *(filePath + length - count - 1) = 0; //add the \000 to the end

  kfree(dirs);

  return work;

}

/*typedef array filled with names of programs*/
typedef struct 
{
  //set a default name size of 16 chars
  char name[16];
  u32int priority;
  u32int burst_time;
} programs_list_t;

#define PROGRAM_LIST_NUMBER    19

programs_list_t programsList[PROGRAM_LIST_NUMBER]=
{
  /*program names go here*/
  "ascii", PRIO_LOW, PROC_VERY_SHORT,
  "echo", PRIO_LOW, PROC_SHORT,
  "tinytext", PRIO_LOW, PROC_MED,
  "pong", PRIO_MED, PROC_MED,
  "song", PRIO_LOW, PROC_LONG,
  "viewer", PRIO_MED, PROC_LONG,
  "start", PRIO_LOW, PROC_VERY_LONG,
  "ls", PRIO_LOW, PROC_VERY_SHORT,
  "cd", PRIO_LOW, PROC_VERY_SHORT,
  "now", PRIO_LOW, PROC_VERY_SHORT,
  "mkdir", PRIO_LOW, PROC_VERY_SHORT,
  "cp", PRIO_LOW, PROC_VERY_SHORT,
  "cat", PRIO_LOW, PROC_VERY_SHORT,
  "rm", PRIO_LOW, PROC_VERY_SHORT,
  "pwd", PRIO_LOW, PROC_VERY_SHORT,
  "help", PRIO_LOW, PROC_VERY_SHORT,
  "mv", PRIO_LOW, PROC_VERY_SHORT,
  "find", PRIO_LOW, PROC_VERY_SHORT,
  "about", PRIO_LOW, PROC_VERY_SHORT
};

void executeInput(char *input, char *arguements)
{
  if(shellInput == ON)
  {
    //~ int spacesRemoved = removeTrailingSpaces(input); //removes trailing space

    //~ int items = 2; //number of programs in programsList
    //~ int items = k_elemInCharArray(*programsList); //number of programs in programsList
    //~ k_printf("\n%d", items);

    int x;

    for(x = 0; x < PROGRAM_LIST_NUMBER; x++)
    {
      if(k_strcmp(input, programsList[x].name) == 0)
      {

        runShellFunction(x, arguements, programsList[x].priority, programsList[x].burst_time);

        //exit
        return;
      }

    }

    //if we did not exit above, i.e., there was no function with the input name
    int length = k_strlen(input);

    if(length != 0)
    {
      /*error command not found*/
      k_printf("%s: command not found. Type 'help' for a list\n", input);
    }
  }

}

void formatInput(char *input, char *arguements)
{
  int spacesRemoved = removeTrailingSpaces(input); //removes trailing space
  int length = k_strlen(input), x;

  for(x = 0; x < length; x++) //finds the x value of the first space in the input string
  {
    if(*input == ' ')
    {
      break;
      //~ k_printf("\nBroken\n");
    }

    input++;
  }

  input = input - x; //goes back to the begining of the input string
  //~ k_printf("\nx is=%d\n", x);

  if(x != length) //if x is equal to the length, there is no need to substring
  {
    k_strchop(input, arguements, x + 1, length); //gets the arguement string
    k_strchop(input, input, 0, x - 1); //gets the command string
    //~ k_printf("\n\t%s\n", input);
    //~ k_printf("\n\t%s\n", arguements);
  }
}

int removeTrailingSpaces(char *string)
{
  int length = k_strlen(string);
  int spacesRemoved = 0;

  string = string + length - 1;
  //~ string = string + length;

  while(*string == ' ')
  {
    *string = 0;

    string--;
    spacesRemoved++;
  }

  return spacesRemoved;
}

//TODO make get_task_args work
void get_task_args(char *arguements)
{
  asm volatile("cli");
  if(current_task->thread_flags)
  {
    //~ u32int length = strlen(current_task->thread_flags);
    //~ arguements = (char*)kmalloc(length + 1); //+1 being the \000 at the end
//~ 
    //~ strcpy(arguements, current_task->thread_flags);
    arguements = (char*)current_task->thread_flags;
  }
  asm volatile("sti");
}

///*char array filled with names of programs*/

u32int bufferCount = 0, index = 0, localIndex = 1;

char index0[250]; //allocate enough space for 10 lines with 25 chars each
char index1[250];
char index2[250];
char index3[250];
char index4[250];
char space[1] = "\000";

void setOrGetCommandBuffer(char *readOrWrite, int index, char *inputOutputChar)
{
  if(k_strcmp("write", readOrWrite) == 0) //if user wants to write to buffer
  {
    //~ removeTrailingSpaces(inputOutputChar);
    //~ k_printf("\nout");

    switch(index)
    {
      case 0:
        k_strcpy(inputOutputChar, index0);
        break;
        //~ k_printf("\n1");

      case 1:
        k_strcpy(inputOutputChar, index1);
        //~ *index1 = 'e';
        break;
        //~ k_printf("\n2");
      case 2:
        k_strcpy(inputOutputChar, index2);
        break;
      case 3:
        k_strcpy(inputOutputChar, index3);
        break;
      case 4:
        k_strcpy(inputOutputChar, index4);
        break;
      case 5:
        k_strcpy(inputOutputChar, space);
        break;
    }
      //~ k_printf("\n%s\tindex:%d", index0, index);
  }else if(k_strcmp("read", readOrWrite) == 0)
  {
    switch (index)
    {
      case 0:
        k_strcpy(index0, inputOutputChar);
        break;
      case 1:
        k_strcpy(index1, inputOutputChar);

        //if(index < 1)
        //{
          //index = 1;
        //}
        break;
      case 2:
        k_strcpy(index2, inputOutputChar);
        //if(index < 2)
        //{
          //index = 2;
        //}
        break;
      case 3:
        k_strcpy(index3, inputOutputChar);
        //if(index < 3)
        //{
          //index = 3;
        //}
        break;
      case 4:
        k_strcpy(index4, inputOutputChar);
        //index = 4;
        //if(index < 4)
        //{
          //index = 4;
        //}
        break;
      case 5:
        k_strcpy(space, inputOutputChar);
        //index = 5;
        //if(index < 5)
        //{
          //index = 5;
        //}
        break;
    }

  }
}

void saveInputToBuffer(char *input)
{
  int length = k_strlen(input), x;

  //char *copy;
  //k_strcpy(input, copy);

  setOrGetCommandBuffer("write", bufferCount, input);
  //~ setOrGetCommandBuffer("write", 1, input);

  //for(x = 0; x < length; x++)
  //{
    //inputCommandsBuffer[bufferCount][x] = *copy;

    //copy++;
  //}
  //~ k_printf("\n%d", bufferCount);
  //~ k_printf("\n%s", input);

  //~ inputCommandsBuffer[bufferCount] = copy;
  //~ k_strcpy(copy, inputCommandsBuffer[bufferCount]);

  //~ k_printf("\nthis: %s, as to in my array: %s", input, inputCommandsBuffer[bufferCount]);

  bufferCount = (bufferCount + 1) % 5;
  localIndex = bufferCount;
}

void printInputBuffer(s32int direction) //value of 1 meaning up and -1 meaning down the array
{
  s32int oldCursor_x = 1, newCursor_x = 2;

  while(oldCursor_x != newCursor_x) //deletes everything after shell indent
  {
    oldCursor_x = getCursorXValue();
    k_printf("\b");

    newCursor_x = getCursorXValue();
  }


  localIndex -= direction;

  if(localIndex < 0)
  {
    localIndex = 0;
  }else if(localIndex > 5 && localIndex != 10) //fifth index is the blank one
  {
    localIndex = 5;
  }else if(localIndex == 10 && direction > 0)
  {
    localIndex = index - 1;
  }

  char *output; //allocates the max size of the output to be printed, 10 rows of 25 chars each
  output = (char*)kmalloc(250);
  //clear the output char with zeros to remove junk
  memset(output, 0, 250);

  setOrGetCommandBuffer("read", localIndex, output);

  //~ k_printf(inputCommandsBuffer[bufferCount - direction]);
  //~ if(k_strcmp(output, "") == 1 || localIndex == 5)
  if(localIndex <= 5)
  {
    k_printf(output);
    //~ if(k_strcmp(output, " ") == 0)
    //~ {
      //~ localIndex--;
    //~ }
    //~ k_printf("IN printed");

  }else if(localIndex >= 5 && direction < 0)
  {
    localIndex = 10;
    //~ k_printf("printed");

  }

  //~ k_printf("printed");
  kfree(output);

}

void arrowKeyFunction(char *callOrWrite, char *keys, void (*func)() )
{
static void (*leftRight)(int);
static void (*upDown)(int);

  if((k_strcmp(keys, "left") == 0 || k_strcmp(keys, "right") == 0 ) && k_strcmp(callOrWrite, "write") == 0) //set input function for left/right keys
  {
    leftRight = func;

  }else if((k_strcmp(keys, "up") == 0 || k_strcmp(keys, "down") == 0 ) && k_strcmp(callOrWrite, "write") == 0) //set input function for up/down keys
  {
    upDown = func;

  }

  if(k_strcmp(keys, "left") == 0 && k_strcmp(callOrWrite, "call") == 0) //calls left key press
  {
    (*leftRight)(-1);
  }else if(k_strcmp(keys, "right") == 0 && k_strcmp(callOrWrite, "call") == 0) //calls right key press
  {
    (*leftRight)(1);
  }else if(k_strcmp(keys, "up") == 0 && k_strcmp(callOrWrite, "call") == 0) //calls up key press
  {
    (*upDown)(1);
  }else if(k_strcmp(keys, "down") == 0 && k_strcmp(callOrWrite, "call") == 0) //calls down key press
  {
    (*upDown)(-1);
  }

}

int countArgs(char *args)
{
  //removeTrailingSpaces(args);

  int length = strlen(args);
  int i;
  int nArgs = 0;

  /*counts the number of args*/
  for(i = 0; i < length + 1; i++)
  {
    /* If we have a space or we are at the end, to get the last arg,
     * there is no space at the end */
    if(*(args + i) == ' ' || (i == length && *(args + i - 1) != 0))
    {
      nArgs++;
    //}else if(*(args + i) == '-' && *(args + i + 1) != ' '
              //&& *(args + i + 1) != 0 && *(args + i + 2) != ' '
              //&& *(args + i + 2) != 0)
    }else if(*(args + i) == '-')
    {
      int argLen = 1;

      /*if the next character is not a space or the end,
       * increase the argLen and the nArgs by one */
      while(*(args + i + argLen) != 0 && *(args + i + argLen) != ' ')
      {
        nArgs++;
        argLen++;
      }

      /*if argLen is unchanged, do not change nArgs, else
       * decrement nArgs by one to acomodate for the "-" not being an arg by itself */
      nArgs = argLen == 1 ? nArgs : nArgs - 1;

      // increment i with the length of the current arg -1, because we add 1 in the for loop to come
      i += (argLen - 1);
    }
  }

  return nArgs;
}

int getArgs(char *args, char **output)
{
  //removeTrailingSpaces(args);

  int length = strlen(args);
  int i;
  //~ int nArgs = 1;
//~
  //~ /*counts the number of args*/
  //~ for(i = 0; i < length; i++)
  //~ {
    //~ if(*(args + i) == ' ')
    //~ {
      //~ nArgs++;
    //~ }
  //~ }

  //~ char *arguments[nArgs]; //creates a 2d array of pointers for chars

  int oldI = 0, curArg = 0;
  for(i = 0; i < length + 1; i++)
  {
    //if we have a space or we are at the end, to get the last arg, there is no space at the end
    if(*(args + i) == ' ' || i == length) 
    {
      //if the begining character of this argument is not a "-", then copy the whole thing
      if(*(args + oldI) != '-')
      { 
        output[curArg] = (char*)kmalloc(i - oldI + 1); //allocates space for the arg names, +1 being \000
        //~ output[curArg] = (char*)(output + oldI);

        memcpy(output[curArg], args + oldI, i - oldI);
        *(output[curArg] + i - oldI) = 0; //adds \000 to the end

        curArg++;     //increments the curArg
        oldI = i + 1; //oldI is set to i and then +1 so we jump over that space
      }else{
        //increment oldI to skip the "-"
        oldI++;
        
        /*i - oldI is the length of the current arg
         * e.g., "-mi" should loop twice
         * to make two args "-m" and "-i" */        
        int b, argLen = i - oldI;
        for(b = 0; b < argLen; b++)
        {
          //a size of 3, because 2 for "-a" and 1 for the \000 at the end
          output[curArg] = (char*)kmalloc(3);

          //copy just the letter arg to the second position
          memcpy(output[curArg] + 1, args + oldI, 1);
          *(output[curArg]) = '-';   //adds "-" to the start of the flag arg
          *(output[curArg] + 2) = 0; //adds \000 to the end

          curArg++;  //increments the curArg
          oldI++;    //oldI incremented to go to next letter       
        }

        //if the arg is just "-" and nothing more
        if(argLen == 0)
        {
          //a size of 2, 1 for "-" and another for the \000
          output[curArg] = (char*)kmalloc(2);

          //copy arg to output
          memcpy(output[curArg], args + oldI - 1, 1);
          *(output[curArg] + 1) = 0; //adds \000 to the end

          curArg++;  //increments the curArg           
        }

        oldI++; //oldI is incremented to skip the space         
        
      }
      
    }

  }

  return curArg;
}
