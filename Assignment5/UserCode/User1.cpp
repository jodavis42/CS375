/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver1.hpp"

#include <unordered_map>
#include <array>
#include "../Drivers/AstNodes.hpp"

const char cDefaultEdgeId = -1;

class MyClass
{

  std::unique_ptr<ParameterNode> Parameter()
  {
    return std::make_unique< ParameterNode>();
  }

  template <typename T, std::unique_ptr<T>(MyClass::* Fn)()>
  bool GetNode(std::unique_ptr<T>& result)
  {
    result = (this->*Fn)();
    return result != nullptr;
  }

  void Run()
  {
    auto&& param0 = Parameter();
    std::unique_ptr< ParameterNode> param1;
    GetNode<ParameterNode, &MyClass::Parameter>(param1);
  }
};

class DfaState
{
public:
  int mAcceptingToken = 0;
  std::unordered_map<char, DfaState*> mEdges;
  bool mValid = true;

  DfaState* Find(char value)
  {
    auto it = mEdges.find(value);
    if (it == mEdges.end())
      it = mEdges.find(cDefaultEdgeId);

    if (it == mEdges.end())
      return nullptr;
    return it->second;
  }
};

DfaState* AddState(int acceptingToken)
{
  DfaState* result = new DfaState();
  result->mAcceptingToken = acceptingToken;
  return result;
}

void AddEdge(DfaState* from, DfaState* to, char c)
{
  from->mEdges[c] = to;
}

void AddDefaultEdge(DfaState* from, DfaState* to)
{
  from->mEdges[cDefaultEdgeId] = to;
}

void ReadToken(DfaState* startingState, const char* stream, Token& outToken)
{
  DfaState* acceptingState = nullptr;
  DfaState* state = startingState;
  size_t index = 0;
  size_t acceptedIndex = 0;

  while (stream[index] != 0)
  {
    char value = stream[index];
    state = state->Find(value);
    if (state == nullptr)
      break;

    ++index;
    if (state->mAcceptingToken != 0)
    {
      acceptingState = state;
      acceptedIndex = index;
    }
  }

  outToken.mLength = index;
  outToken.mText = stream;
  if (acceptingState != nullptr)
  {
    outToken.mTokenType = acceptingState->mAcceptingToken;
    outToken.mLength = acceptedIndex;
  }
}

void DeleteStateAndChildren(DfaState* root)
{
  if (root == nullptr || root->mValid == false)
    return;
  root->mValid = false;

  for (auto&& pair : root->mEdges)
  {
    DeleteStateAndChildren(pair.second);
  }
}

std::unordered_map<std::string, TokenType::Enum> BuildLookupMap()
{
  std::unordered_map<std::string, TokenType::Enum> result;
#define TOKEN(Name, Value) result.insert(std::make_pair(Value, TokenType::Name));
#include "../Drivers/TokenKeywords.inl"
#undef TOKEN
  return result;
}

static std::unordered_map<std::string, TokenType::Enum> cLookupMap = BuildLookupMap();

void ReadLanguageToken(DfaState* startingState, const char* stream, Token& outToken)
{
  ReadToken(startingState, stream, outToken);
  if (outToken.mTokenType == TokenType::Identifier)
  {
    std::string str(outToken.mText, outToken.mLength);
    auto&& it = cLookupMap.find(str);
    if (it != cLookupMap.end())
      outToken.mTokenType = it->second;
  }
}

void CreateRuleWhitespace(DfaState* root)
{
  std::array<char, 4> values = { ' ', '\r', '\n', '\t' };
  DfaState* state = AddState(TokenType::Whitespace);
  for (size_t i = 0; i < values.size(); ++i)
  {
    AddEdge(root, state, values[i]);
    AddEdge(state, state, values[i]);
  }
}

void CreateRuleIdentifier(DfaState* root)
{
  DfaState* state = AddState(TokenType::Identifier);
  for (char value = 'a'; value <= 'z'; ++value)
  {
    AddEdge(root, state, value);
    AddEdge(state, state, value);
  }
  for (char value = 'A'; value <= 'Z'; ++value)
  {
    AddEdge(root, state, value);
    AddEdge(state, state, value);
  }
  AddEdge(root, state, '_');
  AddEdge(state, state, '_');
  for (char value = '0'; value <= '9'; ++value)
    AddEdge(state, state, value);
}

DfaState* CreateIntRule(DfaState* root)
{
  DfaState* intState = AddState(TokenType::IntegerLiteral);
  for (char value = '0'; value <= '9'; ++value)
  {
    AddEdge(root, intState, value);
    AddEdge(intState, intState, value);
  }
  return intState;
}

