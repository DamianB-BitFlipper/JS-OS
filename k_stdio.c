// k_stdio.c -- Defines functions for writing to the monitor.
//             heavily based on Bran's kernel development tutorials,
//             but rewritten for JamesM's kernel tutorials.

#include <stdint.h>
#include <stdarg.h>

#include "k_stdio.h"
#include "k_math.h"
#include "k_shell.h"


// The VGA framebuffer starts at 0xB8000.
u16int *video_memory = (u16int *)0xB8000;
//~ unsigned char *vram = (unsigned char *)0xA0000;
//~ unsigned char *vram = (unsigned char *)0xA0000000L;
//~ u16int *vram = (u16int *)0xA0000;
//~ char *vram = (unsigned char *)0xA0000;//~ char *video_memory = (char *) 0xb8000;
//~ typedef unsigned char byte;
//~ byte far *vram = (byte far*)0xA0000000L;
//~ 
//~ char *vram = (unsigned char *)0xA0000000L;
//~ u16int *vram = (u16int *)0xA0000;

// Stores the cursor position.
u8int cursor_x = 0;
u8int cursor_y = 0;

// The default background colour is black (0), the foreground is white (15).
u8int backColour = 0;
u8int foreColour = 15;

int charCount = 0, charPosition = 0, lineCount = 0, startingYPos = 0;

int yMin = 0, yMax = 25;
char screen[8000]; //char array to hold screen characters when k_save() is called

// Updates the hardware cursor.
static void move_cursor()
{
    // The screen is 80 characters wide...
    u16int cursorLocation = cursor_y * 80 + cursor_x;
    outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
    outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
    outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
    outb(0x3D5, cursorLocation);      // Send the low cursor byte.
}

// Scrolls the text on the screen up by one line.
static void scroll()
{

    // Get a space character with the default colour attributes.
    u8int attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
    u16int blank = 0x20 /* space */ | (attributeByte << 8);

    // Row 25 is the end, this means we need to scroll up
    if(cursor_y >= yMax)
    {
        // Move the current text chunk that makes up the screen
        // back in the buffer by a line
        int i;
        for (i = yMin*80; i < (yMax - 1)*80 + 1; i++)
        {
            video_memory[i] = video_memory[i+80];
        }

        // The last line should now be blank. Do this by writing
        // 80 spaces to it.
        for (i = (yMax - 1)*80; i < yMax*80 + 1; i++)
        {
            video_memory[i] = blank;
        }
        // The cursor should now be on the last line.
        cursor_y = (yMax - 1);
    }
}

