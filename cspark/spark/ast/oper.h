// ============================================================================
// oper.h: AST representing operators
// ============================================================================

#ifndef SPARK_AST_OPER_H
#define SPARK_AST_OPER_H 1

#ifndef SPARK_AST_NODE_H
#include <spark/ast/node.h>
#endif

namespace spark {
namespace type {
class Type;
}
namespace ast {
using spark::collections::StringRef;

/** Unary operator. */
class UnaryOp : public Node {
public:
  UnaryOp(Kind kind, const Location& location, const Node* arg)
    : Node(kind, location)
    , _arg(arg)
  {}

  /** The argumment to the unary operator. */
  const Node* arg() const { return _arg; }

  static inline bool classof(const UnaryOp*) { return true; }
  static inline bool classof(const Node* node) {
    return node->kind() >= Node::KIND_UNARY_START && node->kind() <= Node::KIND_UNARY_END;
  }
private:
  const Node* _arg;
};

/** N-ary operator. */
class Oper : public Node {
public:
  Oper(Kind kind, const Location& location, NodeList operands)
    : Node(kind, location)
    , _op(NULL)
    , _operands(operands)
  {}

  /** The operator. May be NULL if implied by the node type. */
  const Node* op() const { return _op; }
  void setOp(Node* n) { _op = n; }

  /** List of operands. */
  NodeList operands() const { return _operands; }
  void setOperands(const NodeList& operands) { _operands = operands; }

  static inline bool classof(const Oper*) { return true; }
  static inline bool classof(const Node* node) {
    return node->kind() >= Node::KIND_OPER_START && node->kind() <= Node::KIND_OPER_END;
  }
private:
  Node* _op;
  NodeList _operands;
};

/** An operator that has a test expression and multiple branches. */
class ControlStmt : public Node {
public:
  ControlStmt(Kind kind, const Location& location, const Node* test, NodeList outcomes)
    : Node(kind, location)
    , _test(test)
    , _outcomes(outcomes)
  {}

  /** The input test expression. */
  const Node* test() const { return _test; }

  /** List of arguments to the operator. */
  NodeList outcomes() const { return _outcomes; }

  static inline bool classof(const ControlStmt*) { return true; }
  static inline bool classof(const Node* node) {
    return node->kind() >= Node::KIND_CTRL_START && node->kind() <= Node::KIND_CTRL_END;
  }
private:
  const Node* _test;
  NodeList _outcomes;
};

// class CallRequired(Oper):
//   returnType = defscalar(Node)
//   static = defscalar(bool)
//
// # Type operators
// class Modified(Oper):
//   const = defscalar(bool)
//   transitiveConst = defscalar(bool)
//   variadic = defscalar(bool)
//   ref = defscalar(bool)
//
// class FunctionType(Node):
//   selfType = defscalar(Node)
//   returnType = defscalar(Node)
//   paramTypes = deflist(Node)

}}

#endif
