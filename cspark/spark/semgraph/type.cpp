#include "spark/semgraph/type.h"
#include "spark/semgraph/types.h"
#include "spark/support/casting.h"

namespace spark {
namespace semgraph {
using support::dyn_cast;

Type Type::ERROR(Type::Kind::INVALID);
Type Type::IGNORED(Type::Kind::IGNORED);

bool Composite::inheritsFrom(Type* super) {
  if (super == this) {
    return true;
  }
  if (_superType != nullptr) {
    if (auto c = dyn_cast<Composite*>(types::raw(_superType))) {
      if (c->inheritsFrom(super)) {
        return true;
      }
    }
  }
  for (auto iface : _interfaces) {
    if (auto c = dyn_cast<Composite*>(types::raw(iface))) {
      if (c->inheritsFrom(super)) {
        return true;
      }
    }
  }
  return false;
}

}}
