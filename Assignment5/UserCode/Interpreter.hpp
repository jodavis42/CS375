#pragma once

#include "Visitor.hpp"
#include "../Drivers/Driver4.hpp"
#include "InterpreterPrePass.hpp"
#include "VariableStack.hpp"

class ReturnException : public std::exception
{
public:
  ReturnException() {}
  ReturnException(Variant variant)
  {
    mVariant = variant;
  }
  Variant mVariant;
};

#define DefineVariantBinaryArithmeticOp(op)                       \
inline Variant operator op(const Variant& lhs, const Variant& rhs)          \
{                                                                           \
  Variant result;                                                           \
  result.mType = lhs.mType;                                                 \
  if (lhs.mType == ByteType)                                                \
    result.mValue.mByte = lhs.mValue.mByte op rhs.mValue.mByte;             \
  else if (lhs.mType == IntegerType)                                        \
    result.mValue.mInteger = lhs.mValue.mInteger op rhs.mValue.mInteger;    \
  else if (lhs.mType == FloatType)                                          \
    result.mValue.mFloat = lhs.mValue.mFloat op rhs.mValue.mFloat;          \
  return result;                                                            \
}

#define DefineVariantBinaryLogicalOp(op)                                  \
inline Variant operator op(const Variant& lhs, const Variant& rhs)        \
{                                                                         \
  Variant result;                                                         \
  result.mType = BooleanType;                                             \
  if (lhs.mType == ByteType)                                              \
    result.mValue.mBoolean = lhs.mValue.mByte op rhs.mValue.mByte;        \
  else if (lhs.mType == IntegerType)                                      \
    result.mValue.mBoolean = lhs.mValue.mInteger op rhs.mValue.mInteger;  \
  else if (lhs.mType == FloatType)                                        \
    result.mValue.mBoolean = lhs.mValue.mFloat op rhs.mValue.mFloat;      \
  return result;                                                          \
}

DefineVariantBinaryArithmeticOp(+)
DefineVariantBinaryArithmeticOp(-)
DefineVariantBinaryArithmeticOp(*)
DefineVariantBinaryArithmeticOp(/)
DefineVariantBinaryLogicalOp(<)
DefineVariantBinaryLogicalOp(<=)
DefineVariantBinaryLogicalOp(>)
DefineVariantBinaryLogicalOp(>=)
DefineVariantBinaryLogicalOp(==)
DefineVariantBinaryLogicalOp(!=)

#undef DefineVariantBinaryArithmeticOp
#undef DefineVariantBinaryLogicalOp

inline Variant BuildVariant(bool value)
{
  Variant result;
  result.mType = BooleanType;
  result.mValue.mBoolean = value;
  return result;
}

inline Variant BuildVariant(char value)
{
  Variant result;
  result.mType = ByteType;
  result.mValue.mByte = value;
  return result;
}

inline Variant BuildVariant(int value)
{
  Variant result;
  result.mType = IntegerType;
  result.mValue.mInteger = value;
  return result;
}

inline Variant BuildVariant(float value)
{
  Variant result;
  result.mType = FloatType;
  result.mValue.mFloat = value;
  return result;
}

template <typename ToType>
inline Variant CastTo(Variant from)
{
  if (from.mType == ByteType)
    return BuildVariant((ToType)from.mValue.mByte);
  else if (from.mType == IntegerType)
    return BuildVariant((ToType)from.mValue.mInteger);
  else if (from.mType == FloatType)
    return BuildVariant((ToType)from.mValue.mFloat);
  else if (from.mType == BooleanType)
    return BuildVariant((ToType)from.mValue.mBoolean);
  return Variant();
}

inline Variant operator %(const Variant& lhs, const Variant& rhs)
{
  Variant result;
  result.mType = lhs.mType;
  if (lhs.mType == ByteType)
    result.mValue.mByte = lhs.mValue.mByte % rhs.mValue.mByte;
  else if (lhs.mType == IntegerType)
    result.mValue.mInteger = lhs.mValue.mInteger % rhs.mValue.mInteger;
  else if (lhs.mType == FloatType)
    result.mValue.mFloat = fmod(lhs.mValue.mFloat, rhs.mValue.mFloat);
    return result;
}

inline Variant operator &&(const Variant& lhs, const Variant& rhs)
{
  Variant result;
  result.mType = BooleanType;
  result.mValue.mBoolean = lhs.mValue.mBoolean && rhs.mValue.mBoolean;
  return result;
}

inline Variant operator ||(const Variant& lhs, const Variant& rhs)
{
  Variant result;
  result.mType = BooleanType;
  result.mValue.mBoolean = lhs.mValue.mBoolean || rhs.mValue.mBoolean;
  return result;
}


class Interpreter : public Visitor
{
public:
  Interpreter()
  {
  }

