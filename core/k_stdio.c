/*
 * k_stdio.c
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

#include <system.h>
#include <stdint.h>
#include <stdarg.h>

// The standard VGA framebuffer starts at 0xB8000.
u16int *video_memory = (u16int *)0xB8000;

// Stores the cursor position.
u8int cursor_x = 0;
u8int cursor_y = 0;

// The default background colour is black (0), the foreground is white (15).
u8int backColour = 0;
u8int foreColour = 15;

int charCount = 0, charPosition = 0, lineCount = 0, startingYPos = 0;

int yMin = 0, yMax = 25;
u32int screen[25 * 80]; //char array to hold screen characters when k_save() is called

//extern of the keyboard handler
extern void *(*key_handle)(char);

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
    s32int i;
    for(i = yMin*80; i < (yMax - 1)*80 + 1; i++)
    {
      video_memory[i] = video_memory[i+80];
    }

    // The last line should now be blank. Do this by writing
    // 80 spaces to it.
    for(i = (yMax - 1)*80; i < yMax*80 + 1; i++)
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

  // The attribute byte is the top 8 bits of the word we have to send to the
  // VGA board.
  u16int attribute = attributeByte << 8;
  u16int *location;

  // Handle a backspace, by moving the cursor back one space
  if(c == '\b')
  {
    int x;

    if(charPosition > 0)
    {

      if(!cursor_x) //move one back and x_cursor to end
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
  else if(c == '\t')
  {
    cursor_x = (cursor_x + 8) & ~(8-1);
    charCount = charCount + 5;
    charPosition = charPosition + 5;

  }else if(c == '\r')  // Handle carriage return "Enter Key"
  {
    //~ asm volatile("cli");
    //~ char *input, *split;
    //~ static char input[250], split[250]; //allocates maximum size for input, being 10 lines of 25 chars each
    char *input, *split; //allocates maximum size for input, being 10 lines of 25 chars each

    input = (char*)kmalloc(250);
    split = (char*)kmalloc(250);

    memset(input, 0, 250);
    memset(split, 0, 250);

    //~ getTypedText(charCount, cursor_x, cursor_y, input);
    getTypedText(charCount, startingYPos, cursor_y, input); //gets the stuff typed when enter is pressed
    cursor_y++;
    cursor_x = 0;

    saveInputToBuffer(input); //saves "input" to array so it can be accessed with up arrow later

    formatInput(input, split); //assigns command to "input" and args to "split"

    //~ removeTrailingSpaces(split);
    
    executeInput(input, split); //executes program "input" with args "split"

    //~ k_printf("\n\n%s%s%s\n", split, split, split);

    charCount = 0; //resets the char counter and charUnderlinePosition
    charPosition = 0;

    location = video_memory + (cursor_y * 80 + cursor_x);
    *location = ' ' | attribute;

    addShellIndent(); //adds the shell "->" indent

    //~ destroyCharPointer(input); //destroys char array to no interfere with others
    //~ destroyCharPointer(split); //destroys char array to no interfere with others

    lineCount = 0;

    //~ kfree(input);
    //~ kfree(split);
    //~ asm volatile("sti");

  }else if (c == '\n') // Handle newline by moving cursor back to left and increasing the row
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

    //if we are adding a char at the end of our text
    if(charPosition == charCount)
      charCount++;

    charPosition++;
  }

  // Check if we need to insert a new line because we have reached the end
  // of the screen.
  if(cursor_x >= 80)
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

  //get the args after char *c
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
          foreColour = WHITE;
        }else if(c[i+2] == 'b' && c[i+3] == 'k') //black text
        {
          foreColour = BLACK;
          i++;
        }else if(c[i+2] == 'b' && c[i+3] == 'l') //dark blue text
        {
          foreColour = DARK_BLUE;
          i++;
        }else if(c[i+2] == 'b' && c[i+3] == 'r') //brown text
        {
          foreColour = BROWN;
          i++;
        }else if(c[i+2] == 'g') //dark green text
        {
          foreColour = DARK_GREEN;
        }else if(c[i+2] == 'r') //dark red text
        {
          foreColour = DARK_RED;
        }else if(c[i+2] == 'l' && c[i+3] == 'b' && c[i+4] == 'l') //light blue text
        {
          foreColour = LIGHT_BLUE;
          i += 2;
        }

        i = i + 3;

      }else if(c[i+1] == 'C') //user wants to print colored background
      {
        if(c[i+2] == 'w') //white background
          backColour = WHITE;
        else if(c[i+2] == 'b' && c[i+3] == 'k') //black background
        {
          backColour = BLACK;
          i++;
        }else if(c[i+2] == 'b' && c[i+3] == 'l') //dark blue background
        {
          backColour = DARK_BLUE;
          i++;
        }else if(c[i+2] == 'b' && c[i+3] == 'r') //brown background
        {
          backColour = BROWN;
          i++;
        }else if(c[i+2] == 'g') //dark green background
          backColour = DARK_GREEN;
        else if(c[i+2] == 'r') //dark red background
          backColour = DARK_RED;

        i = i + 3;

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
          if(!tmp && noZeroes)
            continue;
      
          if (tmp >= 0xA)
          {
            noZeroes = 0;
            k_putChar(tmp-0xA+'a');
          }else{
            noZeroes = 0;
            k_putChar(tmp+'0');
          }
        }
      
        tmp = hexArg & 0xF;
        if (tmp >= 0xA)
          k_putChar(tmp-0xA+'a');
        else
          k_putChar(tmp+'0');

        i = i + 2;
      }
    }

    if(i < stringLength + 1 && c[i] != '%')
    {
      if(c[i] == 0) //if i is at the terminating 0 '\000' of a string, break from loop
        break;
      else{
        k_putChar(c[i]);
        i++;
      }
    }
  }

  va_end(arguments);
}

enum __k_scanf_type__
{
  __k_scanf_done__,
  __k_scanf_string__,
  __k_scanf_decimal__
};
  
static enum __k_scanf_type__ __k_scanf_type__;

enum __k_scanf_text_handle__
{
  __k_scanf_hide__,
  __k_scanf_print__
};

static enum __k_scanf_text_handle__ __k_scanf_text_handle__;

//internal definition of k_scanf
static u32int __k_scanf__(char input);

char *__k_scanf_out__(u32int add_or_get, char *input)
{
  static char *text = 0;

  //we want to set the static char *text;
  if(add_or_get)
  {
    text = input;
    //there is nothing to return
    return 0;
  }else{//we want to get (return) the static char *text;
    //store the location of text temporarily
    u32int tmp;
    tmp = (u32int)text;

    //reset the value of text
    text = 0;

    return (char*)tmp;
  }
    
}

u32int k_scanf(char *type, u8int **ptr)
{
  //set default flags
  __k_scanf_text_handle__ = __k_scanf_print__;

  u32int i = 0;
  while(*(type + i) != '%')
  {
    switch(*(type + i))
    {
      case 'h':
        __k_scanf_text_handle__ = __k_scanf_hide__;
        break;
      default:
        k_printf("k_scanf error; unknown %s argument", *(type + i));
        return 1; //error
    }
    i++;
  }

  //make sure interupts are enabled
  asm volatile("sti");

  void *(*old_key_handle)(char);

  old_key_handle = key_handle;
  key_handle = (void*)__k_scanf__;

  //set the correct __k_scanf_type
  //+1 because the *(type + i) is the % and we want the char after it
  switch(*(type + i + 1))
  {
    case 's':
      __k_scanf_type__ = __k_scanf_string__;
      break;
    case 'd':
      __k_scanf_type__ = __k_scanf_decimal__;
      break;
    default: //error
      return 1;
  }

  //wait until the __k_scanf__ internal sends an escape call
  while(__k_scanf_type__ != __k_scanf_done__);

  //retrieve the output of the __k_scanf__ internal
  char *out;
  out = __k_scanf_out__(0, 0);

  *ptr = (u8int*)out;

  //reset the keyboard driver handler
  key_handle = old_key_handle;

  //sucess!
  return 0;
}

static void __k_scanf_print_input__(char input)
{
  if(__k_scanf_text_handle__ == __k_scanf_print__)
    //print the character on the screen
    k_putChar(input);
}

static u32int __k_scanf__(char input)
{
  static void *text = 0;
  static u32int size = 50, count = 0;
  switch(__k_scanf_type__)
  {
    //string
    case __k_scanf_string__:
    {
      if(!text)
      {
        text = (void*)kmalloc(size);
        memset((char*)text, 0x0, size);
      }else if(count == size)
      {
        //do a reallocation
        text = (void*)krealloc((u32int*)text, size, 2 * size);

        //increase size
        size *= 2;

      }

      if(input == '\b')
      {
        if(count)
        {
          count--;
          //count down 1 and replace that character with a 0
          *((char*)text + count) = 0;

          __k_scanf_print_input__(input);
        }
      }else if(input == '\r') //if the input is a carridge return (enter)
      {

        //we do not what to print a carrige return, since the runShellFunction will initiat, print \n
        k_putChar('\n');

        //send the static char *text to our middle out function to be retrieved from the real function
        __k_scanf_out__(1, (char*)text);

        //reset the variables
        size = 50;
        count = 0;
        text = 0;

        //send a signal that we have finished
        __k_scanf_type__ = __k_scanf_done__;
        
        //sucess
        return 0;
      }else{ //then it is just normal text

        //save the character to our buffer
        *((char*)text + count) = input;
        count++;

        //print the text
        __k_scanf_print_input__(input);
      }
    }
    case __k_scanf_decimal__:
    {
      if(!text)
      {
        text = (void*)kmalloc(sizeof(u32int));
        *(u32int*)text = 0;
      }

      if(input == '\b')
      {
        //the value of text must not be equal to zero
        if(*(u32int*)text)
        {
          //simply divide by 10 to reverse ex) 199 / 10 = 19.9, the .9 is dropped due to integer, 19 is final output
          *(u32int*)text /= 10;
      
          //print the text
          __k_scanf_print_input__(input);
      
        }
      }if(input == '\r')
      {
        //we do not what to print a carrige return, since the runShellFunction will initiat, print \n
        k_putChar('\n');

        //send the static char *text to our middle out function to be retrieved from the real function
        __k_scanf_out__(1, (char*)text);

        text = 0;

        //send a signal that we have finished
        __k_scanf_type__ = __k_scanf_done__;
        
        //sucess
        return 0;
      }

      //if the input is not an integer (0-9), then ignore it
      if(input < '0' || input > '9')
        return 1;

      //subtract 48 to make '0' equal 0, in the ascii table
      u32int number;
      number = input - 48;

      /*make sure that we do not exceed out u32int size limit
       * if text is big enough to fit another one of itself 
       * inside the the max u32int number limit, it is fine,
       * in theory we have to fit 10 of these plus a little bit more*/
      if((u32int)(-1) - *(u32int*)text > *(u32int*)text)
      {
        if(*(u32int*)text)
          *(u32int*)text *= 10;

        *(u32int*)text += number;

        //print the text
        __k_scanf_print_input__(input);
      }
    }
    default: //some sort of error
      return 1;
  }

  //sucess
  return 0;
}

