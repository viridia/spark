// ============================================================================
// sema/names/resolvetypes.h.
// ============================================================================

#ifndef SPARK_SEMA_NAMES_RESOLVEEXPRS_H
#define SPARK_SEMA_NAMES_RESOLVEEXPRS_H 1

#ifndef SPARK_SEMGRAPH_EXPR_H
  #include "spark/semgraph/expr.h"
#endif

#include "spark/support/arraybuilder.h"

#ifndef SPARK_HAS_UTILITY
  #include <utility>
#endif

namespace spark {
namespace ast {
  class Node;
  class UnaryOp;
  class Oper;
  class IntegerLiteral;
  class FloatLiteral;
  class TextLiteral;
  class ValueDefn;
}
namespace error { class Reporter; }
namespace scope { class ScopeStack; }
namespace support { class Arena; }
namespace sema {
namespace types { class TypeStore; }
namespace names {
using error::Reporter;
using semgraph::Expr;
class Subject;

/** Name resolver specialized for resolving types. */
class ResolveExprs {
public:
  ResolveExprs(
      Reporter& reporter,
      Subject& subject,
      scope::ScopeStack* scopeStack,
      types::TypeStore* typeStore,
      support::Arena& arena)
    : _reporter(reporter)
    , _subject(subject)
    , _scopeStack(scopeStack)
    , _typeStore(typeStore)
    , _arena(arena)
  {}

  Expr* exec(const ast::Node* node);

  /** The current enclosing definition. */
  Subject& subject() { return _subject; }

private:
  Expr* visitIdent(const ast::Ident* node, bool emptyResultOk);
  Expr* visitMemberRef(const ast::MemberRef* node);
  Expr* visitSelfRef(const ast::UnaryOp* node);
  Expr* visitSelf(const ast::Node* node);
  Expr* visitBuiltinType(const ast::BuiltInType* node);
  Expr* visitKeywordArg(const ast::Oper* node);
  Expr* visitNullLiteral(const ast::Node* node);
  Expr* visitBooleanLiteral(const ast::Node* node, bool value);
  Expr* visitIntegerLiteral(const ast::IntegerLiteral* node);
  Expr* visitFloatLiteral(const ast::FloatLiteral* node);
  Expr* visitStringLiteral(const ast::TextLiteral* node);
  Expr* visitCharLiteral(const ast::TextLiteral* node);
  Expr* visitUnaryOp(const ast::UnaryOp* node, Expr::Kind kind);

  Expr* visitAdd(const ast::Oper* node);
  Expr* visitSub(const ast::Oper* node);
  Expr* visitMul(const ast::Oper* node);
  Expr* visitDiv(const ast::Oper* node);
  Expr* visitMod(const ast::Oper* node);
  Expr* visitBitAnd(const ast::Oper* node);
  Expr* visitBitOr(const ast::Oper* node);
  Expr* visitBitXor(const ast::Oper* node);
  Expr* visitLShift(const ast::Oper* node);
  Expr* visitRShift(const ast::Oper* node);

  Expr* visitEqual(const ast::Oper* node);
  Expr* visitRefEqual(const ast::Oper* node);
  Expr* visitNotEqual(const ast::Oper* node);
  Expr* visitLessThan(const ast::Oper* node);
  Expr* visitGreaterThan(const ast::Oper* node);
  Expr* visitLessThanOrEqual(const ast::Oper* node);
  Expr* visitGreaterThanOrEqual(const ast::Oper* node);
  Expr* visitAsType(const ast::Oper* node);
  Expr* visitIs(const ast::Oper* node);
  Expr* visitIsNot(const ast::Oper* node);

  Expr* visitLogicalAnd(const ast::Oper* node);
  Expr* visitLogicalOr(const ast::Oper* node);

  Expr* visitReturn(const ast::UnaryOp* node);
  Expr* visitThrow(const ast::UnaryOp* node);
  Expr* visitTuple(const ast::Oper* node);
  Expr* visitUnion(const ast::Oper* node);
  Expr* visitConst(const ast::UnaryOp* node);
  Expr* visitSpecialize(const ast::Oper* node);
  Expr* visitCall(const ast::Oper* node);
  Expr* visitFunctionType(const ast::Oper* node);

  Expr* visitAssign(const ast::Oper* node);
  Expr* visitAssignAdd(const ast::Oper* node);
  Expr* visitAssignSub(const ast::Oper* node);
  Expr* visitAssignMul(const ast::Oper* node);
  Expr* visitAssignDiv(const ast::Oper* node);
  Expr* visitAssignMod(const ast::Oper* node);
  Expr* visitAssignBitAnd(const ast::Oper* node);
  Expr* visitAssignBitOr(const ast::Oper* node);
  Expr* visitAssignBitXor(const ast::Oper* node);
  Expr* visitAssignLShift(const ast::Oper* node);
  Expr* visitAssignRShift(const ast::Oper* node);
  Expr* visitPreInc(const ast::UnaryOp* node);
  Expr* visitPostInc(const ast::UnaryOp* node);
  Expr* visitPreDec(const ast::UnaryOp* node);
  Expr* visitPostDec(const ast::UnaryOp* node);
  Expr* visitRange(const ast::Oper* node);

  Expr* visitBlock(const ast::Oper* node);
  Expr* visitLocalVar(const ast::ValueDefn* node);
  Expr* visitIf(const ast::ControlStmt* node);
  Expr* visitWhile(const ast::ControlStmt* node);
  Expr* visitLoop(const ast::ControlStmt* node);
  Expr* visitFor(const ast::ControlStmt* node);
  Expr* visitForIn(const ast::ControlStmt* node);
  Expr* visitSwitch(const ast::ControlStmt* ast);
  Expr* visitMatch(const ast::ControlStmt* ast);
  Expr* visitBreak(const ast::Node* node);
  Expr* visitContinue(const ast::Node* node);

  Expr* relationalOp(const ast::Oper* node, const collections::StringRef& name);
  Expr* evalRelational(const source::Location& loc, ast::Kind kind, Expr* left, Expr* right);
  Expr* arithmeticOp(const ast::Oper* node, const collections::StringRef& name);
  Expr* evalArithmetic(const source::Location& loc, ast::Kind kind, Expr* left, Expr* right);
  Expr* binaryOp(const ast::Oper* node, Expr::Kind kind);
  Expr* augmentedAssign(const ast::Oper* ast, const collections::StringRef& name);
  Expr* preModify(const ast::UnaryOp* ast, const collections::StringRef& opName);
  Expr* postModify(const ast::UnaryOp* ast, const collections::StringRef& opName);
  std::tuple<Expr*, Expr*> getReadModifyWriteLocation(Expr* expr);
  std::tuple<Expr*, Expr*> cacheExpr(Expr* expr);
  bool isSimpleExpr(Expr* expr);
  semgraph::TempVarRef* storeTemp(Expr* value);
  void stmtVars(const ast::ValueDefn* var, support::ArrayBuilder<semgraph::ValueDefn*>& result);

  Reporter& _reporter;
  Subject& _subject;
  scope::ScopeStack* _scopeStack;
  types::TypeStore* _typeStore;
  support::Arena& _arena;
  std::vector<scope::SymbolScope*> _localScopes;
};

}}}

#endif
