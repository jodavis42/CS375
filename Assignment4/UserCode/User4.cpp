/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver4.hpp"

Type* Library::CreateType(const std::string& name, bool isGlobal)
{
  return nullptr;
}

Variable* Library::CreateVariable(const std::string& name, bool isGlobal)
{
  return nullptr;
}

Function* Library::CreateFunction(const std::string& name, bool isGlobal)
{
  return nullptr;
}

Label* Library::CreateLabel(const std::string& name, bool isGlobal)
{
  return nullptr;
}

Type* Library::GetPointerType(Type* pointerToType)
{
  return nullptr;
}

Type* Library::GetFunctionType(std::vector<Type*> parameterTypes, Type* returnType)
{
  return nullptr;
}

void SemanticAnalyize(AbstractNode* node, Library* library)
{
}

void PrintTreeWithSymbols(AbstractNode* node)
{
}
