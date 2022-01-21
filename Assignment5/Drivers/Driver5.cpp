/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include "Driver5.hpp"
#include "DriverShared.hpp"
#include <stdio.h>

#if DRIVER5
int main(int argc, char* argv[])
{
  typedef void (*TestFn)();
  TestFn tests[] = 
  {
    Driver5Part1Test0,
    Driver5Part1Test1,
    Driver5Part1Test2,
    Driver5Part1Test3,
    Driver5Part1Test4,
    Driver5Part1Test5,
    Driver5Part1Test6,
    Driver5Part1Test7,
    Driver5Part1Test8,
    Driver5Part1Test9,
    Driver5Part1Test10,
    Driver5Part1Test11,
    Driver5Part1Test12,
    Driver5Part1Test13,
    Driver5Part1Test14
  };

  return DriverMain(argc, argv, tests, DriverArraySize(tests));
}
#endif

// This just provided as an example (we don't use it)
// If you ever wanted to expose malloc/free to your language (see adding it to the core below)
Variant Allocate(Interpreter*, Function*, std::vector<Variant>& parameters)
{
  Variant result;
  int size = parameters[0].mValue.mInteger;
  result.mType = BytePointerType;
  result.mValue.mPointer = (char*)malloc(size);
  return result;
}

Variant Free(Interpreter*, Function*, std::vector<Variant>& parameters)
{
  char* pointer = parameters[0].mValue.mPointer;
  free(pointer);
  return Variant();
}

Variant PrintInteger(Interpreter*, Function*, std::vector<Variant>& parameters)
{
  printf("%d", parameters[0].mValue.mInteger);
  return Variant();
}

Variant PrintFloat(Interpreter*, Function*, std::vector<Variant>& parameters)
{
  printf("%f", parameters[0].mValue.mFloat);
  return Variant();
}

Variant PrintString(Interpreter*, Function*, std::vector<Variant>& parameters)
{
  printf("%s", parameters[0].mValue.mPointer);
  return Variant();
}

Variant PrintCharacter(Interpreter*, Function*, std::vector<Variant>& parameters)
{
  printf("%c", parameters[0].mValue.mByte);
  return Variant();
}

Function* AllocateFunction = nullptr;
Function* FreeFunction = nullptr;
Function* PrintIntegerFunction = nullptr;
Function* PrintFloatFunction = nullptr;
Function* PrintStringFunction = nullptr;
Function* PrintCharacterFunction = nullptr;

void InitializeCoreLibrary5(Library* library)
{
  InitializeCoreLibrary4(library);
  
  AllocateFunction = library->CreateFunction("Allocate", true);
  AllocateFunction->mType = library->GetFunctionType({ IntegerType }, BytePointerType);
  AllocateFunction->mCallback = Allocate;

  FreeFunction = library->CreateFunction("Free", true);
  FreeFunction->mType = library->GetFunctionType({ BytePointerType }, VoidType);
  FreeFunction->mCallback = Free;
  
  PrintIntegerFunction = library->CreateFunction("PrintInteger", true);
  PrintIntegerFunction->mType = library->GetFunctionType({ IntegerType }, VoidType);
  PrintIntegerFunction->mCallback = PrintInteger;
  
  PrintFloatFunction = library->CreateFunction("PrintFloat", true);
  PrintFloatFunction->mType = library->GetFunctionType({ FloatType }, VoidType);
  PrintFloatFunction->mCallback = PrintFloat;
  
  PrintStringFunction = library->CreateFunction("PrintString", true);
  PrintStringFunction->mType = library->GetFunctionType({ BytePointerType }, VoidType);
  PrintStringFunction->mCallback = PrintString;
  
  PrintCharacterFunction = library->CreateFunction("PrintCharacter", true);
  PrintCharacterFunction->mType = library->GetFunctionType({ ByteType }, VoidType);
  PrintCharacterFunction->mCallback = PrintCharacter;
}


//*********************************************************************************************

