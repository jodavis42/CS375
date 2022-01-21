/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver3.hpp"

#include "../Drivers/AstNodes.hpp"

#include <algorithm>

class Parser
{
public:
  void Load(std::vector<Token>& tokens)
  {
    mTokens = tokens;
    mTokenIndex = 0;
  }

  void ReadTokens()
  {
    Block();
    Expect(mTokenIndex == mTokens.size());
  }

  std::unique_ptr<BlockNode> Block()
  {
    //Block               = (Class | Function | Var <Semicolon>)*
    PrintRule rule("Block");
    auto&& result = std::make_unique<BlockNode>();

    while (true)
    {
      if (auto&& node = Class())
        result->mGlobals.push_back(std::move(node));
      else if (auto&& node = Function())
        result->mGlobals.push_back(std::move(node));
      else if (auto&& node = Var())
      {
        Expect(TokenType::Semicolon);
        result->mGlobals.push_back(std::move(node));
      }
      else
        break;
    }
    return rule.Accept(std::move(result));
  }

  std::unique_ptr<ClassNode> Class()
  {
    //Class               = <Class> <Identifier> <OpenCurley> (Var <Semicolon> | Function)* <CloseCurley>
    PrintRule rule("Class");
    if (this->Accept(TokenType::Class) == false)
      return nullptr;

    auto&& result = std::make_unique<ClassNode>();
    this->Expect(TokenType::Identifier, &result->mName);
    this->Expect(TokenType::OpenCurley);

    while (true)
    {
      if (auto&& node = Var())
      {
        Expect(TokenType::Semicolon);
        result->mMembers.push_back(std::move(node));
      }
      else if (auto&& node = Function())
        result->mMembers.push_back(std::move(node));
      else
        break;
    }

    this->Expect(TokenType::CloseCurley);
    return rule.Accept(std::move(result));
  }

  std::unique_ptr<VariableNode> Var()
  {
    //Var                 = <Var> <Identifier> SpecifiedType (<Assignment> Expression)?
    PrintRule rule("Var");
    if (!Accept(TokenType::Var))
      return nullptr;

    auto&& result = std::make_unique<VariableNode>();
    Expect(TokenType::Identifier, &result->mName);
    result->mType = Expect(SpecifiedType());
    if(Accept(TokenType::Assignment))
      result->mInitialValue = Expect(Expression());

    return rule.Accept(std::move(result));
  }

  bool Var(std::unique_ptr<VariableNode>& result)
  {
    result = Var();
    return result != nullptr;
  }

  std::unique_ptr<FunctionNode> Function()
  {
    //Function            = <Function> <Identifier> <OpenPareantheses> (Parameter (<Comma> Parameter)*)? <CloseParentheses> SpecifiedType? Scope
    PrintRule rule("Function");
    if (!Accept(TokenType::Function))
      return nullptr;

    auto&& node = std::make_unique<FunctionNode>();

    Expect(TokenType::Identifier, &node->mName) && Expect(TokenType::OpenParentheses);
    std::unique_ptr<ParameterNode> param;
    if (Parameter(param))
    {
      node->mParameters.push_back(std::move(param));
      while (Accept(TokenType::Comma) && Expect(Parameter(param)))
      {
        node->mParameters.push_back(std::move(param));
      };
    }

    Expect(TokenType::CloseParentheses);
    node->mReturnType = SpecifiedType();
    node->mScope = Expect(Scope());

    return rule.Accept(std::move(node));
  }
  
  bool Function(std::unique_ptr<FunctionNode>& result)
  {
    result = Function();
    return result != nullptr;
  }

  std::unique_ptr<ParameterNode> Parameter()
  {
    //Parameter           = <Identifier> SpecifiedType
    PrintRule rule("Parameter");
    Token name;
    if (!Accept(TokenType::Identifier, &name))
      return nullptr;

    auto&& result = std::make_unique<ParameterNode>();
    result->mName = name;
    result->mType = Expect(SpecifiedType());
    return rule.Accept(std::move(result));
  }

