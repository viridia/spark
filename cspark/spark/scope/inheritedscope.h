// ============================================================================
// scope/inheritedscope.h: Inherited scopes.
// ============================================================================

#ifndef SPARK_SCOPE_INHERITEDSCOPE_H
#define SPARK_SCOPE_INHERITEDSCOPE_H 1

#ifndef SPARK_SCOPE_SCOPE_H
  #include "spark/scope/scope.h"
#endif

namespace spark {
namespace scope {
using collections::StringRef;
using semgraph::Member;

/** InheritedScope handles name lookups for class instance variables, where a name may be found
    in the class itself, or is inherited from one of its supertypes.

    The lookup algorithm works as follows:

    If the name being looked up is defined in the primary scope (that is, the scope of the class
    associated with this scope), then the result is just the the lookup result from the primary
    scope.

    If the name is not defined in the primary scope, then each of the immediate supertype scopes
    are search, and the result is the union of all those results. */
class InheritedScope : public SymbolScope {
public:
  InheritedScope(SymbolScope* primary, Member* owner) : _primary(primary), _owner(owner) {}

  /** The general type of this scope. */
  ScopeType scopetype() const { return INSTANCE; }

  /** Add a secondary scope. */
  void addScope(SymbolScope* s) {
    _secondary.push_back(s);
  }

  void addMember(Member* m);
  void lookupName(const StringRef& name, std::vector<Member*> &result) const;
  void forAllNames(NameFunctor& nameFn) const;
  void describe(std::ostream& strm) const;
private:
  SymbolScope* _primary;
  std::vector<SymbolScope*> _secondary;
  const semgraph::Member* _owner;
};

}}

#endif
