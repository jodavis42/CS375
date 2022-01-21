/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#pragma once
#ifndef COMPILER_CLASS_DRIVER_5
#define COMPILER_CLASS_DRIVER_5

#include "Driver1.hpp"
#include "Driver2.hpp"
#include "Driver3.hpp"
#include "Driver4.hpp"
#include "AstNodes.hpp"
#include "SymbolTable.hpp"

/***************************** DRIVER *****************************/
class Interpreter;

// Run single pass visitor over the AST (does not need your Interpreter yet):
// 1. Pre-compute literal Variants 
// 2. Set the mCallback and mFunctionNode on every Function symbol that we created
// 3. Set the mOperator function on every Binary and Unary node
void InterpreterPrePass(AbstractNode* node);

// Creates your interpreter class
Interpreter* CreateInterpreter();

// Destroys your interpreter class
void DestroyInterpreter(Interpreter* interpreter);

// This function must be implemented and set as the mCallback on every FunctionNode's Function symbol
Variant InterpreterFunctionCall(Interpreter* interpreter, Function* function, std::vector<Variant>& arguments);

// Use CreateFunction to create a custom 'Sin' function that calls the C math sin (float version)
// Be sure to set the mType using GetFunctionType, the signature should be function Sin(Float) : Float
// Create a C++ function to set as the mCallback, which will actually invoke the 'sin' function
// This is exactly how you would bind your scripting language to your game engine, or any other C++ program
void AddSinFunction(Library* library);

// More symbols that are bound to the core library
// You generally don't need to use these, but they're exposed
extern Function* AllocateFunction;
extern Function* FreeFunction;
extern Function* PrintIntegerFunction;
extern Function* PrintFloatFunction;
extern Function* PrintStringFunction;
extern Function* PrintCharacterFunction;

// C++ representations for the core types
// These are potentially useful in helper macros
typedef void  Void;
typedef int   Integer;
typedef float Float;
typedef bool  Boolean;
typedef char  Byte;

/***************************** TESTS  *****************************/

// Parse an empty main function that returns 99
void Driver5Part1Test0();

// Prints an integer literal (visit CallNode, InterpreterFunctionCall does not need to be implemented)
void Driver5Part1Test1();

// Prints an integer literal (both CallNode and InterpreterFunctionCall must be implemented)
void Driver5Part1Test2();

// Uses an initialized local Integer variable
void Driver5Part1Test3();

// Uses binary assignment to a local variable
void Driver5Part1Test4();

// Uses several binary operators on Integers
void Driver5Part1Test5();

// Tests a if statements with binary comparisons between Integers
void Driver5Part1Test6();

// Tests short circuit evaluation via &&, || and function calls
void Driver5Part1Test7();

// Tests printing out string and character literals
void Driver5Part1Test8();

// Tests a while loop via the fibonacci function
void Driver5Part1Test9();

// Tests global variables
void Driver5Part1Test10();

// Tests cast operations between Integer, Float, and Byte
void Driver5Part1Test11();

// Tests many binary operations, if statements, while loops, etc
void Driver5Part1Test12();

// Prints out a fun song
void Driver5Part1Test13();

// Tests a custom bound 'sin' function (same idea as binding a function from your game engine into your scripting language!)
void Driver5Part1Test14();

#endif
