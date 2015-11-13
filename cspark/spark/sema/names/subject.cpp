#include "spark/sema/names/subject.h"
#include "spark/semgraph/defn.h"
#include "spark/semgraph/type.h"
#include "spark/support/casting.h"

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace sema {
namespace names {
using namespace semgraph;
using support::dyn_cast;

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

  if (targetDefn->visibility() == semgraph::PROTECTED) {
    if (auto td = dyn_cast<TypeDefn*>(targetScope)) {
      if (auto classDefiningTarget = dyn_cast<Composite*>(td->type())) {
        for (Member* sym = _value; sym != nullptr; sym = sym->definedIn()) {
          if (auto tds = dyn_cast<TypeDefn*>(sym)) {
            if (auto classContainingSubject = dyn_cast<Composite*>(tds->type())) {
              if (classContainingSubject->inheritsFrom(classDefiningTarget)) {
                return true;
              }
            }
          }
        }
      }
    }
  }
#if 0
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
