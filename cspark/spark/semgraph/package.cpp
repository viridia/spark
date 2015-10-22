// ============================================================================
// Semgraph Module.
// ============================================================================

#include "spark/semgraph/module.h"
#include "spark/semgraph/package.h"

namespace spark {
namespace semgraph {

void Package::addModule(Module* module) {
  _memberScope->addMember(module);
}

}}