// Writes a single character out to the screen.
void k_putChar(char c)
{
  scroll();

  // The attribute byte is made up of two nibbles - the lower being the
  // foreground colour, and the upper the background colour.
  u8int  attributeByte = (backColour << 4) | (foreColour & 0x0F);
  //~ u8int  attributeByte = (foreColour << 4) | (backColour & 0x0F);
  // The attribute byte is the top 8 bits of the word we have to send to the
  // VGA board.
  u16int attribute = attributeByte << 8;
  u16int *location;

  // Handle a backspace, by moving the cursor back one space
  //~ if (c == '\b' && cursor_x)
  if (c == '\b')
  {
    int x;

    if(charPosition > 0)
    {

      if(cursor_x == 0) //move one back and x_cursor to end
      {
        cursor_y--;
        cursor_x = 80;
      }
      cursor_x--;

      location = video_memory + (cursor_y*80 + cursor_x);
      //~ *location = 0 | attribute;
      for(x = 0; x < (charCount - (cursor_x - 3)); x++)
      {
        *(location + x) = *(location + x + 1) | attribute;
      }

      *(location + x + 1) = 0 | attribute;

      charCount--;
      charPosition--;
    }
  }

  // Handle a tab by increasing the cursor's X, but only to a point
  // where it is divisible by 8.
  else if (c == '\t')
  {
    cursor_x = (cursor_x+8) & ~(8-1);
    charCount = charCount + 5;
    charPosition = charPosition + 5;

  }else if (c == '\r')  // Handle carriage return "Enter Key"
  {
    //~ char *input, *split;
    char input[250], split[250]; //allocates maximum size for input, being 10 lines of 25 chars each

    //~ getTypedText(charCount, cursor_x, cursor_y, input);
    getTypedText(charCount, startingYPos, cursor_y, input); //gets the stuff typed when enter is pressed
    cursor_y++;
    cursor_x = 0;

    saveInputToBuffer(input); //saves "input" to array so it can be accessed with up arrow later

    //~ k_printf("\ncontinued\n");

    //~ k_putChar(input[0]);
    //~ k_printf(input);
    //~ k_printf("\n");

    int run;
    for(run = charCount; run < 250; run++) //this loop replaces all of the junk after the inputed text with \000
    {
      input[run] = 0;
    }

    formatInput(input, split); //assigns command to "input" and args to "split"

    //~ removeTrailingSpaces(split);
    
    executeInput(input, split); //executes program "input" with args "split"

    //~ k_printf("\n\n%s%s%s\n", split, split, split);

    charCount = 0; //resets the char counter and charUnderlinePosition
    charPosition = 0;

    location = video_memory + (cursor_y * 80 + cursor_x);
    *location = ' ' | attribute;

    addShellIndent(); //adds the shell "->" indent

    destroyCharPointer(input); //destroys char array to no interfere with others
    destroyCharPointer(split); //destroys char array to no interfere with others

    lineCount = 0;
  }

  // Handle newline by moving cursor back to left and increasing the row
  else if (c == '\n')
  {
    charCount = charCount + (80 - cursor_x);
    charPosition = charPosition + (80 - cursor_x);
    cursor_x = 0;
    cursor_y++;
  }
  // Handle any other printable character.
  else if(c >= ' ')
  {
    location = video_memory + (cursor_y * 80 + cursor_x);
    *location = c | attribute;
    cursor_x++;
    if(charPosition == charCount)
    {
      charCount++;
    }
    charPosition++;
  }

  // Check if we need to insert a new line because we have reached the end
  // of the screen.
  if (cursor_x >= 80)
  {
    lineCount++;
    cursor_x = 0;
    cursor_y++;
  }

  // Scroll the screen if needed.
  scroll();
  // Move the hardware cursor.
  move_cursor();

}

// Clears the screen, by copying lots of spaces to the framebuffer.
void k_clear()
{
    // Make an attribute byte for the default colours
    u8int attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
    //~ u16int blank = 0x20 /* space */ | (attributeByte << 8);
    //~ u16int blank = 0x20 /* space */ | (attributeByte << 8);
    u16int blank = 0x00 /* space */ | (attributeByte << 8);

    int i;
    for (i = 0; i < 80*25; i++)
    {
        video_memory[i] = blank;
        //~ video_memory[i] = 0x20;
    }

    // Move the hardware cursor back to the start.
    cursor_x = 0;
    cursor_y = 0;
    move_cursor();
}

