#pragma once

#include "../Drivers/Driver5.hpp"

class StackFrame
{
public:
  void Add(const Symbol* symbol, const Variant& variant)
  {
    mSymbols.insert(std::make_pair(symbol, variant));
  }

  Variant Find(const Symbol* symbol, const Variant& default)
  {
    auto&& it = mSymbols.find(symbol);
    if (it != mSymbols.end())
      return it->second;
    return default;
  }

  Variant* Find(const Symbol* symbol)
  {
    auto&& it = mSymbols.find(symbol);
    if (it != mSymbols.end())
      return &it->second;
    return nullptr;
  }

  std::unordered_map<const Symbol*, Variant> mSymbols;
};

class VariableStack
{
public:
  VariableStack()
  {
    mStackFrames.resize(1);
  }

  void Push()
  {
    mStackFrames.resize(mStackFrames.size() + 1);
  }

  void Pop()
  {
    mStackFrames.pop_back();
  }

  void Add(const Symbol* symbol, const Variant& variant)
  {
    mStackFrames.back().Add(symbol, variant);
  }

  Variant* Find(const Symbol* symbol)
  {
    for (size_t i = 0; i < mStackFrames.size(); ++i)
    {
      auto&& table = mStackFrames[mStackFrames.size() - i - 1];
      Variant* variant = table.Find(symbol);
      if (variant != nullptr)
        return variant;
    }
    return nullptr;
  }

  std::vector<StackFrame> mStackFrames;
};

class FunctionLocalsStack
{
public:
  struct FunctionScope
  {
    Function* mFunction = nullptr;
    VariableStack mVariableStack;
  };

  FunctionLocalsStack()
  {
    mFunctionStack.resize(1);
  }

  void BeginFunction(Function* function)
  {
    mFunctionStack.resize(mFunctionStack.size() + 1);
    mFunctionStack.back().mFunction = function;
  }

  void EndFunction()
  {
    mFunctionStack.pop_back();
  }

  FunctionScope& CurrentScope()
  {
    return mFunctionStack.back();
  }

  VariableStack& CurrentVariableStack()
  {
    return CurrentScope().mVariableStack;
  }

  void BeginScope()
  {
    CurrentVariableStack().Push();
  }

  void EndScope()
  {
    CurrentVariableStack().Pop();
  }

  void Add(const Symbol* symbol, const Variant& variant)
  {
    CurrentVariableStack().Add(symbol, variant);
  }

  Variant* Find(const Symbol* symbol)
  {
    return CurrentVariableStack().Find(symbol);
  }
 
  std::vector<FunctionScope> mFunctionStack;
};
