#include "spark/collections/stringref.h"
#include "spark/compiler/context.h"
#include "spark/scope/modulepathscope.h"
#include "spark/sema/types/essentials.h"
#include "spark/semgraph/module.h"
#include "spark/semgraph/package.h"
#include "spark/semgraph/type.h"

namespace spark {
namespace sema {
namespace types {
using collections::StringRef;
using semgraph::Composite;
using semgraph::Defn;
using semgraph::Member;
using semgraph::Module;
using semgraph::Package;
using semgraph::Type;
using semgraph::TypeDefn;

struct Essential {
  Essentials::Type id;
  StringRef path;
};

static Essential essentialTypes[] = {
  { Essentials::Type::ANY, "spark.core.any.Any" },
  { Essentials::Type::ENUM, "spark.core.enumeration.Enum" },
  { Essentials::Type::OBJECT, "spark.core.object.Object" },
};

Essentials::Essentials(compiler::Context* context)
  : _context(context)
{
  for (int i = 0; i < (int)Type::MAX; ++i) {
    _types[i] = nullptr;
  }
}

void Essentials::load() {
  for (auto& e : ArrayRef<Essential>(essentialTypes)) {
    auto m = findAbsoluteSymbol(e.path);
    assert(m->kind() == Defn::Kind::TYPE);
    _types[(int)e.id] = static_cast<Composite*>(static_cast<TypeDefn*>(m)->type());
  }
#if 0
    '''Load the essential types (String, Iterator, etc.) that are needed by the compiler.'''
    types = self.typeStore.getEssentialTypes()
    if len(types) == 0:
      functionNames = self.typeStore.getEssentialDefnNames()
      for name, path in functionNames.items():
        symbols = scopeStack.findAbsoluteSymbol(path)
#         assert len(symbols) == 1, (path, len(symbols))
        self.typeStore.essentialDefns[name] = symbols
      primitivetypes.OBJECT = types['object'].getType()
      primitivetypes.ANY = types['any'].getType()
      scopeStack.pop(2)
#endif
}

semgraph::Composite* Essentials::get(Essentials::Type id) {
  return _types[(int)id];
}

Member* Essentials::findAbsoluteSymbol(const StringRef& path) {
  int pos = 0;
  int end = 0;
  scope::SymbolScope* scope = _context->modulePathScope();
  Member* m = nullptr;
  while (pos < path.size()) {
    end = path.find('.', pos);
    if (end <= pos) {
      end = path.size();
    }
    StringRef part = path.substr(pos, end);
    std::vector<Member*> members;
    scope->lookupName(part, members);
    if (members.empty()) {
      _context->reporter().error() << "Essential name '" << path << "' not found.";
      return nullptr;
    }
    assert(members.size() == 1);
    m = members[0];
    pos = end + 1;
    if (pos >= path.size()) {
      break;
    }
    if (m->kind() == Member::Kind::PACKAGE) {
      scope = static_cast<Package*>(m)->memberScope();
    } else if (m->kind() == Member::Kind::MODULE) {
      scope = static_cast<Module*>(m)->memberScope();
    }
  }
  return m;
}

}}}
