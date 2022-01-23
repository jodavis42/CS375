#pragma once

#include "Visitor.hpp"
#include "../Drivers/Driver4.hpp"

class TypeVisitor : public Visitor
{
public:
  
  TypeVisitor(Library* library)
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
    Type* type = mLibrary->CreateType(node->mName.str(), true);
    node->mSymbol = type;
    node->mParent = mBlock;
    return VisitResult::Continue;
  }

  BlockNode* mBlock = nullptr;
  Library* mLibrary = nullptr;
};
