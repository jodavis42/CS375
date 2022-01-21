#pragma once

#include "Visitor.hpp"
#include "../Drivers/Driver4.hpp"

class SymbolPrinterVisitor : public Visitor
{
public:
  
  SymbolPrinterVisitor()
  {
  }

  /*virtual VisitResult Visit(ClassNode* node)
  {
    NodePrinter printer;
    printer << "ClassNode" << "(" << node->mSymbol << ")";
    return VisitResult::Continue;
  }

  virtual VisitResult Visit(PointerTypeNode* node)
  {
    NodePrinter printer;
    printer << node->mSymbol;
    return VisitResult::Continue;
  }

  virtual VisitResult Visit(ReferenceTypeNode* node)
  {
    NodePrinter printer;
    printer << node->mSymbol;
    return VisitResult::Continue;
  }*/

  #define DeclareNodeVisit(NodeType) virtual VisitResult Visit(NodeType* node)\
{\
  NodePrinter printer;                                        \
    printer << #NodeType;\
    PrintValues(printer, node);\
    node->Walk(this, false);                                    \
    return VisitResult::Stop;\
}


#define PrintNode0(NodeType) virtual VisitResult Visit(NodeType* node)\
{\
  NodePrinter printer;                                        \
    printer << #NodeType;\
    node->Walk(this, false);                                    \
    return VisitResult::Stop;\
}

#define PrintNode1(NodeType, Arg1) virtual VisitResult Visit(NodeType* node)\
{\
  NodePrinter printer;                                        \
    printer << #NodeType;\
    printer << "(" << node->Arg1 << ")";\
    node->Walk(this, false);                                    \
    return VisitResult::Stop;\
}


#define PrintNode2(NodeType, Arg1, Arg2) virtual VisitResult Visit(NodeType* node)\
{\
  NodePrinter printer;                                        \
    printer << #NodeType;\
    printer << "(" << node->Arg1 << ", " << node->Arg2 << ")";\
    node->Walk(this, false);                                    \
    return VisitResult::Stop;\
}


#define PrintNode3(NodeType, Arg1, Arg2, Arg3) virtual VisitResult Visit(NodeType* node)\
{\
  NodePrinter printer;                                        \
    printer << #NodeType;\
    printer << "(" << node->Arg1 << ", " << node->Arg2 << ", " << node->Arg3 << ")";\
    node->Walk(this, false);                                    \
    return VisitResult::Stop;\
}

  PrintNode0(BlockNode);
  PrintNode1(ClassNode, mSymbol);
  PrintNode1(VariableNode, mSymbol);
  PrintNode1(ParameterNode, mSymbol);
  PrintNode1(FunctionNode, mSymbol);
  PrintNode1(TypeNode, mSymbol);
  virtual VisitResult Visit(PointerTypeNode* node) { return Visit((TypeNode*)node); }
  virtual VisitResult Visit(ReferenceTypeNode* node) { return Visit((TypeNode*)node); }
  virtual VisitResult Visit(FunctionTypeNode* node) { return Visit((TypeNode*)node); }
  virtual VisitResult Visit(NamedTypeNode* node) { return Visit((TypeNode*)node); }
  PrintNode0(ScopeNode);
  PrintNode1(LabelNode, mSymbol);
  PrintNode1(GotoNode, mResolvedLabel);
  PrintNode1(ExpressionNode, mResolvedType);
  PrintNode0(IfNode);
  PrintNode0(WhileNode);
  PrintNode0(ForNode);
  PrintNode0(BreakNode);
  PrintNode0(ContinueNode);
  PrintNode2(LiteralNode, mToken, mResolvedType);
  PrintNode2(NameReferenceNode, mName, mResolvedType);
  PrintNode2(BinaryOperatorNode, mOperator, mResolvedType);
  PrintNode2(UnaryOperatorNode, mOperator, mResolvedType);
  PrintNode1(PostExpressionNode, mResolvedType);
  PrintNode3(MemberAccessNode, mOperator, mResolvedMember, mResolvedType);
  PrintNode1(CallNode, mResolvedType);
  PrintNode1(CastNode, mResolvedType);
  PrintNode1(IndexNode, mResolvedType);
  PrintNode0(ReturnNode);


//
//#define VisitNode(NodeType)                   \
//  NodePrinter printer;                                        \
//  printer << #NodeType ;\
//  node->Walk(this, false);                                    \
//  return VisitResult::Stop;
//
//#define VisitNodeWithValue(NodeType, Value)                   \
//  NodePrinter printer;                                        \
//  printer << #NodeType "(" << Value << ")"; \
//  node->Walk(this, false);                                    \
//  return VisitResult::Stop;
//
//  //#define DeclareNodeVisitWithValue(NodeType, Value) virtual VisitResult Visit(NodeType* node) { VisitNodeWithValue(NodeType, Value); }
//  //#define DeclareNodeVisit(NodeType) virtual VisitResult Visit(NodeType* node) { VisitNode(NodeType); }
//#define DeclareNodeVisit(NodeType) virtual VisitResult Visit(NodeType* node)\
//{\
//  NodePrinter printer;                                        \
//    printer << #NodeType;\
//    PrintValues(printer, node);\
//    node->Walk(this, false);                                    \
//    return VisitResult::Stop;\
//}
//
//  DeclareNodeVisit(AbstractNode);
//  DeclareNodeVisit(BlockNode);
//  DeclareNodeVisit(ClassNode);
//  DeclareNodeVisit(StatementNode);
//  DeclareNodeVisit(VariableNode);
//  DeclareNodeVisit(ScopeNode);
//  DeclareNodeVisit(ParameterNode);
//  DeclareNodeVisit(FunctionNode);
//  DeclareNodeVisit(TypeNode);
//  DeclareNodeVisit(PointerTypeNode);
//  DeclareNodeVisit(ReferenceTypeNode);
//  //DeclareNodeVisit(NamedTypeNode);
//  //DeclareNodeVisit(FunctionTypeNode);
//  DeclareNodeVisit(LabelNode);
//  DeclareNodeVisit(GotoNode);
//  DeclareNodeVisit(ReturnNode);
//  DeclareNodeVisit(BreakNode);
//  DeclareNodeVisit(ContinueNode);
//  DeclareNodeVisit(ExpressionNode);
//  DeclareNodeVisit(IfNode);
//  DeclareNodeVisit(WhileNode);
//  DeclareNodeVisit(ForNode);
//  DeclareNodeVisit(LiteralNode);
//  DeclareNodeVisit(NameReferenceNode);
//  DeclareNodeVisit(BinaryOperatorNode);
//  DeclareNodeVisit(UnaryOperatorNode);
//  DeclareNodeVisit(PostExpressionNode);
//  DeclareNodeVisit(MemberAccessNode);
//  DeclareNodeVisit(CallNode);
//  DeclareNodeVisit(CastNode);
//  DeclareNodeVisit(IndexNode);
//
//#undef VisitNode
//#undef VisitNodeWithValue
//#undef DeclareNodeVisit
};
