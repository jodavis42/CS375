/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver3.hpp"

// Copy the contents of your User2.cpp over this file and start from there
// You still need functions such as RemoveWhiteSpaceAndComments from Driver2

std::unique_ptr<ExpressionNode> ParseExpression(std::vector<Token>& tokens)
{
  throw ParsingException();
}

std::unique_ptr<BlockNode> ParseBlock(std::vector<Token>& tokens)
{
  throw ParsingException();
}

void PrintTree(AbstractNode* node)
{
}

void RemoveWhitespaceAndComments(std::vector<Token>& tokens)
{
}

void Recognize(std::vector<Token>& tokens)
{
  throw ParsingException();
}

void BlockNode::Walk(Visitor* visitor, bool visit)
{
}

void ClassNode::Walk(Visitor* visitor, bool visit)
{
}

void StatementNode::Walk(Visitor* visitor, bool visit)
{
}

void VariableNode::Walk(Visitor* visitor, bool visit)
{
}

void ScopeNode::Walk(Visitor* visitor, bool visit)
{
}

void ParameterNode::Walk(Visitor* visitor, bool visit)
{
}

void FunctionNode::Walk(Visitor* visitor, bool visit)
{
}

void TypeNode::Walk(Visitor* visitor, bool visit)
{
}

void PointerTypeNode::Walk(Visitor* visitor, bool visit)
{
}

void ReferenceTypeNode::Walk(Visitor* visitor, bool visit)
{
}

void NamedTypeNode::Walk(Visitor* visitor, bool visit)
{
}

void FunctionTypeNode::Walk(Visitor* visitor, bool visit)
{
}

void LabelNode::Walk(Visitor* visitor, bool visit)
{
}

void GotoNode::Walk(Visitor* visitor, bool visit)
{
}

void ReturnNode::Walk(Visitor* visitor, bool visit)
{
}

void BreakNode::Walk(Visitor* visitor, bool visit)
{
}

void ContinueNode::Walk(Visitor* visitor, bool visit)
{
}

void ExpressionNode::Walk(Visitor* visitor, bool visit)
{
}

void IfNode::Walk(Visitor* visitor, bool visit)
{
}

void WhileNode::Walk(Visitor* visitor, bool visit)
{
}

void ForNode::Walk(Visitor* visitor, bool visit)
{
}

void LiteralNode::Walk(Visitor* visitor, bool visit)
{
}

void NameReferenceNode::Walk(Visitor* visitor, bool visit)
{
}

void BinaryOperatorNode::Walk(Visitor* visitor, bool visit)
{
}

void UnaryOperatorNode::Walk(Visitor* visitor, bool visit)
{
}

void PostExpressionNode::Walk(Visitor* visitor, bool visit)
{
}

void MemberAccessNode::Walk(Visitor* visitor, bool visit)
{
}

void CallNode::Walk(Visitor* visitor, bool visit)
{
}

void CastNode::Walk(Visitor* visitor, bool visit)
{
}

void IndexNode::Walk(Visitor* visitor, bool visit)
{
}