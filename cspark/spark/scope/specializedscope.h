// ============================================================================
// scope/specialized.h: A scope that transforms results through an environment.
// ============================================================================

#ifndef SPARK_SCOPE_SPECIALIZEDSCOPE_H
#define SPARK_SCOPE_SPECIALIZEDSCOPE_H 1

#ifndef SPARK_SCOPE_SCOPE_H
  #include "spark/scope/scope.h"
#endif

#ifndef SPARK_SEMA_TYPES_TYPESTORE_H
  #include "spark/sema/types/typestore.h"
#endif

namespace spark {
namespace scope {
using collections::StringRef;
using semgraph::Member;
using semgraph::Env;
using sema::types::TypeStore;

/** SpecializedScope enables name lookups in specialized templates, that is, templates that
    have been bound to an environment. This works by doing a normal lookup in the generic
    template's member scope, and then transforming any results through the same environment. */
class SpecializedScope : public SymbolScope {
public:
  SpecializedScope(SymbolScope* primary, Env& env, TypeStore* typeStore)
    : _primary(primary)
    , _env(env)
    , _typeStore(typeStore)
  {}

  /** The general type of this scope. */
  ScopeType scopetype() const { return _primary->scopetype(); }

  void addMember(Member* m);
  void lookupName(const StringRef& name, std::vector<Member*> &result) const;
  void forAllNames(NameFunctor& nameFn) const;
  void describe(std::ostream& strm) const;
private:
  SymbolScope* _primary;
  Env& _env;
  TypeStore* _typeStore;
};

}}

#endif
