#pragma once

#include "Visitor.hpp"
#include "LibraryHelpers.hpp"
#include "../Drivers/Driver4.hpp"

class TypeResolverVisitor : public Visitor
{
public:
  
  TypeResolverVisitor(Library* library)
  {
    mLibrary = library;
  }

  virtual VisitResult Visit(BlockNode* node)
  {
    mBlock = node;
    Walk(mBlock->mGlobals);
    mBlock = nullptr;
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(ClassNode* node)
  {
    mClass = node;
    Walk(node->mMembers);
    mClass = nullptr;
    return VisitResult::Stop;
  }
  
  virtual VisitResult Visit(FunctionNode* node)
  {
    node->mParent = mClass;
    mFunction = node;
    Walk(node->mParameters);
    TryWalk(node->mScope);
    TryWalk(node->mReturnType);
    
    std::vector<Type*> paramTypes;
    paramTypes.resize(node->mParameters.size());
    for (size_t i = 0; i < node->mParameters.size(); ++i)
      paramTypes[i] = node->mParameters[i]->mSymbol->mType;

    Type* returnType = nullptr;
    if (node->mReturnType != nullptr)
      returnType = node->mReturnType->mSymbol;
    else
      returnType = VoidType;

    node->mSignatureType = mLibrary->GetFunctionType(paramTypes, returnType);

    return VisitResult::Stop;
  }


  virtual VisitResult Visit(ParameterNode* node)
  {
    TryWalk(node->mType);
    node->mParent = mFunction;
    Variable* variableSymbol = mLibrary->CreateVariable(node->mName.str());
    node->mSymbol = variableSymbol;
    if (node->mType->mSymbol == nullptr)
      __debugbreak();
    variableSymbol->mType = node->mType->mSymbol;
    variableSymbol->mIsParameter = true;
    variableSymbol->mParentFunction = mFunction->mSymbol;
    return VisitResult::Stop;
  }

  void HandleIndirectionType(TypeNode* node, TypeNode* typeToNode, const char* extraName)
  {
    TryWalk(typeToNode);

    Type* typeTo = typeToNode->mSymbol;
    ErrorIf(typeTo == nullptr, "");
    Type* symbol = symbol  = mLibrary->GetPointerType(typeTo);
    //std::string typeName = parentSymbol->mName + extraName;
    //Type* symbol = LibraryHelpers::FindGlobalSymbolByName<Type>(mLibrary, typeName);
    //if (symbol == nullptr)
    //  symbol = mLibrary->CreateType(typeName, true);
    node->mSymbol = symbol;
  }
  
  virtual VisitResult Visit(PointerTypeNode* node)
  {
    HandleIndirectionType(node, node->mPointerTo.get(), "*");
    return VisitResult::Continue;
  }

  virtual VisitResult Visit(ReferenceTypeNode* node)
  {
    HandleIndirectionType(node, node->mReferenceTo.get(), "&");
    return VisitResult::Continue;
  }

  virtual VisitResult Visit(NamedTypeNode* node)
  {
    const std::string typeName = node->mName.str();
    node->mSymbol = LibraryHelpers::FindGlobalSymbolByName<Type>(mLibrary, typeName);
    if (node->mSymbol == nullptr)
      ErrorSymbolNotFound(typeName);
    return VisitResult::Stop;
  }

  Library* mLibrary = nullptr;
  BlockNode* mBlock = nullptr;
  ClassNode* mClass = nullptr;
  FunctionNode* mFunction = nullptr;
};
