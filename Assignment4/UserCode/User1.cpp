/******************************************************************\
 * Author: 
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/
#include "../Drivers/Driver1.hpp"

DfaState* AddState(int acceptingToken)
{
  return nullptr;
}

void AddEdge(DfaState* from, DfaState* to, char c)
{
}

void AddDefaultEdge(DfaState* from, DfaState* to)
{
}

void ReadToken(DfaState* startingState, const char* stream, Token& outToken)
{
}

void DeleteStateAndChildren(DfaState* root)
{
}

void ReadLanguageToken(DfaState* startingState, const char* stream, Token& outToken)
{
}

DfaState* CreateLanguageDfa()
{
  return nullptr;
}
