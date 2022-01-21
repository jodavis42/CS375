/******************************************************************\
 * Author: Trevor Sundberg
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include "Variant.hpp"
#include "Driver4.hpp"
#include <iostream>

Variant::Variant()
{
  Clear();
}

std::string Variant::ToString()
{
  std::stringstream stream;

  // Typically you would place this code on an interface on the Type
  // Which allows Types to control how they print out themselves
  // e.g. mType->ToString()
  if (mType == IntegerType)
    stream << mValue.mInteger;
  else if (mType == FloatType)
    stream << mValue.mFloat;
  else if (mType == BooleanType)
    stream << mValue.mBoolean;
  else if (mType == ByteType)
    stream << mValue.mByte;
  else
    stream << mType->mName;

  return stream.str();
}

void Variant::Print()
{
  std::cout << ToString();
}

void Variant::Clear()
{
  memset(this, 0, sizeof(*this));
  mType = NullType;
  mPointerToValue = nullptr;
}
