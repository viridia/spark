// ============================================================================
// sema/names/resolvetypes.h.
// ============================================================================

#ifndef SPARK_SEMA_NAMES_RESOLVETYPES_H
#define SPARK_SEMA_NAMES_RESOLVETYPES_H 1

#ifndef SPARK_SEMGRAPH_EXPRVISITOR_H
  #include "spark/semgraph/exprvisitor.h"
#endif

namespace spark {
namespace compiler { class TypeStore; }
namespace error { class Reporter; }
namespace sema {
namespace names {
using error::Reporter;
using semgraph::Type;
using semgraph::Expr;
using semgraph::Call;
using semgraph::MemberSet;

/** Name resolver specialized for resolving types. */
class ResolveTypes : public semgraph::ExprVisitor<Type*> {
public:
  ResolveTypes(
      Reporter& reporter,
      compiler::TypeStore* typeStore,
      support::Arena& arena)
    : _reporter(reporter)
    , _typeStore(typeStore)
    , _arena(arena)
  {
    (void)_arena;
    (void)_typeStore;
  }

private:
  Type* visitExpr(Expr* e) final;
  Type* visitInvalid(Expr* e) final;
  Type* visitSpecialize(Call* e) final;
  Type* visitMemberSet(MemberSet* e) final;

  Reporter& _reporter;
  compiler::TypeStore* _typeStore;
  support::Arena& _arena;
};

}}}

#endif