  template <typename T, std::unique_ptr<T>(Parser::* Fn)()>
  bool GetNode(std::unique_ptr<T>& result)
  {
    result = (this->*Fn)();
    return result != nullptr;
  }

  bool Parameter(std::unique_ptr<ParameterNode>& result)
  {
    result = Parameter();
    return result != nullptr;
  }

  std::unique_ptr<TypeNode> SpecifiedType()
  {
    //<Colon> Type
    PrintRule rule("SpecifiedType");
    if (!(Accept(TokenType::Colon)))
      return nullptr;
    
    return rule.Accept(std::move(Expect(Type())));
  }

  std::unique_ptr<TypeNode> Type()
  {
    //NamedType | FunctionType
    PrintRule rule("Type");

    if(auto&& result = NamedType())
      return rule.Accept(std::move(result));
    else if(auto && result = FunctionType())
      return rule.Accept(std::move(result));
    else
      return nullptr;
  }

  std::unique_ptr<TypeNode> NamedType()
  {
    //NamedType           = <Identifier> <Asterisk>* <Ampersand>?
    PrintRule rule("NamedType");
    Token token;
    if (!Accept(TokenType::Identifier, &token))
      return nullptr;

    auto&& namedTypeNode = std::make_unique<NamedTypeNode>();
    namedTypeNode->mName = token;

    std::unique_ptr<TypeNode> result = std::move(namedTypeNode);

    while (Accept(TokenType::Asterisk))
    {
      auto&& pointerNode = std::make_unique<PointerTypeNode>();
      pointerNode->mPointerTo = std::move(result);
      result = std::move(pointerNode);
    }
    if (Accept(TokenType::Ampersand))
    {
      auto&& refNode = std::make_unique<ReferenceTypeNode>();
      refNode->mReferenceTo = std::move(result);
      result = std::move(refNode);
    }

    return rule.Accept(std::move(result));
  }

  std::unique_ptr<TypeNode> FunctionType()
  {
    //FunctionType        = <Function> <Asterisk>+ <Ampersand>? <OpenParentheses> Type (<Comma> Type)* <CloseParentheses> SpecifiedType?
    PrintRule rule("FunctionType");
    if (!Accept(TokenType::Function))
      return nullptr;

    auto&& fnTypeNode = std::make_unique<FunctionTypeNode>();
    FunctionTypeNode* fnTypeNodePtr = fnTypeNode.get();
    std::unique_ptr<TypeNode> result = std::move(fnTypeNode);

    Expect(TokenType::Asterisk);
    {
      auto&& pointerNode = std::make_unique<PointerTypeNode>();
      pointerNode->mPointerTo = std::move(result);
      result = std::move(pointerNode);
    }

    while (Accept(TokenType::Asterisk))
    {
      auto&& pointerNode = std::make_unique<PointerTypeNode>();
      pointerNode->mPointerTo = std::move(result);
      result = std::move(pointerNode);
    }

    if (Accept(TokenType::Ampersand))
    {
      auto&& refNode = std::make_unique<ReferenceTypeNode>();
      refNode->mReferenceTo = std::move(result);
      result = std::move(refNode);
    }

    Expect(TokenType::OpenParentheses);
    fnTypeNodePtr->mParameters.push_back(Expect(Type()));
    while (Accept(TokenType::Comma))
    {
      fnTypeNodePtr->mParameters.push_back(Expect(Type()));
    }

    Expect(TokenType::CloseParentheses);
    fnTypeNodePtr->mReturn = SpecifiedType();

    return rule.Accept(std::move(result));
  }
  
  std::unique_ptr<ScopeNode> Scope()
  {
    //Scope               = <OpenCurley> Statement* <CloseCurley>
    PrintRule rule("Scope");
    if (!Accept(TokenType::OpenCurley))
      return nullptr;

    auto&& result = std::make_unique<ScopeNode>();
    while (auto&& node = Statement())
    {
      result->mStatements.push_back(std::move(node));
    }

    Expect(TokenType::CloseCurley);
    return rule.Accept(std::move(result));
  }
  
