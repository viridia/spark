#include "spark/sema/names/subject.h"
#include "spark/semgraph/defn.h"

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace sema {
namespace names {
using namespace semgraph;

bool Subject::isVisible(Member* target) {
  while (target->kind() == Member::Kind::SPECIALIZED) {
    target = static_cast<SpecializedMember*>(target)->generic();
  }

  // Packages and modules are always visible
  if (target->kind() == Member::Kind::PACKAGE || target->kind() == Member::Kind::MODULE) {
    return true;
  }
  Defn* targetDefn = static_cast<Defn*>(target);

  // Is it public
  if (targetDefn->visibility() == PUBLIC) {
    return true;
  }

  // If the scope that mObject is defined in is either mObject or an enclosing scope of mObject.
  assert(target->definedIn() != NULL);
  auto targetScope = target->definedIn();
  if (containsSubject(targetScope)) {
    return true;
  }

#if 0
  if (target->visibility() == PROTECTED) {
    // Do base type test
    if isinstance(targetScope, graph.TypeDefn) and
          isinstance(targetScope.getType(), graph.Composite):
        targetCls = targetScope.getType()
        for subjectAncestor in defns.ancestorDefs(self.subject):
          if isinstance(subjectAncestor, graph.TypeDefn) and
              isinstance(subjectAncestor.getType(), graph.Composite):
            subjectCls = subjectAncestor.getType()
            if types.isSubtype(subjectCls, targetCls):
              return True
  }

  // Check friend declarations
  for friend in targetScope.getFriends():
    if self.contains(self.subject, friend):
      return True
#endif

  return false;
}

bool Subject::containsSubject(Member* container) {
  for (Member* sym = _value; sym != nullptr; sym = sym->definedIn()) {
    if (sym == container) {
      return true;
    }
  }
  return false;
}

}}}
