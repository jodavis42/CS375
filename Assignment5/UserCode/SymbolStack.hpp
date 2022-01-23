#pragma once

#include "../Drivers/Driver4.hpp"

class SymbolStack
{
public:
  class SymbolTable
  {
  public:
    void Add(Symbol* symbol)
    {
      Add(symbol->mName, symbol);
    }

    void Add(const std::string& name, Symbol* symbol)
    {
      mSymbols.insert(std::make_pair(name, symbol));
    }

    Symbol* Find(const std::string& name)
    {
      auto&& it = mSymbols.find(name);
      if (it != mSymbols.end())
        return it->second;
      return nullptr;
    }
    std::unordered_map<std::string, Symbol*> mSymbols;
  };

  SymbolStack()
  {
    mSymbols.resize(1);
  }

  void Push()
  {
    mSymbols.resize(mSymbols.size() + 1);
  }

  void Pop()
  {
    mSymbols.pop_back();
  }

  void Add(Symbol* symbol)
  {
    Add(symbol->mName, symbol);
  }

  void Add(const std::string& name, Symbol* symbol)
  {
    mSymbols.back().Add(name, symbol);
  }

  Symbol* Find(const std::string& name)
  {
    for (size_t i = 0; i < mSymbols.size(); ++i)
    {
      auto&& table = mSymbols[mSymbols.size() - i - 1];
      Symbol* symbol = table.Find(name);
      if (symbol != nullptr)
        return symbol;
    }
    return nullptr;
  }

  std::vector<SymbolTable> mSymbols;
};