  std::unique_ptr<StatementNode> Statement()
  {
    //Statement           = FreeStatement | DelimitedStatement <Semicolon>
    PrintRule rule("Statement");
    if(auto&& node = FreeStatement())
      return rule.Accept(std::move(node));
    if (auto&& node = DelimitedStatement())
    {
      Expect(TokenType::Semicolon);
      return rule.Accept(std::move(node));
    }
    return nullptr;
  }
  
  std::unique_ptr<StatementNode> DelimitedStatement()
  {
    //DelimitedStatement  = Label | Goto | Return | <Break> | <Continue> | Var | Expression
    PrintRule rule("DelimitedStatement");

    if(auto&& node = Label())
      return rule.Accept(std::move(node));
    if (auto&& node = Goto())
      return rule.Accept(std::move(node));
    if (auto&& node = Return())
      return rule.Accept(std::move(node));
    if (Accept(TokenType::Break))
      return rule.Accept(std::make_unique<BreakNode>());
    if (Accept(TokenType::Continue))
      return rule.Accept(std::make_unique<ContinueNode>());
    if (auto&& node = Var())
      return rule.Accept(std::move(node));
    if (auto&& node = Expression())
      return rule.Accept(std::move(node));

    return nullptr;
  }
  
  std::unique_ptr<StatementNode> FreeStatement()
  {
    //If | While | For
    PrintRule rule("FreeStatement");

    if (auto&& node = If())
      return rule.Accept(std::move(node));
    if (auto&& node = While())
      return rule.Accept(std::move(node));
    if (auto&& node = For())
      return rule.Accept(std::move(node));

    return nullptr;
  }



  std::unique_ptr<LabelNode> Label()
  {
    //Label               = <Label> <Identifier>
    PrintRule rule("Label");
    if (!Accept(TokenType::Label))
      return nullptr;

    auto&& result = std::make_unique<LabelNode>();
    Expect(TokenType::Identifier, &result->mName);
    return rule.Accept(std::move(result));
  }

  std::unique_ptr<GotoNode> Goto()
  {
    //Goto                = <Goto> <Identifier>
    PrintRule rule("Goto");
    if (!Accept(TokenType::Goto))
      return nullptr;

    auto&& result = std::make_unique<GotoNode>();
    Expect(TokenType::Identifier, &result->mName);
    return rule.Accept(std::move(result));
  }

  std::unique_ptr<ReturnNode> Return()
  {
    //Return              = <Return> (Expression)?
    PrintRule rule("Return");
    if (!Accept(TokenType::Return))
      return nullptr;

    auto&& result = std::make_unique<ReturnNode>();
    result->mReturnValue = Expression();
    return rule.Accept(std::move(result));
  }


  std::unique_ptr<IfNode> If()
  {
    //If                  = <If> GroupedExpression Scope Else?
    PrintRule rule("If");
    if (!Accept(TokenType::If))
      return nullptr;

    auto&& result = std::make_unique<IfNode>();
    result->mCondition = Expect(GroupedExpression());
    result->mScope = Expect(Scope());
    result->mElse = Else();
    return rule.Accept(std::move(result));
  }

  std::unique_ptr<IfNode> Else()
  {
    //Else                = <Else> (If | Scope)
    PrintRule rule("Else");
    if (!Accept(TokenType::Else))
      return nullptr;

    if(auto&& node = If())
      return rule.Accept(std::move(node));

    auto&& result = std::make_unique<IfNode>();
    result->mScope = Expect(Scope());
    return rule.Accept(std::move(result));
  }

  std::unique_ptr<WhileNode> While()
  {
    //While               = <While> GroupedExpression Scope
    PrintRule rule("While");
    if (!Accept(TokenType::While))
      return nullptr;

    auto&& result = std::make_unique<WhileNode>();
    result->mCondition = Expect(GroupedExpression());
    result->mScope = Expect(Scope());
    return rule.Accept(std::move(result));
  }

