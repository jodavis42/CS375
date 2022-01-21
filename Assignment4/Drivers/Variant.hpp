/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#pragma once
#ifndef COMPILER_CLASS_VARIANT
#define COMPILER_CLASS_VARIANT

#include <vector>

class Type;
class Function;
class Variant;
class Interpreter;

// All functions that we bind get called through this function pointer
typedef Variant (*FunctionCallback)(Interpreter* interpreter, Function* function, std::vector<Variant>& arguments);

union VariantValue
{
  int       mInteger;
  float     mFloat;
  bool      mBoolean;
  char      mByte;
  char*     mPointer;
  Function* mFunction;
};

// A variant can store any type within it
class Variant
{
public:
  // Initializes the variant to the NullType within the core library
  // Zeroes out all other members
  Variant();

  // Turns the variant into a string
  std::string ToString();
  
  // Allows us to print the variant out for debugging purposes
  void Print();

  // Clear a variant back to the NullType (and all zeroes)
  void Clear();

  // The type can be any of the primitive types or a
  // pointer to a type in which case mPointer will be set
  // If this is a Function, mFunction should point at the symbol
  Type* mType;
  
  // This normally would be done as a union, however, for debugging
  // purposes its easier to see which value it is if the rest are zeroed out
  VariantValue mValue;

  // In most compilers, local and global variables typically result in a reference to the type they store,
  // rather than the type itself. For simplicity, our compiler avoided doing anything with LValues, RValues. or references
  // In order to be able to assign to a variable, for example in saying 'test = 5', 'test' must be a pointer to the variable's mInteger
  // We can achieve this easily by pointing back at the original variable's mValue or dereferenced memory
  void* mPointerToValue;
};

#endif