// Outputs a null-terminated ASCII string to the monitor.
void k_printf(char *c, ...)
{

  va_list arguments;

  int i = 0, numberOfArgs = 0, stringLength = k_strlen(c);
  int integerArg;

  while(c[i]) //checks number of arguments that should be in file
  {
    if(c[i] == '%')
    {
      numberOfArgs++;
    }
    i++;
  }

  i = 0; //set i to zero for next while loop

  va_start(arguments, *c);

  while (c[i])
  {

    if(c[i] == '%')
    {
      if(c[i+1] == 'd')
      {
        integerArg = va_arg(arguments, int);

        int numberLength = math_intLength(integerArg);
        int x;

        char integer[numberLength];
        //~ char *stringCharacter;
        //~ k_printf("\n%d\n", numberLength);

        //~ char integer[10];

        k_intToChar(integerArg, integer); //sets integer as char

        k_numbersToAsciInChar(integer, numberLength); //sets char to ASCII for printing

        for(x = 0; x < numberLength; x++)
        {
          //~ *stringCharacter = integer[x];

          k_putChar(integer[x]);

          //~ k_printf("5");
          //~ integer++;
        }

        i = i + 2;
      }else if(c[i+1] == 's') //user wants to print a string
      {
        char *charArrayArg;
        charArrayArg = va_arg(arguments, char*);

        k_printf(charArrayArg);
        i = i + 2;

      }else if(c[i+1] == 'c') //user wants to print colored text
      {
        if(c[i+2] == 'w') //white text
        {
          foreColour = 15;
        }else if(c[i+2] == 'b' && c[i+3] == 'k') //black text
        {
          foreColour = 0;
          i++;
        }else if(c[i+2] == 'b' && c[i+3] == 'l') //blue text
        {
          foreColour = 1;
          i++;
        }else if(c[i+2] == 'g') //green text
        {
          foreColour = 2;
        }else if(c[i+2] == 'r') //red text
        {
          foreColour = 4;
        }

        i = i + 3;

      }else if(c[i+1] == 'C') //user wants to print colored background
      {
        if(c[i+2] == 'w') //white background
        {
          backColour = 15;
        }else if(c[i+2] == 'b' && c[i+3] == 'k') //black background
        {
          backColour = 0;
          i++;
        }else if(c[i+2] == 'b' && c[i+3] == 'l') //blue background
        {
          backColour = 1;
          i++;
        }else if(c[i+2] == 'g') //green background
        {
          backColour = 2;
        }else if(c[i+2] == 'r') //red background
        {
          backColour = 4;
        }

        i = i + 3;
        //~ k_printf("\n%d\n", i);

      }else if(c[i+1] == 'h') //user wants to print a hex number
      {

        u32int hexArg = va_arg(arguments, u32int);

        s32int tmp;

        k_printf("0x");

        char noZeroes = 1;

        int hexCount;
        for (hexCount = 28; hexCount > 0; hexCount -= 4)
        {
          tmp = (hexArg >> hexCount) & 0xF;
          if (tmp == 0 && noZeroes != 0)
          {
            continue;
          }
      
          if (tmp >= 0xA)
          {
            noZeroes = 0;
            k_putChar(tmp-0xA+'a');
          }
          else
          {
            noZeroes = 0;
            k_putChar(tmp+'0');
            
          }
        }
      
        tmp = hexArg & 0xF;
        if (tmp >= 0xA)
        {
          k_putChar(tmp-0xA+'a');
          
        }
        else
        {
          k_putChar(tmp+'0');
          
        }

        i = i + 2;
      }
    }

    if(i < stringLength + 1 && c[i] != '%')
    {
      if(c[i] == 0) //if i is at the terminating 0 '\000' of a string, break from loop
      {
        break;
      }else{
        k_putChar(c[i]);
        i++;
      }
    }



  }

  va_end(arguments);
}

void monitor_write_hex(u32int n)
{
    s32int tmp;

    k_printf("0x");

    char noZeroes = 1;

    int i;
    for (i = 28; i > 0; i -= 4)
    {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && noZeroes != 0)
        {
            continue;
        }

        if (tmp >= 0xA)
        {
            noZeroes = 0;
            k_putChar (tmp-0xA+'a' );
        }
        else
        {
            noZeroes = 0;
            k_putChar( tmp+'0' );
        }
    }

    tmp = n & 0xF;
    if (tmp >= 0xA)
    {
        k_putChar(tmp-0xA+'a');
    }
    else
    {
        k_putChar(tmp+'0');
    }

}

void monitor_write_dec(u32int n)
{
    if (n == 0)
    {
        k_putChar('0');
        return;
    }

    s32int acc = n;
    char c[32];
    int i = 0;
    while (acc > 0)
    {
        c[i] = '0' + acc%10;
        acc /= 10;
        i++;
    }
    c[i] = 0;

    char c2[32];
    c2[i--] = 0;
    int j = 0;
    while(i >= 0)
    {
        c2[i--] = c[j++];
    }
    k_printf(c2);
}


int k_strlen(char *string)
{
  int x = 0;

  while(*string != 0)
  {
    string++;
    x++;
  }

  return x;
}

int k_strcmp(char *stringOne, char *stringTwo) //return 0 if the same and 1 if not
{
  if(k_strlen(stringOne) != k_strlen(stringTwo))
  {
    return 1;
  }else{
    int x, equal = 1, length = k_strlen(stringOne);

    for(x = 0; x < length; x++)
    {
      if(*stringOne != *stringTwo)
      {
        equal = 0; //sets equal to false
        break;
        //~ k_printf("different", 10);
        //~ return 1;
        //~ break;
      }else{
        stringOne++;
        stringTwo++;
        //~ k_printf("9", 10);

      }
    }

    if(equal == 1)
    {
      return 0;
    }else if(equal == 0)
    {
      return 1;
    }
  }
  //~ return 10;

}

