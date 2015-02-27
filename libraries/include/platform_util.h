#ifndef MOAROBOT_PLATFORM_UTIL_H
#define MOAROBOT_PLATFORM_UTIL_H

/** An utility for crosspltform
 *  Author: Lun-Cheng Chu
 *   Mail: lc@cs.wisc.edu
 */

#include<stdio.h>
//
// add the line "#define NONWIXEL"
// if you want to test code in standard c environment.
// The following macros help debugging in standard c environment.
//

#ifndef NONWIXEL
  #include<wixel.h>
  #define MY_PRINT(str) 
  #define MY_PRINT2(str, arg) 
#else
  #define XDATA
  #define MY_PRINT(str) printf(str);
  #define MY_PRINT2(str, arg) printf(str, arg);
#endif



#endif
