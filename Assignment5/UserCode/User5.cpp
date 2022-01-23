/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver5.hpp"
#include "Interpreter.hpp"

void InterpreterPrePass(AbstractNode* node)
{
  InterpreterPrePassVisitor visitor(InterpreterFunctionCall);
  node->Walk(&visitor);
}

Interpreter* CreateInterpreter()
{
  Interpreter* interpreter = new Interpreter();
  return interpreter;
}

void DestroyInterpreter(Interpreter* interpreter)
{
  delete interpreter;
}

Variant InterpreterFunctionCall(Interpreter* interpreter, Function* function, std::vector<Variant>& arguments)
{
  interpreter->mFunctionStack.BeginFunction(function);
  // Collect the parameters and map them to the symbols
  for (size_t i = 0; i < function->mFunctionNode->mParameters.size(); ++i)
  {
    auto&& paramNode = function->mFunctionNode->mParameters[i];
    interpreter->mFunctionStack.Add(paramNode->mSymbol, arguments[i]);
  }

  Variant result;
  try
  {
    function->mFunctionNode->Walk(interpreter);
  }
  catch (ReturnException returnException)
  {
    result = returnException.mVariant;
  }
  interpreter->mFunctionStack.EndFunction();
  
  return result;
}

Variant EvaluateSin(Interpreter*, Function*, std::vector<Variant>& parameters)
{
  Variant result;
  result.mValue.mFloat = sin(parameters[0].mValue.mFloat);
  result.mType = FloatType;
  return result;
}

void AddSinFunction(Library* library)
{
  Function* function = library->CreateFunction("Sin", true);
  function->mType = library->GetFunctionType({ FloatType }, FloatType);
  function->mCallback = EvaluateSin;
}