  std::unique_ptr<ForNode> For()
  {
    //For                 = <For> <OpenParentheses> (Var | Expression)? <Semicolon> Expression? <Semicolon> Expression? <CloseParentheses> Scope
    PrintRule rule("For");
    if (!Accept(TokenType::For))
      return nullptr;

    auto&& result = std::make_unique<ForNode>();
    Expect(TokenType::OpenParentheses);

    if (auto&& varNode = Var())
      result->mInitialVariable = std::move(varNode);
    else if (auto&& expNode = Expression())
      result->mInitialExpression = std::move(expNode);

    Expect(TokenType::Semicolon);
    result->mCondition = Expression();
    Expect(TokenType::Semicolon);
    result->mIterator = Expression();
    Expect(TokenType::CloseParentheses);
    result->mScope = Expect(Scope());
    return rule.Accept(std::move(result));
  }

  // Literals, variables, or grouped expressions
  std::unique_ptr<ExpressionNode> GroupedExpression()
  {
    //GroupedExpression   = <OpenParentheses> Expression <CloseParentheses>
    PrintRule rule("GroupedExpression");
    Token token;
    if (!Accept(TokenType::OpenParentheses, &token))
      return nullptr;

    auto&& result = Expect(Expression());
    Expect(TokenType::CloseParentheses);
    return rule.Accept(std::move(result));
  }

  std::unique_ptr<LiteralNode> Literal()
  {
    //Literal             = <True> | <False> | <Null> | <IntegerLiteral> | <FloatLiteral> | <StringLiteral> | <CharacterLiteral>
    PrintRule rule("Literal");
    Token token;
    bool isValid = Accept(TokenType::True, &token) || Accept(TokenType::False, &token) || Accept(TokenType::Null, &token) ||
      Accept(TokenType::IntegerLiteral, &token) || Accept(TokenType::FloatLiteral, &token) ||
      Accept(TokenType::StringLiteral, &token) || Accept(TokenType::CharacterLiteral, &token);
    if (!isValid)
      return nullptr;

    auto&& node = std::make_unique<LiteralNode>();
    node->mToken = token;

    return rule.Accept(std::move(node));
  }

  std::unique_ptr<NameReferenceNode> NameReference()
  {
    //NameReference       = <Identifier>
    PrintRule rule("NameReference");
    Token token;
    if (!Accept(TokenType::Identifier, &token))
      return false;

    auto&& node = std::make_unique<NameReferenceNode>();
    node->mName = token;
    return rule.Accept(std::move(node));
  }

  std::unique_ptr<ExpressionNode> Value()
  {
    //Value               = Literal | NameReference | GroupedExpression
    PrintRule rule("Value");

    if (auto&& node = Literal())
      return rule.Accept(std::move(node));
    if (auto&& node = NameReference())
      return rule.Accept(std::move(node));
    if (auto&& node = GroupedExpression())
      return rule.Accept(std::move(node));
    return false;
  }

  std::unique_ptr<BinaryOperatorNode> MakeBinaryNode(std::unique_ptr<ExpressionNode> lhs, const Token& token, std::unique_ptr<ExpressionNode> rhs)
  {
    auto&& node = std::make_unique<BinaryOperatorNode>();
    node->mOperator = token;
    node->mLeft = std::move(lhs);
    node->mRight = std::move(rhs);
    return std::move(node);
  }


  // Right to left binary operators (note that Expression recurses into itself and is only optional, this preserves right to left)
  std::unique_ptr<ExpressionNode> Expression()
  {
    //Expression          = Expression1 ((<Assignment> | <AssignmentPlus> | <AssignmentMinus> | <AssignmentMultiply> | <AssignmentDivide> | <AssignmentModulo>) Expression)?
    PrintRule rule("Expression");
    auto&& root = Expression1();
    if (!root)
      return nullptr;

    Token token;
    if (Accept(TokenType::Assignment, &token) || Accept(TokenType::AssignmentPlus, &token) || Accept(TokenType::AssignmentMinus, &token) ||
      Accept(TokenType::AssignmentMultiply, &token) || Accept(TokenType::AssignmentDivide, &token) || Accept(TokenType::AssignmentModulo, &token))
    {
      root = MakeBinaryNode(std::move(root), token, Expect(Expression()));
    }

    return rule.Accept(std::move(root));
  }


