#include "spark/scope/specializedscope.h"
#include "spark/semgraph/defn.h"

#if SPARK_HAVE_UNORDERED_SET
  #include <unordered_set>
#endif

#ifndef SPARK_SEMA_TYPES_APPLYENV_H
  #include "spark/sema/types/applyenv.h"
#endif

namespace spark {
namespace scope {
using spark::collections::StringRef;

void SpecializedScope::addMember (Member* m) {
  assert(false && "not implemented");
}

void SpecializedScope::lookupName(const StringRef& name, std::vector<Member*> &result) const {
  std::vector<Member*> members;
  _primary->lookupName(name, members);
  sema::types::ApplyEnv apply(_typeStore);
  for (auto m : members) {
    auto spec = apply.specializeMember(m, _env.bindings());
    result.push_back(spec);
  }
}

void SpecializedScope::forAllNames(NameFunctor& nameFn) const {
  _primary->forAllNames(nameFn);
}

void SpecializedScope::describe(std::ostream& strm) const {
  strm << "specialized scope for ";
  _primary->describe(strm);
}

}}
