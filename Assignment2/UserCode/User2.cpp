/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver2.hpp"

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

  bool Block()
  {
    //Block               = (Class | Function | Var <Semicolon>)*
    PrintRule rule("Block");
    bool result = false;

    while (true)
    {
      result = Class() || Function() || (Var() && Expect(TokenType::Semicolon));
      if (!result)
        break;
    }
    return rule.Accept();
  }

  bool Class()
  {
    //Class               = <Class> <Identifier> <OpenCurley> (Var <Semicolon> | Function)* <CloseCurley>
    PrintRule rule("Class");
    if (this->Accept(TokenType::Class) == false)
      return false;

    this->Expect(TokenType::Identifier);
    this->Expect(TokenType::OpenCurley);

    while ((this->Var() && this->Expect(TokenType::Semicolon)) || this->Function());

    return rule.Accept(this->Expect(TokenType::CloseCurley));
  }

  bool Var()
  {
    //Var                 = <Var> <Identifier> SpecifiedType (<Assignment> Expression)?
    PrintRule rule("Var");
    if (!Accept(TokenType::Var))
      return false;

    Expect(TokenType::Identifier) && Expect(SpecifiedType());
    Accept(TokenType::Assignment) && Expect(Expression());

    return rule.Accept();
  }

  bool Function()
  {
    //Function            = <Function> <Identifier> <OpenPareantheses> (Parameter (<Comma> Parameter)*)? <CloseParentheses> SpecifiedType? Scope
    PrintRule rule("Function");
    if (!Accept(TokenType::Function))
      return false;

    Expect(TokenType::Identifier) && Expect(TokenType::OpenParentheses);
    if (Parameter())
    {
      while (Accept(TokenType::Comma) && Expect(Parameter())) {};
    }

    Expect(TokenType::CloseParentheses);
    SpecifiedType();
    Expect(Scope());

    return rule.Accept();
  }

  bool Parameter()
  {
    //Parameter           = <Identifier> SpecifiedType
    PrintRule rule("Parameter");
    if (!Accept(TokenType::Identifier))
      return false;
    Expect(SpecifiedType());
    return rule.Accept();
  }

  bool SpecifiedType()
  {
    //<Colon> Type
    PrintRule rule("SpecifiedType");
    if (!(Accept(TokenType::Colon)))
      return false;
    Expect(Type());
    return rule.Accept();
  }

  bool Type()
  {
    //NamedType | FunctionType
    PrintRule rule("Type");
    if(!(NamedType() || FunctionType()))
      return false;
    return rule.Accept();
  }

  bool NamedType()
  {
    //NamedType           = <Identifier> <Asterisk>* <Ampersand>?
    PrintRule rule("NamedType");
    if (!Accept(TokenType::Identifier))
      return false;

    while (Accept(TokenType::Asterisk)) {}
    Accept(TokenType::Ampersand);

    return rule.Accept();
  }

  bool FunctionType()
  {
    //FunctionType        = <Function> <Asterisk>+ <Ampersand>? <OpenParentheses> Type (<Comma> Type)* <CloseParentheses> SpecifiedType?
    PrintRule rule("FunctionType");
    if (!Accept(TokenType::Function))
      return false;

    Expect(TokenType::Asterisk);
    while (Accept(TokenType::Asterisk));
    Accept(TokenType::Ampersand);
    Expect(TokenType::OpenParentheses);
    Expect(Type());
    while (Accept(TokenType::Comma) && Expect(Type()));
    Expect(TokenType::CloseParentheses);
    SpecifiedType();

    return rule.Accept();
  }
  
  bool Scope()
  {
    //Scope               = <OpenCurley> Statement* <CloseCurley>
    PrintRule rule("Scope");
    if (!Accept(TokenType::OpenCurley))
      return false;

    while (Statement());
    Expect(TokenType::CloseCurley);
    return rule.Accept();
  }
  
  bool Statement()
  {
    //Statement           = FreeStatement | DelimitedStatement <Semicolon>
    PrintRule rule("Statement");
    if(FreeStatement() || (DelimitedStatement() && Expect(TokenType::Semicolon)))
      return rule.Accept();
    return false;
  }
  
  bool DelimitedStatement()
  {
    //DelimitedStatement  = Label | Goto | Return | <Break> | <Continue> | Var | Expression
    PrintRule rule("DelimitedStatement");
    if (!(Label() || Goto() || Return() || Accept(TokenType::Break) || Accept(TokenType::Continue) || Var() || Expression()))
      return false;
    return rule.Accept();
  }
  
  bool FreeStatement()
  {
    //If | While | For
    PrintRule rule("FreeStatement");
    if (!(If() || While() || For()))
      return false;
    return rule.Accept();
  }



  bool Label()
  {
    //Label               = <Label> <Identifier>
    PrintRule rule("Label");
    if (!Accept(TokenType::Label))
      return false;
    Expect(TokenType::Identifier);
    return rule.Accept();
  }

  bool Goto()
  {
    //Goto                = <Goto> <Identifier>
    PrintRule rule("Goto");
    if (!Accept(TokenType::Goto))
      return false;
    Expect(TokenType::Identifier);
    return rule.Accept();
  }

  bool Return()
  {
    //Return              = <Return> (Expression)?
    PrintRule rule("Return");
    if (!Accept(TokenType::Return))
      return false;
    Expression();
    return rule.Accept();
  }


  bool If()
  {
    //If                  = <If> GroupedExpression Scope Else?
    PrintRule rule("If");
    if (!Accept(TokenType::If))
      return false;
    Expect(GroupedExpression()) && Expect(Scope()) && Else();
    return rule.Accept();
  }

  bool Else()
  {
    //Else                = <Else> (If | Scope)
    PrintRule rule("Else");
    if (!Accept(TokenType::Else))
      return false;
    Expect(If() || Scope());
    return rule.Accept();
  }

  bool While()
  {
    //While               = <While> GroupedExpression Scope
    PrintRule rule("While");
    if (!Accept(TokenType::While))
      return false;
    Expect(GroupedExpression()) && Expect(Scope());
    return rule.Accept();
  }

  bool For()
  {
    //For                 = <For> <OpenParentheses> (Var | Expression)? <Semicolon> Expression? <Semicolon> Expression? <CloseParentheses> Scope
    PrintRule rule("For");
    if (!Accept(TokenType::For))
      return false;

    Expect(TokenType::OpenParentheses);
    Var() || Expression();
    Expect(TokenType::Semicolon);
    Expression();
    Expect(TokenType::Semicolon);
    Expression();
    Expect(TokenType::CloseParentheses) && Expect(Scope());
    return rule.Accept();
  }


  // Literals, variables, or grouped expressions
  bool GroupedExpression()
  {
    //GroupedExpression   = <OpenParentheses> Expression <CloseParentheses>
    PrintRule rule("GroupedExpression");
    if (!Accept(TokenType::OpenParentheses))
      return false;

    Expect(Expression()) && Expect(TokenType::CloseParentheses);
    return rule.Accept();
  }

  bool Literal()
  {
    //Literal             = <True> | <False> | <Null> | <IntegerLiteral> | <FloatLiteral> | <StringLiteral> | <CharacterLiteral>
    PrintRule rule("Literal");
    bool isValid = Accept(TokenType::True) || Accept(TokenType::False) || Accept(TokenType::Null) ||
      Accept(TokenType::IntegerLiteral) || Accept(TokenType::FloatLiteral) ||
      Accept(TokenType::StringLiteral) || Accept(TokenType::CharacterLiteral);
    if (!isValid)
      return false;

    return rule.Accept();
  }

  bool NameReference()
  {
    //NameReference       = <Identifier>
    PrintRule rule("NameReference");
    if (!Accept(TokenType::Identifier))
      return false;
    return rule.Accept();
  }

  bool Value()
  {
    //Value               = Literal | NameReference | GroupedExpression
    PrintRule rule("Value");
    if (!(Literal() || NameReference() || GroupedExpression()))
      return false;
    return rule.Accept();
  }


  // Right to left binary operators (note that Expression recurses into itself and is only optional, this preserves right to left)
  bool Expression()
  {
    //Expression          = Expression1 ((<Assignment> | <AssignmentPlus> | <AssignmentMinus> | <AssignmentMultiply> | <AssignmentDivide> | <AssignmentModulo>) Expression)?
    PrintRule rule("Expression");
    if (!Expression1())
      return false;

    if(Accept(TokenType::Assignment) || Accept(TokenType::AssignmentPlus) || Accept(TokenType::AssignmentMinus) ||
      Accept(TokenType::AssignmentMultiply) || Accept(TokenType::AssignmentDivide) || Accept(TokenType::AssignmentModulo))
      Expression();

    return rule.Accept();
  }


  // Left to right binary operators
  bool Expression1()
  {
    //Expression1         = Expression2 (<LogicalOr> Expression2)*
    PrintRule rule("Expression1");
    if (!Expression2())
      return false;
    Accept(TokenType::LogicalOr) && Expression2();
    return rule.Accept();
  }

  bool Expression2()
  {
    //Expression2         = Expression3 (<LogicalAnd> Expression3)*
    PrintRule rule("Expression2");
    if (!Expression3())
      return false;
    Accept(TokenType::LogicalAnd) && Expression3();
    return rule.Accept();
  }

  bool Expression3()
  {
    //Expression3         = Expression4 ((<LessThan> | <GreaterThan> | <LessThanOrEqualTo> | <GreaterThanOrEqualTo> | <Equality> | <Inequality>) Expression4)*
    PrintRule rule("Expression3");
    if (!Expression4())
      return false;

    if(Accept(TokenType::LessThan) || Accept(TokenType::GreaterThan) ||
      Accept(TokenType::LessThanOrEqualTo) || Accept(TokenType::GreaterThanOrEqualTo) ||
      Accept(TokenType::Equality) || Accept(TokenType::Inequality))
      Expression4();
    return rule.Accept();
  }

  bool Expression4()
  {
    //Expression4         = Expression5 ((<Plus> | <Minus>) Expression5)*
    PrintRule rule("Expression4");
    if (!Expression5())
      return false;

    (Accept(TokenType::Plus) || Accept(TokenType::Minus)) && Expect(Expression5());
    return rule.Accept();
  }

  bool Expression5()
  {
    //Expression5         = Expression6 ((<Asterisk> | <Divide> | <Modulo>) Expression6)*
    PrintRule rule("Expression5");
    if (!Expression6())
      return false;

    if (Accept(TokenType::Asterisk) || Accept(TokenType::Divide) || Accept(TokenType::Modulo))
      Expression6();
    return rule.Accept();
  }


  // Right to left unary operators
  bool Expression6()
  {
    //Expression6         = (<Asterisk> | <Ampersand> | <Plus> | <Minus> | <LogicalNot> | <Increment> | <Decrement>)* Expression7
    PrintRule rule("Expression6");
    Accept(TokenType::Asterisk) || Accept(TokenType::Ampersand) ||
      Accept(TokenType::Plus) || Accept(TokenType::Minus) ||
      Accept(TokenType::LogicalNot) || Accept(TokenType::Increment) || Accept(TokenType::Decrement);
    if (!Expression7())
      return false;
    return rule.Accept();
  }


    // Left to right binary operators
  bool Expression7()
  {
    //Expression7         = Value (MemberAccess | Call | Cast | Index)*
    PrintRule rule("Expression7");
    if (!Value())
      return false;

    while (MemberAccess() || Call() || Cast() || Index());
    return rule.Accept();
  }


  bool MemberAccess()
  {
    //MemberAccess        = (<Dot> | <Arrow>) <Identifier>
    PrintRule rule("MemberAccess");
    if (!(Accept(TokenType::Dot) || Accept(TokenType::Arrow)))
      return false;

    Expect(TokenType::Identifier);
    return rule.Accept();
  }

  bool Call()
  {
    //Call                = <OpenParentheses> (Expression (<Comma> Expression)*)? <CloseParentheses>
    PrintRule rule("Call");
    if (!Accept(TokenType::OpenParentheses))
      return false;

    if (Expression())
    {
      while (Accept(TokenType::Comma))
        Expect(Expression());
    }
    Expect(TokenType::CloseParentheses);
    return rule.Accept();
  }

  bool Cast()
  {
    //Cast                = <As> Type
    PrintRule rule("Cast");
    if (!Accept(TokenType::As))
      return false;

    Expect(Type());
    return rule.Accept();
  }

  bool Index()
  {
    //Index = <OpenBracket> Expression <CloseBracket>
    PrintRule rule("Index");
    if (!Accept(TokenType::OpenBracket))
      return false;

    Expect(Expression()) && Expect(TokenType::CloseBracket);
    return rule.Accept();
  }

  bool Accept(TokenType::Enum tokenType)
  {
    size_t index = mTokenIndex;
    if (mTokenIndex < mTokens.size() && mTokens[index].mTokenType == tokenType)
    {
      ++mTokenIndex;
      PrintRule::AcceptedToken(mTokens[index]);
      return true;
    }
    return false;
  }

  bool Expect(TokenType::Enum tokenType)
  {
    return Expect(Accept(tokenType));
  }

  bool Expect(bool state)
  {
    if(!state)
      throw ParsingException();
    return state;
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
    if (tokens[j].mTokenType != TokenType::Whitespace)
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