DfaState* CreateFloatRule(DfaState* intState)
{
  DfaState* dotState = AddState(0);
  DfaState* eState = AddState(0);
  DfaState* signState = AddState(0);
  DfaState* floatState0 = AddState(TokenType::FloatLiteral);
  DfaState* floatState1 = AddState(TokenType::FloatLiteral);
  DfaState* floatState2 = AddState(TokenType::FloatLiteral);

  AddEdge(intState, dotState, '.');
  for (char value = '0'; value <= '9'; ++value)
  {
    AddEdge(dotState, floatState0, value);
    AddEdge(floatState0, floatState0, value);
  }

  AddEdge(floatState0, eState, 'e');
  AddEdge(eState, signState, '+');
  AddEdge(eState, signState, '-');
  for (char value = '0'; value <= '9'; ++value)
  {
    AddEdge(eState, floatState1, value);
    AddEdge(signState, floatState1, value);
    AddEdge(floatState1, floatState1, value);
  }
  AddEdge(floatState1, floatState2, 'f');
  AddEdge(floatState0, floatState2, 'f');
  return floatState2;
}

void BuildTokenRule(DfaState* root, TokenType::Enum tokenType, char* str)
{
  DfaState* state = root;
  while (str != nullptr && *str != '\0')
  {
    char value = *str;
    DfaState* nextState = state->Find(value);
    if (nextState == nullptr)
    {
      nextState = AddState(0);
      AddEdge(state, nextState, value);
    }
    state = nextState;
    ++str;
  }
  state->mAcceptingToken = tokenType;
}

void BuildTokenRules(DfaState* root)
{

#define TOKEN(Name, Value) BuildTokenRule(root, TokenType::Name, Value);
#include "../Drivers/TokenSymbols.inl"
#undef TOKEN
}

DfaState* StringLiteralRule(DfaState* root, TokenType::Enum tokenType, char delimiterChar)
{
  DfaState* begin = AddState(0);
  DfaState* charState = AddState(0);
  DfaState* startEscapeState = AddState(0);
  DfaState* badEscapeState = AddState(0);
  DfaState* end = AddState(tokenType);
  AddEdge(root, begin, delimiterChar);
  AddDefaultEdge(begin, charState);
  AddDefaultEdge(charState, charState);
  AddEdge(charState, startEscapeState, '\\');
  AddEdge(startEscapeState, charState, 'n');
  AddEdge(startEscapeState, charState, 'r');
  AddEdge(startEscapeState, charState, 't');
  AddEdge(startEscapeState, charState, delimiterChar);
  AddEdge(charState, end, delimiterChar);
  return end;
}

DfaState* StringLiteralRule(DfaState* root)
{
  return StringLiteralRule(root, TokenType::StringLiteral, '\"');
}

void CommentRule(DfaState* root)
{
  DfaState* firstSlash = root->Find('/');
  DfaState* singleCommentBody = AddState(0);
  DfaState* singleCommentEnd = AddState(TokenType::SingleLineComment);
  DfaState* multiCommentBody = AddState(0);
  DfaState* multiCommentStar = AddState(0);
  DfaState* multiCommentEnd = AddState(TokenType::MultiLineComment);

  AddEdge(root, firstSlash, '/');

  AddEdge(firstSlash, singleCommentBody, '/');
  AddDefaultEdge(singleCommentBody, singleCommentBody);
  AddEdge(singleCommentBody, singleCommentEnd, '\r');
  AddEdge(singleCommentBody, singleCommentEnd, '\n');
  AddEdge(singleCommentBody, singleCommentEnd, '\0');

  AddEdge(firstSlash, multiCommentBody, '*');
  AddDefaultEdge(multiCommentBody, multiCommentBody);
  AddEdge(multiCommentBody, multiCommentStar, '*');
  AddDefaultEdge(multiCommentStar, multiCommentBody);
  AddEdge(multiCommentStar, multiCommentEnd, '/');
}

DfaState* CharLiteralRule(DfaState* root)
{
  DfaState* begin = AddState(0);
  DfaState* charState = AddState(0);
  DfaState* end = AddState(TokenType::CharacterLiteral);
  AddEdge(root, begin, '\'');
  AddDefaultEdge(begin, charState);
  AddDefaultEdge(charState, charState);
  AddEdge(charState, end, '\'');
  return end;
}

DfaState* CreateLanguageDfa()
{
  DfaState* root = AddState(0);
  CreateRuleWhitespace(root);
  CreateRuleIdentifier(root);
  DfaState* intRule = CreateIntRule(root);
  DfaState* floatRule = CreateFloatRule(intRule);
  BuildTokenRules(root);
  StringLiteralRule(root);
  CharLiteralRule(root);
  CommentRule(root);
  return root;
}
