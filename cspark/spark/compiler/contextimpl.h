// ============================================================================
// compiler/contextimpl.h: Place to store global singletons.
// ============================================================================

#ifndef SPARK_COMPILER_CONTEXTIMPL_H
#define SPARK_COMPILER_CONTEXTIMPL_H 1

#ifndef SPARK_COMPILER_CONTEXT_H
  #include "spark/compiler/context.h"
#endif

#ifndef SPARK_SUPPORT_ARENA_H
  #include "spark/support/arena.h"
#endif

#ifndef SPARK_SEMA_TYPES_TYPESTORE_H
  #include "spark/sema/types/typestore.h"
#endif

#if SPARK_HAVE_MEMORY
  #include <memory>
#endif

namespace spark {
namespace compiler {
using spark::error::Reporter;

class Compiler;

/** Standard implementation of the Context interface. */
class ContextImpl : public Context {
public:
  ContextImpl(Reporter& reporter, Compiler& compiler);

  Reporter& reporter() const final { return _reporter; }
  Arena& arena() final { return _arena; }
  ModuleList& sourceModules() final { return _sourceModules; }
  ModuleList& sourceImportModules() final { return _sourceImportModules; }
  semgraph::Module* importModuleFromSource(const support::Path& path) final;
  bool moduleSetsChanged() const final { return _moduleSetsChanged; }
  void setModuleSetsChanged(bool changed) final { _moduleSetsChanged = changed; }
  scope::ModulePathScope* modulePathScope() const final { return _modulePathScope.get(); }
  sema::types::TypeStore* typeStore() const final { return _typeStore.get(); }
  sema::types::Essentials* essentials() const final { return _essentials.get(); }

private:
  Reporter& _reporter;
  Arena _arena;
  ModuleList _sourceModules;
  ModuleList _sourceImportModules;
  bool _moduleSetsChanged;
  std::auto_ptr<scope::ModulePathScope> _modulePathScope;
  std::auto_ptr<sema::types::TypeStore> _typeStore;
  std::auto_ptr<sema::types::Essentials> _essentials;
  Compiler& _compiler;
};

}}

#endif
