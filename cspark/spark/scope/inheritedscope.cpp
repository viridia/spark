#include "spark/scope/inheritedscope.h"
#include "spark/semgraph/defn.h"

#if SPARK_HAVE_UNORDERED_SET
  #include <unordered_set>
#endif

namespace spark {
namespace scope {
using spark::collections::StringRef;

void InheritedScope::addMember(Member* m) {
  assert(false && "not implemented");
}

void InheritedScope::lookupName(const StringRef& name, std::vector<Member*> &result) const {
  std::vector<Member*> members;
  _primary->lookupName(name, members);
  if (!members.empty()) {
    result.insert(result.end(), members.begin(), members.end());
    return;
  }

  std::unordered_set<Member*> seen;
  for (auto s : _secondary) {
    members.clear();
    s->lookupName(name, members);
    for (auto m : members) {
      // Filter duplicate entries.
      if (seen.count(m) == 0) {
        seen.insert(m);
        result.push_back(m);
      }
    }
  }
}

void InheritedScope::forAllNames(NameFunctor& nameFn) const {
  _primary->forAllNames(nameFn);
  for (auto s : _secondary) {
    s->forAllNames(nameFn);
  }
}

void InheritedScope::describe(std::ostream& strm) const {
  assert(_owner != NULL);
  strm << "inherited scope for " << _owner->name();
}

}}