  // Left to right binary operators
  std::unique_ptr<ExpressionNode> Expression1()
  {
    //Expression1         = Expression2 (<LogicalOr> Expression2)*
    PrintRule rule("Expression1");
    auto&& root = Expression2();
    if (!root)
      return nullptr;

    Token token;
    while (Accept(TokenType::LogicalOr, &token))
    {
      root = MakeBinaryNode(std::move(root), token, Expect(Expression2()));
    }
    return rule.Accept(std::move(root));
  }

  std::unique_ptr<ExpressionNode> Expression2()
  {
    //Expression2         = Expression3 (<LogicalAnd> Expression3)*
    PrintRule rule("Expression2");
    auto&& root = Expression3();
    if (!root)
      return nullptr;

    Token token;
    while (Accept(TokenType::LogicalAnd, &token))
      root = MakeBinaryNode(std::move(root), token, Expect(Expression3()));

    return rule.Accept(std::move(root));
  }

  std::unique_ptr<ExpressionNode> Expression3()
  {
    //Expression3         = Expression4 ((<LessThan> | <GreaterThan> | <LessThanOrEqualTo> | <GreaterThanOrEqualTo> | <Equality> | <Inequality>) Expression4)*
    PrintRule rule("Expression3");
    auto&& root = Expression4();
    if (!root)
      return nullptr;

    Token token;
    while (Accept(TokenType::LessThan, &token) || Accept(TokenType::GreaterThan, &token) ||
      Accept(TokenType::LessThanOrEqualTo, &token) || Accept(TokenType::GreaterThanOrEqualTo, &token) ||
      Accept(TokenType::Equality, &token) || Accept(TokenType::Inequality, &token))
    {
      root = MakeBinaryNode(std::move(root), token, Expect(Expression4()));
    }
    return rule.Accept(std::move(root));
  }

  std::unique_ptr<ExpressionNode> Expression4()
  {
    //Expression4         = Expression5 ((<Plus> | <Minus>) Expression5)*
    PrintRule rule("Expression4");
    auto&& root = Expression5();
    if (!root)
      return nullptr;

    Token token;
    while (Accept(TokenType::Plus, &token) || Accept(TokenType::Minus, &token))
    {
      root = MakeBinaryNode(std::move(root), token, Expect(Expression5()));
    }
    return rule.Accept(std::move(root));
  }

  std::unique_ptr<ExpressionNode> Expression5()
  {
    //Expression5         = Expression6 ((<Asterisk> | <Divide> | <Modulo>) Expression6)*
    PrintRule rule("Expression5");
    auto&& root = Expression6();
    if (!root)
      return nullptr;

    Token token;
    while (Accept(TokenType::Asterisk, &token) || Accept(TokenType::Divide, &token) || Accept(TokenType::Modulo, &token))
    {
      root = MakeBinaryNode(std::move(root), token, Expect(Expression6()));
    }
    return rule.Accept(std::move(root));
  }


  // Right to left unary operators
  std::unique_ptr<ExpressionNode> Expression6()
  {
    //Expression6         = (<Asterisk> | <Ampersand> | <Plus> | <Minus> | <LogicalNot> | <Increment> | <Decrement>)* Expression7
    PrintRule rule("Expression6");

    Token token;
    std::unique_ptr<UnaryOperatorNode> root = nullptr;
    UnaryOperatorNode* current = nullptr;
    
    while (Accept(TokenType::Asterisk, &token) || Accept(TokenType::Ampersand, &token) ||
      Accept(TokenType::Plus, &token) || Accept(TokenType::Minus, &token) ||
      Accept(TokenType::LogicalNot, &token) || Accept(TokenType::Increment, &token) || Accept(TokenType::Decrement, &token))
    {
      auto&& node = std::make_unique<UnaryOperatorNode>();
      node->mOperator = token;
      UnaryOperatorNode* nodePtr = node.get();
      
      if (root == nullptr)
        root = std::move(node);
      else
        current->mRight = std::move(node);
      current = nodePtr;
    }

    std::unique_ptr<ExpressionNode> exp7 = Expression7();
    if (exp7 == nullptr)
      return nullptr;

    if (current == nullptr)
    {
      return rule.Accept(std::move(exp7));
    }
    else
    {
      current->mRight = std::move(exp7);

      return rule.Accept(std::move(root));
    }
  }


