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
