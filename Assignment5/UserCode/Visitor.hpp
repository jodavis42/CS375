#pragma once

#include "../Drivers/AstNodes.hpp"
#include "../Drivers/Driver3.hpp"

#define ErrorIf(expression, str) if(expression) __debugbreak()

enum VisitResult
{
  Continue,
  Stop
};

class Visitor
{
public:
  //#define DeclareNodeVisitWithValue(NodeType, Value) virtual VisitResult Visit(NodeType* node) { VisitNodeWithValue(NodeType, Value); }
  //#define DeclareNodeVisit(NodeType) virtual VisitResult Visit(NodeType* node) { VisitNode(NodeType); }
#define DeclareNodeVisit(NodeType, BaseType) virtual VisitResult Visit(NodeType* node)\
{\
    /*node->Walk(this, false);*/                                    \
    Visit(static_cast<BaseType*>(node));\
    return VisitResult::Continue;\
}

  virtual VisitResult Visit(AbstractNode* node) { return VisitResult::Continue; }
  DeclareNodeVisit(BlockNode, AbstractNode);
  DeclareNodeVisit(ClassNode, AbstractNode);
  DeclareNodeVisit(StatementNode, AbstractNode);
  DeclareNodeVisit(VariableNode, StatementNode);
  DeclareNodeVisit(ScopeNode, StatementNode);
  DeclareNodeVisit(ParameterNode, VariableNode);
  DeclareNodeVisit(FunctionNode, AbstractNode);
  DeclareNodeVisit(TypeNode, AbstractNode);
  DeclareNodeVisit(PointerTypeNode, TypeNode);
  DeclareNodeVisit(ReferenceTypeNode, TypeNode);
  DeclareNodeVisit(NamedTypeNode, TypeNode);
  DeclareNodeVisit(FunctionTypeNode, TypeNode);
  DeclareNodeVisit(LabelNode, StatementNode);
  DeclareNodeVisit(GotoNode, StatementNode);
  DeclareNodeVisit(ReturnNode, StatementNode);
  DeclareNodeVisit(BreakNode, StatementNode);
  DeclareNodeVisit(ContinueNode, StatementNode);
  DeclareNodeVisit(ExpressionNode, StatementNode);
  DeclareNodeVisit(IfNode, StatementNode);
  DeclareNodeVisit(WhileNode, StatementNode);
  DeclareNodeVisit(ForNode, StatementNode);
  DeclareNodeVisit(LiteralNode, ExpressionNode);
  DeclareNodeVisit(NameReferenceNode, ExpressionNode);
  DeclareNodeVisit(BinaryOperatorNode, ExpressionNode);
  DeclareNodeVisit(UnaryOperatorNode, ExpressionNode);
  DeclareNodeVisit(PostExpressionNode, ExpressionNode);
  DeclareNodeVisit(MemberAccessNode, PostExpressionNode);
  DeclareNodeVisit(CallNode, PostExpressionNode);
  DeclareNodeVisit(CastNode, PostExpressionNode);
  DeclareNodeVisit(IndexNode, PostExpressionNode);

  template <typename NodeType> void TryWalk(NodeType* node)
  {
    if (node != nullptr)
      node->Walk(this);
  }

  template <typename NodeType> void TryWalk(std::unique_ptr<NodeType>& node)
  {
    if (node != nullptr)
      node->Walk(this);
  }

  template <typename NodeType> void Walk(unique_vector<NodeType>& nodes)
  {
    for (size_t i = 0; i < nodes.size(); ++i)
      nodes[i]->Walk(this);
  }
#undef DeclareNodeVisit
};