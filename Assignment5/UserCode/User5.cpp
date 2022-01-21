/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver5.hpp"

void InterpreterPrePass(AbstractNode* node)
{
}

Interpreter* CreateInterpreter()
{
  return nullptr;
}

void DestroyInterpreter(Interpreter* interpreter)
{
}

Variant InterpreterFunctionCall(Interpreter* interpreter, Function* function, std::vector<Variant>& arguments)
{
  return Variant();
}

void AddSinFunction(Library* library)
{
}