    // Left to right binary operators
  std::unique_ptr<ExpressionNode> Expression7()
  {
    //Expression7         = Value (MemberAccess | Call | Cast | Index)*
    PrintRule rule("Expression7");
    std::unique_ptr<ExpressionNode>&& root = Value();
    if (!root)
      return nullptr;

    while (true)
    {
      if (auto&& node = MemberAccess())
      {
        node->mLeft = std::move(root);
        root = std::move(node);
      }
      else if (auto&& node = MemberAccess())
      {
        node->mLeft = std::move(root);
        root = std::move(node);
      }
      else if (auto&& node = Call())
      {
        node->mLeft = std::move(root);
        root = std::move(node);
      }
      else if (auto&& node = Cast())
      {
        node->mLeft = std::move(root);
        root = std::move(node);
      }
      else if (auto&& node = Index())
      {
        node->mLeft = std::move(root);
        root = std::move(node);
      }
      else
        break;
    }

    return rule.Accept(std::move(root));
  }


  std::unique_ptr<MemberAccessNode> MemberAccess()
  {
    //MemberAccess        = (<Dot> | <Arrow>) <Identifier>
    PrintRule rule("MemberAccess");
    Token operatorToken;
    if (!(Accept(TokenType::Dot, &operatorToken) || Accept(TokenType::Arrow, &operatorToken)))
      return false;

    auto&& node = std::make_unique<MemberAccessNode>();
    node->mOperator = operatorToken;
    Expect(TokenType::Identifier, &node->mName);
    return rule.Accept(std::move(node));
  }

  std::unique_ptr<CallNode> Call()
  {
    //Call                = <OpenParentheses> (Expression (<Comma> Expression)*)? <CloseParentheses>
    PrintRule rule("Call");
    if (!Accept(TokenType::OpenParentheses))
      return false;

    auto&& node = std::make_unique<CallNode>();
    if (auto&& exp0 = Expression())
    {
      node->mArguments.push_back(std::move(exp0));
      while (Accept(TokenType::Comma))
        node->mArguments.push_back(Expect(Expression()));
    }
    Expect(TokenType::CloseParentheses);
    return rule.Accept(std::move(node));
  }

  std::unique_ptr<CastNode> Cast()
  {
    //Cast                = <As> Type
    PrintRule rule("Cast");
    if (!Accept(TokenType::As))
      return false;

    auto&& node = std::make_unique<CastNode>();
    node->mType = Expect(Type());
    return rule.Accept(std::move(node));
  }

  std::unique_ptr<IndexNode> Index()
  {
    //Index = <OpenBracket> Expression <CloseBracket>
    PrintRule rule("Index");
    if (!Accept(TokenType::OpenBracket))
      return nullptr;

    auto&& node = std::make_unique<IndexNode>();
    node->mIndex = Expect(Expression());
    Expect(TokenType::CloseBracket);

    return rule.Accept(std::move(node));
  }

  bool Accept(TokenType::Enum tokenType, Token* token = nullptr)
  {
    size_t index = mTokenIndex;
    if (mTokenIndex < mTokens.size() && mTokens[index].mTokenType == tokenType)
    {
      ++mTokenIndex;
      if (token != nullptr)
        *token = mTokens[index];
      PrintRule::AcceptedToken(mTokens[index]);
      return true;
    }
    return false;
  }

