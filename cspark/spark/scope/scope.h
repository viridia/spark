// ============================================================================
// scope/scope.h: Symbol table scopes.
// ============================================================================

#ifndef SPARK_SCOPE_SCOPE_H
#define SPARK_SCOPE_SCOPE_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

#if SPARK_HAVE_OSTREAM
  #include <ostream>
#endif

#if SPARK_HAVE_VECTOR
  #include <vector>
#endif

namespace spark {
namespace semgraph {
class Member;
}
namespace scope {
using collections::StringRef;
using semgraph::Member;

/** A function that takes a symbol name. */
class NameFunctor {
public:
  virtual void operator()(const StringRef& name) = 0;
};

class SymbolScope {
public:
  enum ScopeType {
    DEFAULT,        // Scope representing static members of some definition or module.
    INSTANCE,       // Scope representing instance variables of some type.
    LOCAL           // Local scope such as a block.
  };

  virtual ~SymbolScope() {}

  /** The general type of this scope. */
  virtual ScopeType scopetype() const = 0;

  /** Add a member to this scope. Note that many scope implementations don't allow this. */
  virtual void addMember(semgraph::Member* m) = 0;

  /** Lookup a name, and produce a list of results for that name. */
  virtual void lookupName(const StringRef& name, std::vector<Member*> &result) const = 0;

  /** Lookup a name, and produce a list of results for that name. */
  const std::vector<Member*> lookupName(const StringRef& name) const {
    std::vector<Member*> result;
    lookupName(name, result);
    return result;
  }

  /** Call the specified functor for all names defined in this scope. */
  virtual void forAllNames(NameFunctor& nameFn) const = 0;

  /** Produce a description of this scope. */
  virtual void describe(std::ostream& strm) const = 0;
};

}}

#endif
