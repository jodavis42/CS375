/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver3.hpp"

#include "../Drivers/AstNodes.hpp"
#include "Parser.hpp"
#include "Visitor.hpp"

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

