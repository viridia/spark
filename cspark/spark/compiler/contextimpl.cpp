#include "spark/compiler/contextimpl.h"
#include "spark/compiler/compiler.h"
#include "spark/scope/modulepathscope.h"
#include "spark/sema/types/essentials.h"
#include "spark/sema/types/typestore.h"

namespace spark {
namespace compiler {

ContextImpl::ContextImpl(Reporter& reporter, Compiler& compiler)
  : _reporter(reporter)
  , _moduleSetsChanged(false)
  , _modulePathScope(new scope::ModulePathScope())
  , _typeStore(new sema::types::TypeStore())
  , _essentials(new sema::types::Essentials(this))
  , _compiler(compiler)
{}

semgraph::Module* ContextImpl::importModuleFromSource(const support::Path& path) {
  return _compiler.parseImportSource(path);
}

}}
