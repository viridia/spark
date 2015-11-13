// ============================================================================
// sema/names/resolvetypes.h.
// ============================================================================

#ifndef SPARK_SEMA_NAMES_RESOLVETYPES_H
#define SPARK_SEMA_NAMES_RESOLVETYPES_H 1

#ifndef SPARK_SEMGRAPH_EXPRVISITOR_H
  #include "spark/semgraph/exprvisitor.h"
#endif

#ifndef SPARK_SEMA_TYPES_APPLYENV_H
  #include "spark/sema/types/applyenv.h"
#endif

namespace spark {
namespace error { class Reporter; }
namespace sema {
namespace types {
class TypeStore;
}
namespace names {
using error::Reporter;
using namespace semgraph;

/** Name resolver specialized for resolving types. */
class ResolveTypes : public semgraph::ExprVisitor<Type*> {
public:
  ResolveTypes(
      Reporter& reporter,
      sema::types::TypeStore* typeStore,
      support::Arena& arena)
    : _reporter(reporter)
    , _typeStore(typeStore)
    , _arena(arena)
    , _apply(typeStore)
  {
    assert(_typeStore != nullptr);
  }

private:
  Type* visitExpr(Expr* e) final;
  Type* visitInvalid(Expr* e) final;
  Type* visitSpecialize(Call* e) final;
  Type* visitMemberSet(MemberSet* e) final;
  Type* visitPack(MultiArgOp* e) final;
  Type* visitConstType(UnaryOp* e) final;
  Type* visitProvisionalConstType(UnaryOp* e) final;
  Type* visitUnionType(MultiArgOp* e) final;
  Type* visitFunctionType(Call* e);
  Type* visitOptionalType(UnaryOp* e) final;

  void specializeMember(Call* e);
  bool specialize(
    const collections::ArrayRef<semgraph::TypeParameter*>& params,
    const collections::ArrayRef<Expr*>& args,
    semgraph::Env& result);

  Reporter& _reporter;
  sema::types::TypeStore* _typeStore;
  support::Arena& _arena;
  types::ApplyEnv _apply;
};

}}}

#endif
