/*
 * k_math.h
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

#ifndef K_MATH
#define K_MATH

/*floors a number*/
float math_floor(float floorNumber);

/*ceils a number*/
float math_ceil(float ceilNumber);

/*gets the absolute value of a number*/
float math_abs(float absNumber);

//Make this log10 work
float math_log10 (float logNumber);

/*finds the length of an integer, ex: if input == 10, output would == 2, if input == 12345, output == 5*/
int math_intLength(int intNumber);

/*takes the power of a number, only works with positive exponent, as of NOW*/
int math_pow(int base, int exponent);

#endif
