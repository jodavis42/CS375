#pragma once

#include "Visitor.hpp"
#include "../Drivers/Driver4.hpp"
#include "LibraryHelpers.hpp"

class MemberResolverVisitor : public Visitor
{
public:
  
  MemberResolverVisitor(Library* library)
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

  virtual VisitResult Visit(ParameterNode* node)
  {
    mFunction->mSymbol->mLocals.push_back(node->mSymbol);
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(VariableNode* node)
  {
    bool isGlobal = mFunction == nullptr && mClass == nullptr;
    node->mParent = mFunction;
    Variable* variableSymbol = mLibrary->CreateVariable(node->mName.str(), isGlobal);
    node->mSymbol = variableSymbol;
    variableSymbol->mType = node->mType->mSymbol;
    variableSymbol->mIsParameter = false;
    if (mFunction != nullptr)
    {
      variableSymbol->mParentFunction = mFunction->mSymbol;
      mFunction->mSymbol->mLocals.push_back(variableSymbol);
    }
    else if(mClass != nullptr)
    {
      variableSymbol->mParentType = mClass->mSymbol;
      if (mClass->mSymbol->mMembersByName.find(variableSymbol->mName) != mClass->mSymbol->mMembersByName.end())
        ErrorSameName(variableSymbol->mName);
      mClass->mSymbol->mMembers.push_back(variableSymbol);
      mClass->mSymbol->mMembersByName.insert(std::make_pair(variableSymbol->mName, variableSymbol));
    }
    else
    {
      // Global variable
    }

    return VisitResult::Stop;
  }

  virtual VisitResult Visit(FunctionNode* node)
  {
    mFunction = node;

    const std::string& name = node->mName.str();
    Function* functionSymbol = mLibrary->CreateFunction(name, mClass == nullptr);
    node->mSymbol = functionSymbol;
    functionSymbol->mFunctionNode = node;
    functionSymbol->mType = node->mSignatureType;

    if (mClass != nullptr)
    {
      functionSymbol->mParentType = mClass->mSymbol;
      mClass->mSymbol->mMembers.push_back(functionSymbol);
      mClass->mSymbol->mMembersByName.insert(std::make_pair(name, functionSymbol));
    }

    Walk(node->mParameters);
    TryWalk(node->mReturnType);
    TryWalk(node->mScope);

    mFunction = nullptr;

    return VisitResult::Stop;
  }

  virtual VisitResult Visit(LabelNode* node)
  {
    bool isGlobal = (mFunction == nullptr && mClass == nullptr);
    Label* labelSymbol = mLibrary->CreateLabel(node->mName.str(), isGlobal);
    node->mSymbol = labelSymbol;
    node->mParent = mFunction;

    auto&& labelsByName = mFunction->mSymbol->mLabelsByName;

    if (labelsByName.find(labelSymbol->mName) != labelsByName.end())
      ErrorSameName(labelSymbol->mName);

    mFunction->mSymbol->mLabelsByName.insert(std::make_pair(labelSymbol->mName, labelSymbol));
    mFunction->mSymbol->mLocals.push_back(labelSymbol);
    return VisitResult::Stop;
  }

  Library* mLibrary = nullptr;
  BlockNode* mBlock = nullptr;
  ClassNode* mClass = nullptr;
  FunctionNode* mFunction = nullptr;
};
