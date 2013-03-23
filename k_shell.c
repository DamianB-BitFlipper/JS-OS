
#include "k_shell.h"

#include "k_stdio.h"
#include "k_programs.h"

#include "fs.h"
#include "initrd.h"

int bufferCount = 0, index = 0, localIndex = 1;
int shellIndentOn = ON, shellInput = ON;

extern char *path; //out string for the path from root to the current directory

void turnShellIndentOnOff(int onOrOff)
{
  if(onOrOff == 1) //switch value of shellIndentOn
  {
    shellIndentOn = ON;
  }else if(onOrOff == 0)
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

    deleteLine(-1); //deletes any chrachter on the line cursor_y, since parameter is negative, function defaults

    if(cursor_xValue == 0) //if starting cursor is at 0, new line is already there
    {
      k_printf("[%cg%s]%cw\n->", path);
    }else{ //if starting cursor is not at 0, new line is not present
      k_printf("\n[%cg%s]%cw\n->", path);
    }

    startingCursorY(); //sets the starting cursor_y position to a veriable

    resetCharCount(); //sets char count back to zero
  }
}

void getTypedText(int charCount, int startingYPos, int cursor_y, char *c)
{
  if(shellInput == ON)
  {
    //~ int spacesRemoved = removeTrailingSpaces(c); //removes trailing space

    //~ char c[charCount];
    int x, maxCursor_x = charCount + 2; //the max value the cursor was at, +2 because "->" is 2 characters

    //~ k_printf("\n%d\n", lineCount);
    
    u16int *video_memory = (u16int *)0xB8000;
    
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

    //~ k_printf("\n");

    c = c + charCount;

    //~ for(x = charCount; x > 0; x--)
    for(x = 0; x < charCount + 1; x++)
    {
      location = video_memory + (startingYPos*80 + maxCursor_x - x);
      //~ location = video_memory + (cursor_y*80 + charCount - x);
      *c = *location | attribute;

      //~ k_putChar(*c);

      c--;
    }

    //~ k_printf("\nstartingY=%d\n", startingYPos);
  }
  
}

void dirFilePathCount(char *args, int *dirCount, int *fileCount)
{
  int a, length = strlen(args), count = -1;

  //goes from the back, when it breaks, a == the position of the very last "/"
  for(a = 0; a < length; a++)
  {
    if(*(args + a) == '/')
    {

      count = a;
    }
  }

  if(count != -1) //if count remained unchanged (in char *args, there is only text, no "/")
  {
    *dirCount = count + 1;
    *fileCount = length - (count + 1);

    return;
  }else{
    fs_node_t *testDir;

    testDir = finddir_fs(&root_nodes[currentDir_inode], args);

    if(testDir != 0 && testDir->flags == FS_DIRECTORY) //if testDir is a directory
    {
      *dirCount = length;
      *fileCount = -1;
    }else{
      *dirCount = -1;
      *fileCount = length;
    }

    return;
  }
}

int cdFormatArgs(char *args, char *dirPath, char *filePath)
{
  int i = 0, length = strlen(args), count = -1;

  for(i; i < length; i++)
  {
    /* using the following, after it is done executing, the integer
     * count will be equeal to the number of character before and
     * including the last "/" in the destination string of input */
    if(*(args + i) == '/') //for every "/" in the dest of input, increment count to i
    {
      count = i;
    }
  }

  if(count == -1)
  {
    count = i - 1;

    fs_node_t *isDir;
    isDir = finddir_fs(&root_nodes[currentDir_inode], args);

    if(isDir == 0) //no such entry exists
    {
      //failure!
      return 1;
    }else if(isDir != 0 && isDir->flags == FS_FILE) //the input is a file, cannot be cd'ed to
    {
      *(dirPath) = 0; //set dirPath to null
      strcpy(filePath, isDir->name);
      *(filePath + strlen(isDir->name)) = 0; //add \000 to the end

      return 0;
    }else if(isDir != 0 && isDir->flags == FS_DIRECTORY) //the input is just a directory
    {
      int w = program_cd(args);
      return w;
    }
  }

  //~ char *dir, *file;
  //~ 
  //~ dir = (char*)kmalloc(count + 2); //the extra +1 is for the \000 and the other one is because i, and thus count start from 0 on the first element
  memcpy(dirPath, args, count + 1);
  *(dirPath + count + 1) = 0; //add \000 to the end

  //~ file = (char*)kmalloc(length - count);
  memcpy(filePath, args + count + 1, length - count - 1);
  *(filePath + length - count - 1) = 0; //add the \000 to the end

  int work = program_cd(dirPath);

  return work;

}