void monitor_write_hex(u32int n)
{
  s32int tmp;

  k_printf("0x");

  s8int noZeroes = 1;

  s32int i;
  /*we start at 28 because we want to start from the 4 most significant
   * bits in the u32int n */
  for (i = 28; i > 0; i -= 4)
  {
    //assign tmp to be a 4 bit number
    tmp = (n >> i) & 0xF;
    //if tmp is 0, then there is no need to process those bits
    if(!tmp && noZeroes)
    {
      continue;
    }

    if(tmp >= 0xA)
    {
      noZeroes = 0;
      k_putChar(tmp - 0xA + 'a');
    }else{
      noZeroes = 0;
      k_putChar( tmp+'0' );
    }
  }

  tmp = n & 0xF;
  if(tmp >= 0xA)
  {
    k_putChar(tmp - 0xA + 'a');
  }else{
    k_putChar(tmp + '0');
  }

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
  s32int integerLength = math_intLength(integer), x, endX;
  s32int number = integer % 10;

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
  //~ while(*pointer != 0)
  //~ {
    //~ *pointer = 0;
    //~ pointer++;
  //~ }

  memset(pointer, 0, strlen(pointer));
  
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
  u32int i;

  for(i = 0; i < 25 * 80 + 1; i++)
    screen[i] = *((u32int*)video_memory + i);

}

void k_restore()
{
  u32int i;

  for(i = 0; i < 25 * 80 + 1; i++)
    *((u32int*)video_memory + i) = screen[i];

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

void working()
{
  asm volatile("sti");
  init_timer(globalFreq); // Initialise timer to globalFreq-Hz

  //loop untill the OS kills this process
  for(;;)
  {
    //sleep a bit
    mSleep(400);

    //then print the text
    k_printf(".");
  }
}

//TODO, make this function take variable number of arguments
void k_setprintf(int x, int y, char *text, void *arg1, void *arg2, void *arg3)
{
  
  int tmpX = cursor_x;
  int tmpY = cursor_y;

  cursor_x = x;
  cursor_y = y;

  k_printf(text, arg1, arg2, arg3);

  cursor_x = tmpX;
  cursor_y = tmpY;

  move_cursor();

  scroll();  
}

void k_warning(char *warning, char *added_text, u32int row)
{
  setScreenYMinMax(row, 25); //reserve rows
  
  //row - 1 since row starts at 0
  k_setprintf(0, row - 1, "%Cr%cbk%s%s%Cbk%cw ", warning, added_text, 0);

}
