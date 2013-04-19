/*
 * k_math.c
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

float math_floor(float floorNumber)
{
  float returnFlooredNumber = floorNumber;

  returnFlooredNumber = (float)((int)returnFlooredNumber);

  return returnFlooredNumber;

}

float math_ceil(float ceilNumber)
{
  float returnCieledNumber = ceilNumber;

  returnCieledNumber = (float)((int)returnCieledNumber) + 1;

  return returnCieledNumber;
}
float math_abs(float absNumber)
{
  if(absNumber < 0)
  {
    //if(absNumber == (int)absNumber)
    //{
      //int intAbsNumber = (int)absNumber;

      //intAbsNumber = intAbsNumber << 1;
      //intAbsNumber = intAbsNumber >> 1;

      ////~ k_printf("\nabs int: %d", math_abs(-10)); 
      //return (float)intAbsNumber;
    //}else{
      
    absNumber = -1 * absNumber;
      //return absNumber;
      
    //}
  } //else{
    //~ 
  return absNumber;
  //~ }
  
}

float math_log10 (float logNumber)
{
  //TODO make log function
  float exp = 0.0, ans = 1.0, expAdder = 1.0;
  
  while (ans != logNumber)
  {
    if(ans < logNumber)
    {
      ans = ans * 10;
      exp = exp + expAdder;
    }else if(ans > logNumber)
    {
      expAdder = expAdder / 10;
      
      ans = ans * 10;
      exp = exp - expAdder;
    }
  }
  
  if (ans == logNumber)
  {
    return exp;
  }else{
    /*print error*/
  }
}

int math_intLength(int intNumber)
{
  int inv = 0, count = 0, originalInt = intNumber;

  if(intNumber < 0)
  {
    intNumber = math_abs(intNumber);
  }

  while (intNumber > 0)
  {
    count = count + 1;

    //~ intNumber = intNumber / 10;

    inv = inv * 10 + (intNumber % 10);
    intNumber = intNumber / 10; //since reverseNumber is int, dividing by ten also floors integer
  }

  if(originalInt != 0)
  {
    return count;
  }else if(originalInt == 0)//if the original in is 0, above will not enter while loop, so count will equal 0
  {			    //and will return a number length of 0, so here i will brute return a length of 1
    return 1;
  }

}

int math_pow(int base, int exponent)
{
  int x, output = 1;

  for(x = 0; x < exponent; x++)
  {
    output = output * base;
  }

  return output;
}