/*char array filled with names of programs*/
#define PROGRAM_LIST_NUMBER    17

char *programsList[PROGRAM_LIST_NUMBER]=
{
  /*program names go here*/
  "ascii",
  "echo",
  "tinytext",
  "pong",
  "song",
  "viewer",
  "start",
  "ls",
  "cd",
  "now",
  "mkdir",
  "cp",
  "cat",
  "rm",
  "pwd",
  "help",
  "mv"
};

void executeInput(char *input, char *arguements)
{
  if(shellInput == ON)
  {
    //~ int spacesRemoved = removeTrailingSpaces(input); //removes trailing space
    
    //~ int items = 2; //number of programs in programsList
    //~ int items = k_elemInCharArray(*programsList); //number of programs in programsList
    //~ k_printf("\n%d", items);
    
    int x, hasProgramRun = 0;
    
    for(x = 0; x < PROGRAM_LIST_NUMBER; x++)
    {
      if(k_strcmp(input, programsList[x]) == 0)
      {
        runShellFunction(x, arguements);
        
        //~ k_printf("\nelement 2: %d\n", x);

        hasProgramRun = 1;
      }

      //~ k_printf("\n%s\t%s", input, programsList[x]);
    }

    if(hasProgramRun == 0) //if function did not enter runShell if statement
    {
      int length = k_strlen(input);

      if(length != 0)
      {
        /*error command not found*/
        k_printf("%s: command not found\n", input);
      }
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

///*char array filled with names of programs*/
//char *inputCommandsBuffer[5][]=
//{
  ///*empty commands here that will be filled in*/
  //"a",
  //"a",
  //"a",
  //"a",
  //"a"
//};

  char index0[250]; //allocate enough space for 10 lines with 25 chars each
  char index1[250];
  char index2[250];
  char index3[250];
  char index4[250];
  char space[1] = "";
  
void setOrGetCommandBuffer(char *readOrWrite, int index, char *inputOutputChar)
{
  if(k_strcmp("write", readOrWrite) == 0) //if user wants to write to buffer
  {
    removeTrailingSpaces(inputOutputChar);
    //~ k_printf("\nout");
    
    switch (index)
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

        if(index < 1)
        {
          index = 1;
        }
        break;
      case 2:
        k_strcpy(index2, inputOutputChar);
        if(index < 2)
        {
          index = 2;
        }
        break;
      case 3:
        k_strcpy(index3, inputOutputChar);
        if(index < 3)
        {
          index = 3;
        }
        break;
      case 4:
        k_strcpy(index4, inputOutputChar);
        index = 4;
        if(index < 4)
        {
          index = 4;
        }
        break;
      case 5:
        k_strcpy(space, inputOutputChar);
        index = 5;
        if(index < 5)
        {
          index = 5;
        }
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

void printInputBuffer(int direction) //value of 1 meaning up and -1 meaning down the array
{
  int oldCursor_x = 1, newCursor_x = 2;
  char output[250]; //allocates the max size of the output to be printed, 10 rows of 25 chars each

  while(oldCursor_x != newCursor_x) //deletes everything after shell indent
  {
    oldCursor_x = getCursorXValue();
    k_printf("\b");

    newCursor_x = getCursorXValue();
  }

  
  localIndex = localIndex - direction;

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
  removeTrailingSpaces(args);

  int length = strlen(args);
  int i;
  int nArgs = 0;

  /*counts the number of args*/
  for(i = 0; i < length + 1; i++) 
  {
    /* If we have a space or we are at the end, to get the last arg,
     * there is no space at the end */
    if(*(args + i) == ' ' || i == length)
    {
      nArgs++;
    }
  }

  return nArgs;
}

int getArgs(char *args, char **output)
{
  removeTrailingSpaces(args);

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
    if(*(args + i) == ' ' || i == length) //if we have a space or we are at the end, to get the last arg, there is no space at the end
    {
      output[curArg] = (char*)kmalloc(i - oldI + 1); //allocates space for the arg names, +1 being \000
      //~ output[curArg] = (char*)(output + oldI);

      memcpy(output[curArg], args + oldI, i - oldI);
      *(output[curArg] + i - oldI) = 0; //adds \000 to the end
      
      curArg++; //increments the curArg
      oldI = i + 1; //oldI is set to i and then +1 so we jump over that space
    }
  }

  return curArg;
}
