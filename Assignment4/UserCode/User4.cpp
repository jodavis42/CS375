/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver4.hpp"
#include "Visitor.hpp"
#include "TypeVisitor.hpp"
#include "TypeResolverVisitor.hpp"
#include "MemberResolverVisitor.hpp"
#include "SymbolPrinterVisitor.hpp"
#include "ExpresionResolverVisitor.hpp"

template <typename SymbolType>
SymbolType* CreateSymbolOfType(Library* library, const std::string& name, bool isGlobal)
{
  auto&& symbol = std::make_unique<SymbolType>();
  SymbolType* symbolPtr = symbol.get();

  symbolPtr->mName = name;
  symbolPtr->mLibrary = library;
  library->mOwnedSymbols.push_back(std::move(symbol));
  if (isGlobal)
  {
    library->mGlobals.push_back(symbolPtr);
    library->mGlobalsByName[name] = symbolPtr;
  }

  return symbolPtr;
}

Type* CreateTypeSymbol(Library* library, const std::string& name, bool isGlobal, TypeMode::Enum typeMode)
{
  Type* type = CreateSymbolOfType<Type>(library, name, isGlobal);
  type->mMode = typeMode;
  return type;
}

Type* CreateTypeSymbolErrorOnDuplicate(Library* library, const std::string& name, bool isGlobal, TypeMode::Enum typeMode)
{
  if (LibraryHelpers::FindGlobalSymbolByName(library, name) != nullptr)
  {
    ErrorSameName(name);
  }

  return CreateTypeSymbol(library, name, isGlobal, typeMode);
}

Type* Library::CreateType(const std::string& name, bool isGlobal)
{
  return CreateTypeSymbolErrorOnDuplicate(this, name, isGlobal, TypeMode::Class);
}

Variable* Library::CreateVariable(const std::string& name, bool isGlobal)
{
  return CreateSymbolOfType<Variable>(this, name, isGlobal);
}

Function* Library::CreateFunction(const std::string& name, bool isGlobal)
{
  return CreateSymbolOfType<Function>(this, name, isGlobal);
}

Label* Library::CreateLabel(const std::string& name, bool isGlobal)
{
  return CreateSymbolOfType<Label>(this, name, isGlobal);
}

Type* Library::GetPointerType(Type* pointerToType)
{
  std::string typeName = pointerToType->mName + "*";
  auto&& it = mGlobalsByName.find(typeName);
  if (it != mGlobalsByName.end())
    return static_cast<Type*>(it->second);


  Type* type = CreateTypeSymbolErrorOnDuplicate(this, typeName, true, TypeMode::Pointer);
  type->mPointerToType = pointerToType;
  return type;
}

Type* Library::GetFunctionType(std::vector<Type*> parameterTypes, Type* returnType)
{
  std::stringstream stream;
  stream << "function(";
  for (size_t i = 0; i < parameterTypes.size(); ++i)
  {
    if (i != 0)
      stream << ", ";
    stream << parameterTypes[i]->mName;
  }
  stream << ")";
  if (returnType != nullptr)
    stream << " : " << returnType->mName;
  std::string typeName = stream.str();

  Type* existingType = LibraryHelpers::FindGlobalSymbolByName<Type>(this, typeName);
  if (existingType != nullptr)
    return existingType;

  Type* type = CreateTypeSymbolErrorOnDuplicate(this, typeName, true, TypeMode::Function); 
  type->mParameterTypes = parameterTypes;
  type->mReturnType = returnType;
  return type;
}

void SemanticAnalyize(AbstractNode* node, Library* library)
{
  TypeVisitor typeVisitor(library);
  node->Walk(&typeVisitor);
  TypeResolverVisitor typeResolverVisitor(library);
  node->Walk(&typeResolverVisitor);
  MemberResolverVisitor memberResolverVisitor(library);
  node->Walk(&memberResolverVisitor);
  ExpresionResolverVisitor expresionResolverVisitor(library);
  node->Walk(&expresionResolverVisitor);
}

void PrintTreeWithSymbols(AbstractNode* node)
{
  SymbolPrinterVisitor visitor;
  node->Walk(&visitor);
}
