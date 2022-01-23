#pragma once

#include "Visitor.hpp"
#include "../Drivers/Driver4.hpp"

class InterpreterPrePassVisitor : public Visitor
{
public:
  InterpreterPrePassVisitor(FunctionCallback fnCallback)
  {
    mFnCallback = fnCallback;
  }

  virtual VisitResult Visit(FunctionNode* node) override
  {
    node->mSymbol->mCallback = mFnCallback;
    node->mSymbol->mFunctionNode = node;
    return VisitResult::Continue;
  }

  static std::string ParseLiteral(const std::string& str)
  {
    std::string result;
    for (size_t i = 1; i < str.size() - 1; ++i)
    {
      char byte = 0;
      if (str[i] == '\\')
      {
        ++i;
        if (str[i] == 'r')
          byte = '\r';
        else if (str[i] == 'n')
          byte = '\n';
        else if (str[i] == 't')
          byte = '\t';
        else if (str[i] == '\"')
          byte = '\'';
        else if (str[i] == '\"')
          byte = '\"';
      }
      else
        byte = str[i];
      result.push_back(byte);
    }
    return result;
  }

  virtual VisitResult Visit(LiteralNode* node) override
  {
    const std::string& str = node->mToken.str();
    Variant variant;
    if (node->mToken.mEnumTokenType == TokenType::IntegerLiteral)
    {
      variant.mType = IntegerType;
      variant.mValue.mInteger = atoi(str.c_str());
    }
    else if (node->mToken.mEnumTokenType == TokenType::FloatLiteral)
    {
      variant.mType = FloatType;
      variant.mValue.mFloat = (float)atof(str.c_str());
    }
    else if (node->mToken.mEnumTokenType == TokenType::True)
    {
      variant.mType = BooleanType;
      variant.mValue.mBoolean = true;
    }
    else if (node->mToken.mEnumTokenType == TokenType::False)
    {
      variant.mType = BooleanType;
      variant.mValue.mBoolean = false;
    }
    else if (node->mToken.mEnumTokenType == TokenType::Null)
      node->mResolvedType = mLibrary->GetPointerType(NullType);
    else if (node->mToken.mEnumTokenType == TokenType::CharacterLiteral)
    {
      variant.mType = ByteType;
      node->mString = ParseLiteral(str);
      variant.mValue.mByte = node->mString[0];
    }
    else if (node->mToken.mEnumTokenType == TokenType::StringLiteral)
    {
      variant.mType = BytePointerType;
      node->mString = ParseLiteral(str);
      variant.mValue.mPointer = &node->mString[0];
    }
    else
    {
      ErrorIf(true, "");
    }
    node->mValue = variant;
    return VisitResult::Stop;
  }

  FunctionCallback mFnCallback = nullptr;
  Library* mLibrary = nullptr;
};
