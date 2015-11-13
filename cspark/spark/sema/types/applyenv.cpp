#include "spark/sema/types/applyenv.h"

namespace spark {
namespace sema {
namespace types {
using semgraph::EnvMap;
using semgraph::Member;
// using semgraph::Type;
// using semgraph::TypeDefn;
// using semgraph::TypeParameter;
// using semgraph::ValueDefn;

Member* ApplyEnv::specializeMember(Member* m, const EnvMap& env) {
  assert(false && "implement ApplyEnv::specializeMember.");
}

}}}
