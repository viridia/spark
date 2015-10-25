// ============================================================================
// sema/names/resolvetypes.h.
// ============================================================================

#ifndef SPARK_SEMA_NAMES_RESOLVEEXPRS_H
#define SPARK_SEMA_NAMES_RESOLVEEXPRS_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

namespace spark {
namespace ast { class Node; class Oper; }
namespace error { class Reporter; }
namespace scope { class ScopeStack; }
namespace semgraph { class Expr; }
namespace support { class Arena; }
namespace sema {
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
      support::Arena& arena)
    : _reporter(reporter)
    , _subject(subject)
    , _scopeStack(scopeStack)
    , _arena(arena)
  {}

  Expr* exec(const ast::Node* node);

  /** The current enclosing definition. */
  Subject& subject() { return _subject; }

private:
  Expr* visitIdent(const ast::Ident* ast, bool emptyResultOk);
  Expr* visitMemberRef(const ast::MemberRef* ast);
  Expr* visitSpecialize(const ast::Oper* ast);
  Expr* visitBuiltinType(const ast::BuiltInType* ast);

  Reporter& _reporter;
  Subject& _subject;
  scope::ScopeStack* _scopeStack;
  support::Arena& _arena;
};

}}}

#endif
