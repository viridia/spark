// ============================================================================
// Standard Scope implementation.
// ============================================================================

#include "spark/scope/stdscope.h"
#include "spark/semgraph/defn.h"

namespace spark {
namespace scope {
using spark::collections::StringRef;

void StandardScope::addMember(semgraph::Member* m) {
  _entries[m->name()].push_back(m);
}

void StandardScope::lookupName(const StringRef& name, std::vector<Member*>& result) const {
  EntryMap::const_iterator it = _entries.find(name);
  if (it != _entries.end()) {
    result.insert(result.end(), it->second.begin(), it->second.end());
  }
}

void StandardScope::forAllNames(NameFunctor& nameFn) const {
  for (EntryMap::value_type v : _entries) {
    nameFn(v.first);
  }
}

void StandardScope::describe(std::ostream& strm) const {
  if (_member) {
    if (_description.size() > 0) {
      strm << _description << " scope for " << _member->name();
    } else {
      strm << "scope for " << _member->name();
    }
  } else if (_description.size() > 0) {
    strm << _description << " scope";
  } else {
    strm << "scope containing " << _entries.size() << " members";
  }
}

}}