void RunInterpreterTest(int part, int test, const char* stream)
{
  printf("************** PART %d TEST %d **************\n", part, test);
  DfaState* root = CreateLanguageDfa();
  std::vector<Token> tokens;
  TokenizeAndDeleteRoot(root, stream, TokenNames, &tokens, ReadLanguageToken);
  printf("\n");

  try
  {
    RemoveWhitespaceAndComments(tokens);
    auto rootNode = ParseBlock(tokens);
    auto rootNodePointer = rootNode.get();
    if (rootNodePointer)
    {
      auto library = std::make_unique<Library>();
      InitializeCoreLibrary5(library.get());
      AddSinFunction(library.get());
      try
      {
        SemanticAnalyize(rootNodePointer, library.get());

        PrintTreeWithSymbols(rootNodePointer);
        printf("\n");

        library->Print();

        InterpreterPrePass(rootNodePointer);

        auto mainSymbol = library->mGlobalsByName.find("Main");
        if (mainSymbol != library->mGlobalsByName.end())
        {
          Function* function = dynamic_cast<Function*>(mainSymbol->second);
          if (function != nullptr)
          {
            if (function->mCallback != nullptr && function->mFunctionNode != nullptr)
            {
              if (function->mType->mParameterTypes.empty() && function->mType->mReturnType == IntegerType)
              {
                Interpreter* interpreter = CreateInterpreter();

                printf("\n-------------------------------------------\n");

                std::vector<Variant> parameters;
                Variant result = function->mCallback(interpreter, function, parameters);
                printf("\nMain returned: ");
                result.Print();
                printf("\n");

                DestroyInterpreter(interpreter);
              }
              else
              {
                printf("Main must take no parameters and return an Integer\n");
              }
            }
            else
            {
                printf("Main Function mCallback and mFunctionNode were not filled out (must be done in the pre-pass)\n");
            }
          }
          else
          {
            printf("Main was found, but it was not a Function\n");
          }
        }
        else
        {
          printf("Main entrypoint not found\n");
        }
      }
      catch (SemanticException& e)
      {
        PrintTreeWithSymbols(rootNodePointer);
        printf("\n");

        printf("%s\n", e.what());
      }
    }
    else
    {
      printf("Parsing Failed (Null Node)\n");
    }
  }
  catch (ParsingException&)
  {
    printf("Parsing Failed (Exception)\n");
  }

  printf("*******************************************\n\n");
}

void Driver5Part1Test0()
{
  RunInterpreterTest(1, 0, STRINGIZE(
    function Main() : Integer
    {
      return 99;
    }
  ));
}

void Driver5Part1Test1()
{
  RunInterpreterTest(1, 1, STRINGIZE(
    function Main() : Integer
    {
      PrintInteger(42);
      return 12345;
    }
  ));
}

void Driver5Part1Test2()
{
  RunInterpreterTest(1, 2, STRINGIZE(
    // This just tests that we actually handled hooking up Variant parameters
    // passed to us to the parameter's in that same position
    function CheckParameterPassing(n : Integer)
    {
      PrintInteger(n);
      return;
    }

    function Main() : Integer
    {
      CheckParameterPassing(42);
      return 54321;
    }
  ));
}

void Driver5Part1Test3()
{
  RunInterpreterTest(1, 3, STRINGIZE(
    function Main() : Integer
    {
      var test : Integer = 7;
      return test;
    }
  ));
}

void Driver5Part1Test4()
{
  RunInterpreterTest(1, 4, STRINGIZE(
    function Main() : Integer
    {
      var test : Integer = 42;
      test = 11;
      return test;
    }
  ));
}

void Driver5Part1Test5()
{
  RunInterpreterTest(1, 5, STRINGIZE(
    function Main() : Integer
    {
      var intTest : Integer;
      intTest = (5 + 5) * 2 + 550 / 10 % 10;
      return intTest;
    }
  ));
}

void Driver5Part1Test6()
{
  RunInterpreterTest(1, 6, STRINGIZE(
    function Main() : Integer
    {
      if (5 > 10)
      {
        return 11;
      }
      else
      {
        if (20 <= 10)
        {
          return 22;
        }
        else if (100 == 100)
        {
          return 33;
        }
        else
        {
          return 44;
        }
      }
    }
  ));
}

