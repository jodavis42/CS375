#pragma once

#include "../Drivers/Driver4.hpp"

class LibraryHelpers
{
public:
  template <typename SymbolType = Symbol>
  static SymbolType* FindGlobalSymbolByName(Library* library, const std::string& name)
  {
    auto&& it = library->mGlobalsByName.find(name);
    if (it != library->mGlobalsByName.end())
      return static_cast<SymbolType*>(it->second);
    return nullptr;
  }
};