  bool Expect(TokenType::Enum tokenType, Token* token = nullptr)
  {
    return Expect(Accept(tokenType, token));
  }

  bool Expect(bool state)
  {
    if(!state)
      throw ParsingException();
    return state;
  }

  template <typename T>
  std::unique_ptr<T> Expect(std::unique_ptr<T> ptr)
  {
    if (!ptr)
      throw ParsingException();
    return ptr;
  }

  std::vector<Token> mTokens;
  size_t mTokenIndex = 0;
};

void RemoveWhitespaceAndComments(std::vector<Token>& tokens)
{
  size_t i = 0;
  size_t j = 0;
  while (j < tokens.size())
  {
    auto&& tokenType = tokens[j].mTokenType;
    bool skip = tokenType == TokenType::Whitespace || tokenType == TokenType::SingleLineComment || tokenType == TokenType::MultiLineComment;
    if (!skip)
    {
      tokens[i] = tokens[j];
      ++i;
    }
    ++j;
  }
  tokens.resize(i);
}

void Recognize(std::vector<Token>& tokens)
{
  Parser parser;
  parser.Load(tokens);
  parser.ReadTokens();
}


std::unique_ptr<ExpressionNode> ParseExpression(std::vector<Token>& tokens)
{
  Parser parser;
  parser.Load(tokens);
  return parser.Expression();
}

std::unique_ptr<BlockNode> ParseBlock(std::vector<Token>& tokens)
{
  Parser parser;
  parser.Load(tokens);
  return parser.Block();
}

enum VisitResult
{
  Continue,
  Stop
};


void PrintValues(NodePrinter& printer, AbstractNode* node)
{

}

void PrintValues(NodePrinter& printer, ClassNode* node) {   printer << "(" << node->mName << ")"; }
void PrintValues(NodePrinter& printer, VariableNode* node) {   printer << "(" << node->mName << ")"; }
void PrintValues(NodePrinter& printer, FunctionNode* node) {   printer << "(" << node->mName << ")"; }
void PrintValues(NodePrinter& printer, NamedTypeNode* node) {   printer << "(" << node->mName << ")"; }
void PrintValues(NodePrinter& printer, GotoNode* node) {   printer << "(" << node->mName << ")"; }
void PrintValues(NodePrinter& printer, LabelNode* node) {   printer << "(" << node->mName << ")"; }
void PrintValues(NodePrinter& printer, LiteralNode* node) {   printer << "(" << node->mToken << ")"; }
void PrintValues(NodePrinter& printer, NameReferenceNode* node) { printer << "(" << node->mName << ")"; }
void PrintValues(NodePrinter& printer, BinaryOperatorNode* node) { printer << "(" << node->mOperator << ")"; }
void PrintValues(NodePrinter& printer, UnaryOperatorNode* node) { printer << "(" << node->mOperator << ")"; }
void PrintValues(NodePrinter& printer, MemberAccessNode* node) {   printer << "(" << node->mOperator << ", " << node->mName << ")"; }

class Visitor
{
public:
#define VisitNode(NodeType)                   \
  NodePrinter printer;                                        \
  printer << #NodeType ;\
  node->Walk(this, false);                                    \
  return VisitResult::Stop;

#define VisitNodeWithValue(NodeType, Value)                   \
  NodePrinter printer;                                        \
  printer << #NodeType "(" << Value << ")"; \
  node->Walk(this, false);                                    \
  return VisitResult::Stop;

//#define DeclareNodeVisitWithValue(NodeType, Value) virtual VisitResult Visit(NodeType* node) { VisitNodeWithValue(NodeType, Value); }
//#define DeclareNodeVisit(NodeType) virtual VisitResult Visit(NodeType* node) { VisitNode(NodeType); }
#define DeclareNodeVisit(NodeType) virtual VisitResult Visit(NodeType* node)\
{\
  NodePrinter printer;                                        \
    printer << #NodeType;\
    PrintValues(printer, node);\
    node->Walk(this, false);                                    \
    return VisitResult::Stop;\
}