void Driver5Part1Test7()
{
  RunInterpreterTest(1, 7, STRINGIZE(
    function True() : Boolean
    {
      PrintInteger(1);
      return true;
    }

    function False() : Boolean
    {
      PrintInteger(0);
      return false;
    }

    function Main() : Integer
    {
      var test : Integer = 0;

      if (True() && True())
      {
        test = test + 1;
      }
      if (True() && False())
      {
        test = test + 2;
      }
      if (False() && True())
      {
        test = test + 4;
      }
      if (False() && False())
      {
        test = test + 8;
      }
      
      if (True() || True())
      {
        test = test + 16;
      }
      if (True() || False())
      {
        test = test + 32;
      }
      if (False() || True())
      {
        test = test + 64;
      }
      if (False() || False())
      {
        test = test + 128;
      }
      return test;
    }
  ));
}

void Driver5Part1Test8()
{
  RunInterpreterTest(1, 8, STRINGIZE(
    function Main() : Integer
    {
      PrintString("Hello");
      PrintString("\tWorld\n");
      PrintCharacter('A');
      PrintCharacter('\t');
      PrintCharacter('B');
      PrintCharacter('\t');
      PrintCharacter('C');
      PrintCharacter('\n');
      return 123;
    }
  ));
}

void Driver5Part1Test9()
{
  RunInterpreterTest(1, 9, STRINGIZE(
    function Fibonacci(n : Integer) : Integer
    {
      var a : Integer = 0;
      var b : Integer = 1;
      var i : Integer = 0;

      while (i < n)
      {
        var c : Integer = a + b;
        a = b;
        b = c;

        i = i + 1;
      }

      return a;
    }

    function Main() : Integer
    {
      return Fibonacci(23);
    }
  ));
}

void Driver5Part1Test10()
{
  RunInterpreterTest(1, 10, STRINGIZE(
    var Global : Integer;
    
    function Test(value : Integer)
    {
      Global = Global + value;
      return;
    }

    function Main() : Integer
    {
      Global = 100;
      Test(23);
      return Global;
    }
  ));
}

void Driver5Part1Test11()
{
  RunInterpreterTest(1, 11, STRINGIZE(
    function Main() : Integer
    {
      var f : Float = 355.5f;
      return f as Integer as Byte as Integer + true as Integer;
    }
  ));
}

// Tests many binary operations, if statements, while loops, etc
void Driver5Part1Test12()
{
  RunInterpreterTest(1, 12, STRINGIZE(
    var Counter : Integer;
    function Main() : Integer
    {
      var lives : Integer = 5;

      while (Counter <= 100)
      {
        lives = lives + ((Counter as Float) * 5.0f) as Integer;

        if (lives > 4000)
        {
          lives = (((lives as Float) * (0.3f + 0.2f)) % 500.0f) as Integer;
        }
        else
        {
          lives = lives - 1;
        }

        Counter = Counter + 1;
      }
      return lives;
    }
  ));
}

void Driver5Part1Test13()
{
  RunInterpreterTest(1, 13, STRINGIZE(

    function PrintPlural(count : Integer)
    {
      if (count != 1)
      {
        PrintString("s");
      }
      return;
    }

    function Main() : Integer
    {
      var count : Integer = 99;
      while (count >= 1)
      {
        PrintInteger(count);
        PrintString(" bottle");
        PrintPlural(count);
        PrintString(" of beer on the wall...\n");
        PrintInteger(count);
        PrintString(" bottle");
        PrintPlural(count);
        PrintString(" of beer...\n");
        PrintString("Take one down, pass it around\n");

        count = count - 1;

        PrintInteger(count);
        PrintString(" bottle");
        PrintPlural(count);
        PrintString(" of beer on the wall!\n\n");
      }
      return 0;
    }
  ));
}

void Driver5Part1Test14()
{
  RunInterpreterTest(1, 14, STRINGIZE(
    function Main() : Integer
    {
      PrintFloat(Sin(1.0f));
      return 0;
    }
  ));
}

