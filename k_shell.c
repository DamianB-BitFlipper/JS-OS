
#include "k_shell.h"

#include "k_stdio.h"
#include "k_programs.h"

int bufferCount = 0, index = 0, localIndex = 1;

void addShellIndent()
{
  int cursor_xValue = getCursorXValue();

  if(cursor_xValue == 0) //if starting cursor is at 0, new line is already there
  {
    k_printf("->");
  }else{ //if starting cursor is not at 0, new line is not present
    k_printf("\n->");
  }

  startingCursorY(); //sets the starting cursor_y position to a veriable

  resetCharCount(); //sets char count back to zero

}

void getTypedText(int charCount, int startingYPos, int cursor_y, char *c)
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

/*char array filled with names of programs*/
char *programsList[]=
{
  /*program names go here*/
  "ascii",
  "echo"
};

void executeInput(char *input, char *arguements)
{

  //~ int spacesRemoved = removeTrailingSpaces(input); //removes trailing space
  
  //~ int items = 2; //number of programs in programsList
  int items = k_elemInCharArray(*programsList); //number of programs in programsList
  int x, hasProgramRun = 0;
  
  for(x = 0; x < items; x++)
  {
    if(k_strcmp(input, programsList[x]) == 0)
    {
      runShellFunction(x, arguements);
      //~ k_printf("\nelement 2: %s\n", programsList[2]);
      hasProgramRun = 1;
    }
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

void formatInput(char *input, char *arguements)
{
  int spacesRemoved = removeTrailingSpaces(input); //removes trailing space
  int length = k_strlen(input), x;

  for(x = 0; x < length; x++)
  {
    if(*input == ' ')
    {
      break;
      //~ k_printf("\nBroken\n");
    }

    input++;
  }

  input = input - x;
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
  char *output;

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
    k_printf("%s", output);
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