  DeclareNodeVisit(AbstractNode);
  DeclareNodeVisit(BlockNode);
  DeclareNodeVisit(ClassNode);
  DeclareNodeVisit(StatementNode);
  DeclareNodeVisit(VariableNode);
  DeclareNodeVisit(ScopeNode);
  DeclareNodeVisit(ParameterNode);
  DeclareNodeVisit(FunctionNode);
  DeclareNodeVisit(TypeNode);
  DeclareNodeVisit(PointerTypeNode);
  DeclareNodeVisit(ReferenceTypeNode);
  DeclareNodeVisit(NamedTypeNode);
  DeclareNodeVisit(FunctionTypeNode);
  DeclareNodeVisit(LabelNode);
  DeclareNodeVisit(GotoNode);
  DeclareNodeVisit(ReturnNode);
  DeclareNodeVisit(BreakNode);
  DeclareNodeVisit(ContinueNode);
  DeclareNodeVisit(ExpressionNode);
  DeclareNodeVisit(IfNode);
  DeclareNodeVisit(WhileNode);
  DeclareNodeVisit(ForNode);
  DeclareNodeVisit(LiteralNode);
  DeclareNodeVisit(NameReferenceNode);
  DeclareNodeVisit(BinaryOperatorNode);
  DeclareNodeVisit(UnaryOperatorNode);
  DeclareNodeVisit(PostExpressionNode);
  DeclareNodeVisit(MemberAccessNode);
  DeclareNodeVisit(CallNode);
  DeclareNodeVisit(CastNode);
  DeclareNodeVisit(IndexNode);
};

#define VisitAndWalkBase(visitor)\
if (visit && visitor->Visit(this) == Stop) \
  return; \
__super::Walk(visitor, false);

template <typename NodeType> void TryWalk(Visitor* visitor, std::unique_ptr<NodeType>& node)
{
  if (node != nullptr)
    node->Walk(visitor);
}

template <typename NodeType> void TryWalk(Visitor* visitor, unique_vector<NodeType>& nodes)
{
  for (size_t i = 0; i < nodes.size(); ++i)
    nodes[i]->Walk(visitor);
}

void BlockNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mGlobals);
}

void ClassNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mMembers);
}

void StatementNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void VariableNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  mType->Walk(visitor);
  TryWalk(visitor, mInitialValue);
}

void ScopeNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mStatements);
}

void ParameterNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void FunctionNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mParameters);
  TryWalk(visitor, mReturnType);
  TryWalk(visitor, mScope);
}

void TypeNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void PointerTypeNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mPointerTo);
}

void ReferenceTypeNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mReferenceTo);
}

void NamedTypeNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void FunctionTypeNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mParameters);
  TryWalk(visitor, mReturn);
}

void LabelNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void GotoNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void ReturnNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mReturnValue);
}

void BreakNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void ContinueNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void ExpressionNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void IfNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mCondition);
  TryWalk(visitor, mScope);
  TryWalk(visitor, mElse);
}

void WhileNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mCondition);
  TryWalk(visitor, mScope);
}

void ForNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mInitialVariable);
  TryWalk(visitor, mInitialExpression);
  TryWalk(visitor, mCondition);
  TryWalk(visitor, mScope);
  TryWalk(visitor, mIterator);
}

void LiteralNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void NameReferenceNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void BinaryOperatorNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  mLeft->Walk(visitor);
  mRight->Walk(visitor);
}

void UnaryOperatorNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  mRight->Walk(visitor);
}

void PostExpressionNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  mLeft->Walk(visitor);
}

void MemberAccessNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
}

void CallNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  TryWalk(visitor, mArguments);
}

void CastNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  mType->Walk(visitor);
}

void IndexNode::Walk(Visitor* visitor, bool visit)
{
  VisitAndWalkBase(visitor);
  mIndex->Walk(visitor);
}



void PrintTree(AbstractNode* node)
{
  Visitor visitor;
  node->Walk(&visitor, true);
}

