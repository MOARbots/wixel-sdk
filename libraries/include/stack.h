#ifndef MOAROBOT_STACK_H
#define MOAROBOT_STACK_H

/** A stack implemntation for Wixel. 
 *
 *  Author: Lun-Cheng Chu
 *   Mail: lc@cs.wisc.edu
 */

#include "platform_util.h"

/*************************************
 * Container: Stack
 *************************************/
#define SContSize 64
#define STupleSize 5 

// forward decl.
struct Stack;

//
// FiveBytes Structure
//
typedef struct STuple{
  struct Stack* caller; 
  char bytes[STupleSize];
} STuple;

// 
// Container: Stack 
//  - contains several fivebytes data
// 
typedef struct Stack
{
  // member data
  STuple Sdata[SContSize];
  int top_idx; // indicate the available slot in data[]

  // member function declaration
  int (*Push)(STuple* p) __reentrant;
  int (*Pop)(STuple* p)  __reentrant;
} Stack;


int InitStack(Stack* stack);
int LinkStackSTuple(Stack* stack, STuple* tuple);

#endif 