void k_intToChar(int integer, char *numbersInChar)
{
  int integerLength = math_intLength(integer), x, endX;
  int number = integer % 10;

  //~ char *numbersInChar[integerLength];

  for(x = 0; x < integerLength + 1; x++)
  {
    if(x != 0)
    {
      integer = (integer - number) / 10;
      number = (integer) % 10;
    }

    endX = integerLength - x - 1;

    numbersInChar[endX] = number;
  }

}

void k_numbersToAsciInChar(char *asciInChar, int integerLength)
{
  int x;

  //~ while(asciInChar[x] != 3)
  //~ {
  for(x = 0; x < integerLength; x++)
  {
    asciInChar[x] = asciInChar[x] + 48; //increase everthing in char array by 48 to make it asci (only works on integers)
    //~ asciInChar[x] = 48; //increase everthing in char array by 48 to make it asci (only works on integers)
    //~ x++; //move to next char in char array
  }

}

void shiftCursor(int shiftAmount)
{
  if(charPosition > 0 && shiftAmount < 0) //move cursor to the left
  {
    if(cursor_x != 1)
    {
      cursor_x = cursor_x + shiftAmount;
      charPosition = charPosition + shiftAmount;
    }else{
      cursor_y--;
      cursor_x = 81 + shiftAmount;
      charPosition = charPosition + shiftAmount;

    }

    // Scroll the screen if needed.
    scroll();
    // Move the hardware cursor.
    move_cursor();
  }else if(charPosition < charCount && shiftAmount > 0) //move cursor to the right
  {
    //~ u16int *location;
//~
    //~ location = video_memory + (cursor_y*80 + cursor_x + 1);
//~
    //~ k_printf("\n%d", *location);
//~
    if(cursor_x != 79)
    {
      cursor_x = cursor_x + shiftAmount;
      charPosition = charPosition + shiftAmount;
    }else{
      cursor_y++;
      cursor_x = 1 - shiftAmount;
      charPosition = charPosition + shiftAmount;
    }

      // Scroll the screen if needed.
      scroll();
      // Move the hardware cursor.
      move_cursor();

  }

}

void resetCharCount()
{
  charCount = 0;
  charPosition = 0;
}

void destroyCharPointer(char *pointer)
{
  while(*pointer != 0)
  {
    *pointer = 0;
    pointer++;
  }
}

void startingCursorY()
{
  //set starting y cursor to a variable
  startingYPos = cursor_y;

}

int getCursorXValue()
{
  return cursor_x;
}

void k_sortArray(int *array, int count)
{

  int one, two;

  int a, b;

  if(count > 1)
  {
    for(b = 0; b < count - 1; b++)
    {    
      for(a = 0; a < count - 1; a++)
      {
        one = array[a];
        two = array[a + 1];
  
        if(one > two)
        {
          array[a] = two;
          array[a + 1] = one;
        }
      }
    }
    
  }
  
}

void k_strchop(char *stringIn, char *stringOut, int start, int end)
{
  int x, length = k_strlen(stringIn);

  //~ destroyCharPointer(stringOut);

  if(start > end) //if start > end, switch the values
  {
    int tmp = end; //assign end to tmp var so we can set tmp to start
    end = start;
    start = tmp;
  }
  end++;
  stringIn = stringIn + start;

  if(stringIn != stringOut)
  {
    for(x = 0; x < (end - start); x++)
    {
      *stringOut = *stringIn;

      stringIn++;
      stringOut++;
    }
    //~ k_printf("\nIm in\n");
  }else{
    //~ k_printf("\nIm OTHER in\n");

    for(x = 0; x < (end - start); x++)
    {
      *stringOut = *stringIn;

      stringIn++;
      stringOut++;
    }

    for(x = x + 1; x < length; x++)
    {
      *stringOut = 0;

      stringOut++;
    }
  }

}

int k_elemInCharArray(char *array)
{
  int x = 0;

  while(array[x] != 0)
  {
    x++;
  }

  return x;

}

void k_strcpy(char *input, char *output)
{
  int length = k_strlen(input), x;
  destroyCharPointer(output);

  //~ while(*input != 0)
  for(x = 0; x < length; x++)
  {
    *output = *input;

    output++;
    input++;
  }

  //~ while(*input)
      //~ *output++ = *input++;

  //~ *output = '\0'; /* null terminates the string */

}

void k_save()
{
  int i;

  for (i = 0*80; i < 25*80 + 1; i++)
  {
    screen[i] = video_memory[i];
  }

}

