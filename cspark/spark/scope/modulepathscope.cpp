#include "spark/scope/modulepathscope.h"

namespace spark {
namespace scope {

void ModulePathScope::addMember(semgraph::Member* m) {
  assert(false && "addMember() not implemented for ModulePathScope");
}

void ModulePathScope::lookupName(const StringRef& name, std::vector<Member*> &result) const {
  for (Importer* imp : _importers) {
    imp->lookupName(name, result);
  }
}

void ModulePathScope::describe(std::ostream& strm) const {
  strm << "<module path scope>";
}

}}