  virtual VisitResult Visit(FunctionNode* node) override
  {
    TryWalk(node->mScope);
    return VisitResult::Stop;

  }
  virtual VisitResult Visit(ReturnNode* node) override
  {
    TryWalk(node->mReturnValue);
    throw ReturnException(mResult);
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(LiteralNode* node) override
  {
    mResult = node->mValue;
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(VariableNode* node) override
  {
    Variant variant;
    variant.mType = node->mSymbol->mType;
    if(node->mInitialValue)
      variant = WalkAndGetResult(node->mInitialValue);
    mFunctionStack.Add(node->mSymbol, variant);
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(NameReferenceNode* node) override
  {
    const std::string& name = node->mName.str();
    Function* function = dynamic_cast<Function*>(node->mSymbol);

    // Find a variable, first checking the function locals then the glboals
    Variant* variable = mFunctionStack.Find(node->mSymbol);
    if(variable == nullptr)
      variable = mGlobalStack.Find(node->mSymbol);
    if (variable != nullptr)
    {
      mResult = *variable;
      mResult.mPointerToValue = &variable->mValue;
    }
    else if (function != nullptr)
    {
      mResult.mType = function->mType;
      mResult.mValue.mFunction = function;
    }
    // Assume this is a global that we haven't seen yet. Create the global.
    else
    {
      Variant globalVariant;
      globalVariant.mType = node->mSymbol->mType;
      mGlobalStack.Add(node->mSymbol, globalVariant);
      Variant* globalVariable = mGlobalStack.Find(node->mSymbol);
      mResult = *globalVariable;
      mResult.mPointerToValue = &globalVariable->mValue;
    }
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(CallNode* node) override
  {
    std::vector<Variant> arguments;
    for (size_t i = 0; i < node->mArguments.size(); ++i)
      arguments.push_back(WalkAndGetResult(node->mArguments[i]));
    Variant fnVariant = WalkAndGetResult(node->mLeft);
    mResult = fnVariant.mValue.mFunction->mCallback(this, fnVariant.mValue.mFunction, arguments);

    return VisitResult::Stop;
  }

  virtual VisitResult Visit(BinaryOperatorNode* node) override
  {
    switch (node->mOperator.mEnumTokenType)
    {
    case TokenType::Assignment:
      {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
        *(VariantValue*)lhs.mPointerToValue = rhs.mValue;
        lhs.mValue = rhs.mValue;
        mResult = lhs;
        break;
      }
    case TokenType::Plus:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs + rhs;
      break;
    }
    case TokenType::Minus:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs - rhs;
      break;
    }
    case TokenType::Asterisk:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs * rhs;
      break;
    }
    case TokenType::Divide:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs / rhs;
      break;
    }
    case TokenType::Modulo:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs % rhs;
      break;
    }
    case TokenType::LessThan:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs < rhs;
      break;
    }
    case TokenType::LessThanOrEqualTo:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs <= rhs;
      break;
    }
    case TokenType::GreaterThan:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs > rhs;
      break;
    }
    case TokenType::GreaterThanOrEqualTo:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs >= rhs;
      break;
    }
    case TokenType::Equality:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs == rhs;
      break;
    }
    case TokenType::Inequality:
    {
      Variant lhs = WalkAndGetResult(node->mLeft);
      Variant rhs = WalkAndGetResult(node->mRight);
      mResult = lhs != rhs;
      break;
    }
    case TokenType::LogicalAnd:
    {
      mResult = WalkAndGetResult(node->mLeft);
      // Make sure to do short circuit evalulation
      if (mResult.mValue.mBoolean)
        mResult = mResult && WalkAndGetResult(node->mRight);
      break;
    }
    case TokenType::LogicalOr:
    {
      mResult = WalkAndGetResult(node->mLeft);
      // Make sure to do short circuit evalulation
      if(!mResult.mValue.mBoolean)
        mResult = mResult || WalkAndGetResult(node->mRight);
      break;
    }
    default:
      mResult = Variant();
      break;
    }
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(CastNode* node) override
  {
    Variant value = WalkAndGetResult(node->mLeft);
    Type* toType = node->mResolvedType;
    if (toType == ByteType)
      mResult = CastTo<char>(value);
    else if (toType == IntegerType)
      mResult = CastTo<int>(value);
    else if (toType == FloatType)
      mResult = CastTo<float>(value);
    else if (toType == BooleanType)
      mResult = CastTo<bool>(value);
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(IfNode* node) override
  {
    if (node->mCondition != nullptr)
    {
      Variant condition = WalkAndGetResult(node->mCondition);
      if (condition.mValue.mBoolean)
      {
        TryWalk(node->mScope);
      }
      else
      {
        TryWalk(node->mElse);
      }
    }
    else
    {
      TryWalk(node->mScope);
    }
    
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(WhileNode* node) override
  {
    if (node->mCondition != nullptr)
    {
      while (WalkAndGetResult(node->mCondition).mValue.mBoolean)
      {
        TryWalk(node->mScope);
      }
    }

    return VisitResult::Stop;
  }

  virtual VisitResult Visit(ScopeNode* node) override
  {
    mFunctionStack.BeginScope();
    Walk(node->mStatements);
    mFunctionStack.EndScope();
    return VisitResult::Stop;
  }

  template <typename NodeType> Variant WalkAndGetResult(std::unique_ptr<NodeType>& node)
  {
    return WalkAndGetResult(node.get());
  }

  template <typename NodeType>
  Variant WalkAndGetResult(NodeType* node)
  {
    node->Walk(this);
    return mResult;
  }

  Variant mResult;
  VariableStack mLocalVariables;
  FunctionLocalsStack mFunctionStack;
  StackFrame mGlobalStack;
};
