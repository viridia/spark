// ============================================================================
// semgraph/exprvisitor.h: Graph traversal for exprs.
// ============================================================================

#ifndef SPARK_SEMGRAPH_EXPRVISITOR_H
#define SPARK_SEMGRAPH_EXPRVISITOR_H 1

#ifndef SPARK_SEMGRAPH_EXPR_H
  #include "spark/semgraph/expr.h"
#endif

namespace spark {
namespace semgraph {

template<typename ReturnType, typename... Args>
class ExprVisitor {
public:
  /** Call the appropriate visitor function for this definition. */
  ReturnType operator()(Expr* e, Args&&... args) {
    return exec(e, std::forward<Args>(args)...);
  }

  /** Call the appropriate visitor function for this definition. */
  ReturnType exec(Expr* e, Args&&... args) {
    switch (e->kind()) {
      case Expr::Kind::INVALID:
        return visitInvalid(e, std::forward<Args>(args)...);
      case Expr::Kind::IGNORED:
        return visitIgnored(e, std::forward<Args>(args)...);
      case Expr::Kind::CALL:
        return visitCall(static_cast<Call*>(e), std::forward<Args>(args)...);
      case Expr::Kind::SPECIALIZE:
        return visitSpecialize(static_cast<Call*>(e), std::forward<Args>(args)...);
      case Expr::Kind::MEMBER_SET:
        return visitMemberSet(static_cast<MemberSet*>(e), std::forward<Args>(args)...);
      case Expr::Kind::PACK:
        return visitPack(static_cast<MultiArgOp*>(e), std::forward<Args>(args)...);
      case Expr::Kind::CONST_TYPE:
        return visitConstType(static_cast<UnaryOp*>(e), std::forward<Args>(args)...);
      case Expr::Kind::PROVISIONAL_CONST_TYPE:
        return visitProvisionalConstType(static_cast<UnaryOp*>(e), std::forward<Args>(args)...);
      case Expr::Kind::UNION_TYPE:
        return visitUnionType(static_cast<MultiArgOp*>(e), std::forward<Args>(args)...);
      case Expr::Kind::FUNCTION_TYPE:
        return visitFunctionType(static_cast<Call*>(e), std::forward<Args>(args)...);
      case Expr::Kind::OPTIONAL:
        return visitOptionalType(static_cast<UnaryOp*>(e), std::forward<Args>(args)...);
      default:
        assert(false && "Invalid expression kind.");
        return ReturnType();
    }
  }

  /** Call the appropriate visitor functions for a collection of definitions. */
  void exec(const collections::ArrayRef<Expr*>& exprs, Args&&... args) {
    for (Expr* e : exprs) {
      exec(e, std::forward<Args>(args)...);
    }
  }

  /** Map an array of exprs into some result type. */
  void map(
      const collections::ArrayRef<Expr*>& exprs,
      std::vector<ReturnType>& results,
      Args&&... args) {
    for (Expr* e : exprs) {
      results.push_back(exec(e, std::forward<Args>(args)...));
    }
  }

  virtual ReturnType visitExpr(Expr* e, Args&&... args) { return ReturnType(); }
  virtual ReturnType visitInvalid(Expr* e, Args&&... args) {
    return visitExpr(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitIgnored(Expr* e, Args&&... args) {
    return visitExpr(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitUnaryOp(UnaryOp* e, Args&&... args) {
    return visitExpr(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitMultiArgOp(MultiArgOp* e, Args&&... args) {
    return visitExpr(e, std::forward<Args>(args)...);
  }

  virtual ReturnType visitCall(Call* e, Args&&... args) {
    return visitExpr(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitSpecialize(Call* e, Args&&... args) {
    return visitExpr(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitMemberSet(MemberSet* e, Args&&... args) {
    return visitExpr(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitPack(MultiArgOp* e, Args&&... args) {
    return visitMultiArgOp(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitConstType(UnaryOp* e, Args&&... args) {
    return visitUnaryOp(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitProvisionalConstType(UnaryOp* e, Args&&... args) {
    return visitUnaryOp(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitUnionType(MultiArgOp* e, Args&&... args) {
    return visitMultiArgOp(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitFunctionType(Call* e, Args&&... args) {
    return visitExpr(e, std::forward<Args>(args)...);
  }
  virtual ReturnType visitOptionalType(UnaryOp* e, Args&&... args) {
    return visitUnaryOp(e, std::forward<Args>(args)...);
  }
};

}}

#endif
