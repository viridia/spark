#include "spark/compiler/contextimpl.h"
#include "spark/compiler/compiler.h"
#include "spark/scope/modulepathscope.h"

namespace spark {
namespace compiler {

ContextImpl::ContextImpl(Reporter& reporter, Compiler& compiler)
  : _reporter(reporter)
  , _moduleSetsChanged(false)
  , _modulePathScope(new scope::ModulePathScope())
  , _compiler(compiler)
{}

semgraph::Module* ContextImpl::importModuleFromSource(const support::Path& path) {
  return _compiler.parseImportSource(path);
}

TypeStore* ContextImpl::typeStore() const {
  return NULL;
}

}}
