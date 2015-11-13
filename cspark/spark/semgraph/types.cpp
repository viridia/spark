#include "spark/semgraph/types.h"

namespace spark {
namespace semgraph {
namespace types {

Type* raw(Type* t) {
  for (;;) {
    if (t->kind() == Type::Kind::SPECIALIZED) {
      t = static_cast<SpecializedType*>(t)->generic();
    } else if (t->kind() == Type::Kind::CONST) {
      t = static_cast<ConstType*>(t)->base();
    } else {
      return t;
    }
  }
}

}}}