void k_restore()
{
  int i;

  for (i = 0*80; i < 25*80 + 1; i++)
  {
    //~ screen[i] = video_memory[i];
    video_memory[i] = 3840 + screen[i];
    //~ k_putChar(screen[i]);
  }

}

void setScreenYMinMax(int yMinimun, int yMaximum)
{
  if(yMinimun >= 0)
  {
    yMin = yMinimun;
  }

  if(yMaximum >= 0)
  {
    yMax = yMaximum;
  }

  if(cursor_y < yMin)
  {
    cursor_y = yMin;
  }

  if(cursor_y > yMax)
  {
    cursor_y = yMax;
  }

  //~ scroll();
}

void deleteLine(int yValue)
{
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

  int x;

  if(yValue < 0)
  {
    yValue = cursor_y;
  }

  for(x = 0; x < 81; x++)
  {
    location = video_memory + (yValue*80 + x);
    *location = ' ' | attribute;
  }

}

void normalHCursor(int movement) //left and right cursor movement
{
  u16int *location;

  if(movement == -1) //left arrow
  {
    location = video_memory + (cursor_y*80 + cursor_x - 1);

    if(cursor_x != 0 && *location != 3840) //if cursor_x is at beggining and user wants to move left, go up one row and go to far right
    {
      cursor_x--;
    }else if(cursor_x <= 0)
    {
      cursor_x = 79;

      normalVCursor(1); //move cursor_y up one
    }
  }else if(movement == 1) //right arrow
  {

    //~ u16int *prev;
    u16int *after;

    //~ prev = video_memory + (cursor_y*80 + cursor_x - 1);
    location = video_memory + (cursor_y*80 + cursor_x);
    after = video_memory + (cursor_y*80 + cursor_x + 1);

    //~ if((cursor_x != 79 && *after != 3840) || (*prev != 3840 && *location != 3840 && *after == 3840)) //if cursor_x is at end and user wants to move tigh, go down one row and go to far left
    if((cursor_x != 79 && *after != 3840) || (*location != 3872 && *location != 3840 && *after == 3840)) //if cursor_x is at end and user wants to move tigh, go down one row and go to far left
    {
      cursor_x++;
    //~ }else if((cursor_x == 79) || (*prev != 3840 && *location == 3840 && *after == 3840))
    }else if((cursor_x == 79) || (*location == 3840 && *after == 3840) || (*location == 3872 && *after == 3840))
    {
      normalVCursor(-1); //move cursor_y down one
      cursor_x = 0;
    }


  }

  move_cursor();

  scroll();
}

void normalVCursor(int movement) //up and down cursor movement
{
  u16int *location;

  if(movement == 1) //up arrow
  {
    if(cursor_y != yMin)
    {
      cursor_y--;
    }
  }else if(movement == -1) //down arrow
  {
    location = video_memory + (cursor_y*80 + 0);

    if(cursor_y != (yMax - 1) && *location != 3840)
    {
      cursor_y++;
    }
  }

  location = video_memory + (cursor_y*80 + cursor_x);

  int x = 0;

  //~ while(*location == 3840 && cursor_x != 0) //if the location above or below is null, "3840" move the x cursor to the left
  while(*location == 3840 && cursor_x != 0) //if the location above or below is null, "3840" move the x cursor to the left
  {
    cursor_x--;
    location = video_memory + (cursor_y*80 + cursor_x);

    x = 1;

    if(cursor_x == 0)
    {
      break;
    }

  }

  u16int *nextRow;

  location = video_memory + (cursor_y*80 + cursor_x);
  nextRow = video_memory + ((cursor_y + 1)*80 + 0);

  //~ if(x == 1 && ((cursor_x != 0 && *nextRow == 3840) || (cursor_x == 0 && *location != 3872 && *nextRow != 3840))) //if we have entered the while loop above, add one to cursor_x to correct an error
  if(x == 1 && ((cursor_x != 0) || (cursor_x == 0 && *location != 3872 && *nextRow != 3840))) //if we have entered the while loop above, add one to cursor_x to correct an error
  {
    cursor_x++;
  }

  move_cursor();

  scroll();
}

void k_setprintf(int x, int y, char *text)
{
  int tmpX = cursor_x;
  int tmpY = cursor_y;

  cursor_x = x;
  cursor_y = y;

  k_printf(text);

  cursor_x = tmpX;
  cursor_y = tmpY;

  move_cursor();

  scroll();  
}
