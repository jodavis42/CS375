#pragma once

#include "Visitor.hpp"
#include "../Drivers/Driver4.hpp"
#include "LibraryHelpers.hpp"

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

class ExpresionResolverVisitor : public Visitor
{
public:
  ExpresionResolverVisitor(Library* library)
  {
    mLibrary = library;
  }

  virtual VisitResult Visit(BlockNode* node) override
  {
    mBlock = node;
    mSymbolStack.Push();
    Walk(mBlock->mGlobals);
    mSymbolStack.Pop();
    mBlock = nullptr;
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(ClassNode* node) override
  {
    mClass = node;
    mSymbolStack.Push();
    //for(size_t i = 0; i < node->mMembers.size(); ++i)
    //  mSymbolStack.Add(node->mMembers[i])
    Walk(node->mMembers);
    mSymbolStack.Pop();
    mClass = nullptr;
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(FunctionNode* node) override
  {
    mFunction = node;
    mSymbolStack.Add(node->mSymbol);
    mSymbolStack.Push();
    Walk(node->mParameters);
    TryWalk(node->mReturnType);
    TryWalk(node->mScope);
    mSymbolStack.Pop();

    mFunction = nullptr;

    return VisitResult::Stop;
  }

  virtual VisitResult Visit(ParameterNode* node) override
  {
    const std::string& varName = node->mName.str();
    //Variable* varSymbol = mLibrary->CreateVariable(varName, false);
    mSymbolStack.Add(node->mSymbol);
    TryWalk(node->mInitialValue);
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(VariableNode* node) override
  {
    const std::string& varName = node->mName.str();
    //Variable* varSymbol = mLibrary->CreateVariable(varName, false);

    mSymbolStack.Add(node->mSymbol);
    TryWalk(node->mInitialValue);
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(LiteralNode* node) override
  {
    if (node->mToken.mEnumTokenType == TokenType::IntegerLiteral)
      node->mResolvedType = IntegerType;
    else if (node->mToken.mEnumTokenType == TokenType::FloatLiteral)
      node->mResolvedType = FloatType;
    else if (node->mToken.mEnumTokenType == TokenType::StringLiteral)
      node->mResolvedType = BytePointerType;
    else if (node->mToken.mEnumTokenType == TokenType::CharacterLiteral)
      node->mResolvedType = ByteType;
    else if (node->mToken.mEnumTokenType == TokenType::True)
      node->mResolvedType = BooleanType;
    else if (node->mToken.mEnumTokenType == TokenType::False)
      node->mResolvedType = BooleanType;
    else if (node->mToken.mEnumTokenType == TokenType::Null)
      node->mResolvedType = mLibrary->GetPointerType(NullType);
    else
    {
      ErrorIf(true, "");
    }
    //else
    //  node->mResolvedType = ErrorTypeMismatch
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(NameReferenceNode* node) override
  {
    const std::string& name = node->mName.str();
    node->mSymbol = mSymbolStack.Find(name);
    if(node->mSymbol == nullptr)
      ErrorSymbolNotFound(name);
    node->mResolvedType = node->mSymbol->mType;
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(IndexNode* node) override
  {
    TryWalk(node->mLeft);
    TryWalk(node->mIndex);
    Type* baseType = node->mLeft->mResolvedType;
    if (baseType->mMode != TypeMode::Pointer)
      ErrorInvalidIndexer(node);

    node->mResolvedType = baseType->mPointerToType;
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(MemberAccessNode* node) override
  {
    TryWalk(node->mLeft);

    const std::string& name = node->mName.str();
    Type* leftType = node->mLeft->mResolvedType;
    Type* derefType = leftType;
    //std::unordered_map<std::string, Symbol*>* membersByName = nullptr;
    if (node->mOperator.mTokenType == TokenType::Dot)
    {
      if (leftType->mMode != TypeMode::Class)
        ErrorInvalidMemberAccess(node);
    }
    else if (node->mOperator.mTokenType == TokenType::Arrow)
    {
      if (leftType->mMode != TypeMode::Pointer)
        ErrorInvalidMemberAccess(node);
      derefType = leftType->mPointerToType;
    }
    auto&& it = derefType->mMembersByName.find(name);
    if (it == derefType->mMembersByName.end())
      ErrorInvalidMemberAccess(node);

    node->mResolvedMember = it->second;
    node->mResolvedType = node->mResolvedMember->mType;
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(BinaryOperatorNode* node) override
  {
    TryWalk(node->mLeft);
    TryWalk(node->mRight);
    Type* leftType = node->mLeft->mResolvedType;
    Type* rightType = node->mRight->mResolvedType;

    switch (node->mOperator.mTokenType)
    {
    case TokenType::Plus:
    {
      if ((leftType == ByteType && rightType == ByteType) ||
          (leftType == IntegerType && rightType == IntegerType) ||
          (leftType == FloatType && rightType == FloatType))//leftType == IntegerType && leftType == FloatType)
        node->mResolvedType = leftType;
      else if(leftType == IntegerType && rightType->mMode ==TypeMode::Pointer)
        node->mResolvedType = rightType;
      else if (leftType->mMode == TypeMode::Pointer && rightType == IntegerType)
        node->mResolvedType = leftType;
      else
        ErrorInvalidBinaryOperator(node);
      break;
    }
    case TokenType::Minus:
    {
      if ((leftType == ByteType && rightType == ByteType) ||
        (leftType == IntegerType && rightType == IntegerType) ||
        (leftType == FloatType && rightType == FloatType))
        node->mResolvedType = leftType;
      else if (leftType == IntegerType && rightType->mMode == TypeMode::Pointer)
        node->mResolvedType = rightType;
      else if (leftType->mMode == TypeMode::Pointer && rightType == IntegerType)
        node->mResolvedType = leftType;
      else if (leftType->mMode == TypeMode::Pointer && rightType->mMode == TypeMode::Pointer && leftType == rightType)
        node->mResolvedType = IntegerType;
      else
        ErrorInvalidBinaryOperator(node);
      break;
    }
    case TokenType::Asterisk:
    case TokenType::Divide:
    case TokenType::Modulo:
    {
      if (leftType == rightType && (leftType == ByteType || leftType == IntegerType || leftType == FloatType))
        node->mResolvedType = leftType;
      else
        ErrorInvalidBinaryOperator(node);
      break;
    }
    case TokenType::GreaterThan:
    case TokenType::GreaterThanOrEqualTo:
    case TokenType::LessThan:
    case TokenType::LessThanOrEqualTo:
    case TokenType::Equality:
    case TokenType::Inequality:
    {
      node->mResolvedType = BooleanType;
      break;
    }
    case TokenType::LogicalAnd:
    case TokenType::LogicalOr:
    {
      if ((leftType == BooleanType || leftType->mMode == TypeMode::Pointer) && (rightType == BooleanType || rightType->mMode == TypeMode::Pointer))
        node->mResolvedType = BooleanType;
      else
        ErrorInvalidBinaryOperator(node);
      break;
    }
    case TokenType::Assignment:
    {
      if (leftType != rightType || leftType->mMode == TypeMode::Function)
        ErrorInvalidBinaryOperator(node);
      node->mResolvedType = rightType;
      break;
    }
    default:
      ErrorInvalidBinaryOperator(node);
    }
    
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(UnaryOperatorNode* node) override
  {
    TryWalk(node->mRight);
    Type* rightType = node->mRight->mResolvedType;
    switch (node->mOperator.mTokenType)
    {
    case TokenType::Asterisk:
    {
      if (rightType->mMode != TypeMode::Pointer)
        ErrorInvalidUnaryOperator(node);
      node->mResolvedType = rightType->mPointerToType;
      break;
    }
    case TokenType::Plus:
    case TokenType::Minus:
    {
      if (rightType != ByteType && rightType != IntegerType && rightType != FloatType)
        ErrorInvalidUnaryOperator(node);
      node->mResolvedType = rightType;
      break;
    }
    case TokenType::Increment:
    case TokenType::Decrement:
    {
      if (rightType != ByteType && rightType != IntegerType && rightType != FloatType && rightType->mMode != TypeMode::Pointer)
        ErrorInvalidUnaryOperator(node);
      node->mResolvedType = rightType;
      break;
    }
    case TokenType::Not:
    {
      if (rightType != ByteType && rightType->mMode != TypeMode::Pointer)
        ErrorInvalidUnaryOperator(node);
      node->mResolvedType = BooleanType;
      break;
    }
    case TokenType::LogicalNot:
    {
      if (rightType != BooleanType && rightType != IntegerType && rightType->mMode != TypeMode::Pointer)
        ErrorInvalidUnaryOperator(node);
      node->mResolvedType = BooleanType;
      break;
    }
    case TokenType::Ampersand:
    {
      //if (rightType != ByteType && rightType->mMode != TypeMode::Pointer)
      //  ErrorInvalidUnaryOperator(node);
      node->mResolvedType = mLibrary->GetPointerType(rightType);
      break;
    }
    default:
      ErrorInvalidUnaryOperator(node);
    }
    return VisitResult::Stop;
  }

  /*virtual VisitResult Visit(IndexNode* node) override
  {
    TryWalk(node->mLeft);
    ErrorIf(node->mLeft->mResolvedType->mPointerToType == nullptr);
    node->mResolvedType = node->mLeft->mResolvedType->mPointerToType;
    return VisitResult::Stop;
  }*/

  virtual VisitResult Visit(CallNode* node) override
  {
    TryWalk(node->mLeft);
    Walk(node->mArguments);

    Type* fnType = node->mLeft->mResolvedType;
    if (fnType->mMode != TypeMode::Function)
      ErrorInvalidCall(node);
    if(node->mArguments.size() != fnType->mParameterTypes.size())
      ErrorInvalidCall(node);
    for (size_t i = 0; i < node->mArguments.size(); ++i)
    {
      if (node->mArguments[i]->mResolvedType != fnType->mParameterTypes[i])
        ErrorInvalidCall(node);
    }
    
    node->mResolvedType = node->mLeft->mResolvedType->mReturnType;
    
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(CastNode* node) override
  {
    TryWalk(node->mLeft);
    Type* castType = node->mType->mSymbol;
    Type* leftType = node->mLeft->mResolvedType;
    bool castInt = castType == IntegerType;
    bool castBool = castType == BooleanType;
    bool castFloat = castType == FloatType;
    bool castByte = castType == ByteType;
    bool castPointer = castType->mMode == TypeMode::Pointer;
    bool leftInt = leftType == IntegerType;
    bool leftBool = leftType == BooleanType;
    bool leftByte = leftType == ByteType;
    bool leftFloat = leftType == FloatType;
    bool leftPointer = leftType->mMode == TypeMode::Pointer;


    node->mResolvedType = castType;
    if((castInt || castBool || castFloat || castByte) && (leftInt || leftBool || leftFloat || leftByte))
    {
      node->mResolvedType = castType;
    }
    else if (castPointer && leftPointer)
    {
      node->mResolvedType = castType;
    }
    else if ((castInt && leftPointer) || (castPointer && leftInt))
    {
      node->mResolvedType = castType;
    }
    else if ((castBool && leftPointer))
    {

      node->mResolvedType = castType;
    }
    else 
      ErrorInvalidCast(leftType, castType);
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(GotoNode* node) override
  {
    const std::string& labelName = node->mName.str();
    auto&& labelsByName = mFunction->mSymbol->mLabelsByName;
    auto&& it = labelsByName.find(labelName);
    if (it == labelsByName.end())
      ErrorGotoLabelNotFound(labelName);
    node->mResolvedLabel = it->second;
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(ReturnNode* node) override
  {
    TryWalk(node->mReturnValue);

    Type* fnType = mFunction->mSymbol->mType;
    Type* fnReturnType = fnType->mReturnType;
    //Type* fnReturnType = mFunction->mReturnType->mSymbol;
    if (node->mReturnValue == nullptr && fnReturnType != VoidType)
      ErrorTypeMismatch(fnReturnType, nullptr);

    Type* returnType = node->mReturnValue->mResolvedType;
    if (fnReturnType != returnType)
      ErrorTypeMismatch(fnReturnType, returnType);
    
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(ScopeNode* node) override
  {
    mSymbolStack.Push();
    Walk(node->mStatements);
    mSymbolStack.Pop();
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(IfNode* node) override
  {
    TryWalk(node->mCondition);
    TryWalk(node->mScope);
    TryWalk(node->mElse);
    if (node->mCondition != nullptr)
    {
      Type* conditionType = node->mCondition->mResolvedType;
      if (conditionType != BooleanType && conditionType->mMode != TypeMode::Pointer)
        ErrorConditionExpectedBooleanOrPointer(conditionType);
    }
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(WhileNode* node) override
  {
    TryWalk(node->mCondition);
    mInLoop = true;
    TryWalk(node->mScope);
    mInLoop = false;
    return VisitResult::Stop;
  }


  virtual VisitResult Visit(ForNode* node) override
  {
    TryWalk(node->mInitialVariable);
    TryWalk(node->mInitialExpression);
    TryWalk(node->mCondition);
    mInLoop = true;
    TryWalk(node->mScope);
    mInLoop = false;
    TryWalk(node->mIterator);
    
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(BreakNode* node) override
  {
    if (!mInLoop)
      ErrorBreakContinueMustBeInsideLoop();
    return VisitResult::Stop;
  }

  virtual VisitResult Visit(ContinueNode* node) override
  {
    if (!mInLoop)
      ErrorBreakContinueMustBeInsideLoop();
    return VisitResult::Stop;
  }


  Library* mLibrary = nullptr;
  BlockNode* mBlock = nullptr;
  ClassNode* mClass = nullptr;
  FunctionNode* mFunction = nullptr;
  SymbolStack mSymbolStack;
  bool mInLoop = false;
};
