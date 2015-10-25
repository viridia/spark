// ============================================================================
// scope/stdscope.h: A standard scope.
// ============================================================================

#ifndef SPARK_SCOPE_STDSCOPE_H
#define SPARK_SCOPE_STDSCOPE_H 1

#ifndef SPARK_SCOPE_SCOPE_H
  #include "spark/scope/scope.h"
#endif

#ifndef SPARK_COLLECTIONS_HASHING_H
  #include "spark/collections/hashing.h"
#endif

#if SPARK_HAVE_STRING
  #include <string>
#endif

#if SPARK_HAVE_UNORDERED_MAP
  #include <unordered_map>
#endif

namespace spark {
namespace semgraph {
class Member;
}
namespace scope {
using collections::StringRef;
using semgraph::Member;

class StandardScope : public SymbolScope {
public:
  StandardScope(ScopeType st) : _scopeType(st), _owner(NULL) {}
  StandardScope(ScopeType st, const StringRef& description)
    : _scopeType(st)
    , _description(description.begin(), description.end())
    , _owner(NULL)
  {}
  StandardScope(ScopeType st, const semgraph::Member* owner)
    : _scopeType(st)
    , _owner(owner)
  {}

  /** Add a member to this scope. */
  void addMember(semgraph::Member* m);

  ScopeType scopetype() const { return _scopeType; }
  void lookupName(const StringRef& name, std::vector<Member*> &result) const;
  void forAllNames(NameFunctor& nameFn) const;
  void describe(std::ostream& strm) const;
protected:
  typedef std::unordered_map<StringRef, std::vector<Member*>> EntryMap;

  ScopeType _scopeType;
  EntryMap _entries;
  std::string _description;
  const semgraph::Member* _owner;
};

}}

#endif
