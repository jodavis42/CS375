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
