// ============================================================================
// compiler/context.h: Place to store global singletons.
// ============================================================================

#ifndef SPARK_COMPILER_CONTEXT_H
#define SPARK_COMPILER_CONTEXT_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_ERROR_REPORTER_H
  #include "spark/error/reporter.h"
#endif

#ifndef SPARK_SUPPORT_PATH_H
  #include "spark/support/path.h"
#endif

namespace spark {
namespace scope {
class ModulePathScope;
}
namespace semgraph {
class Module;
}
namespace support {
class Arena;
}
namespace compiler {
using spark::error::Reporter;
using spark::support::Arena;

class TypeStore;

typedef std::vector<semgraph::Module*> ModuleList;
// typedef std::unordered_set<semgraph::Module*> ModuleSet;

class Context {
public:
  /** The error reporter for this context. */
  virtual Reporter& reporter() const = 0;

  /** The global arena for long-lived objects. */
  virtual Arena& arena() = 0;

  /** The list of source modules to be compiled. Initially this will be the set of source files
      specified on the command line, but the list may grow if uncompiled sources are imported. */
  virtual ModuleList& sourceModules() = 0;

  /** The list of source modules that were loaded in order to resolve an imported symbol. This
      only occurs when an imported module has not been compiled, or the compiled version is too
      old. This list should not overlap with sourceModules. */
  virtual ModuleList& sourceImportModules() = 0;

  /** Import a module given a path to the source file. */
  virtual semgraph::Module* importModuleFromSource(const support::Path& path) = 0;

  /** Dirty bit which says whether any of the module sets changed. */
  virtual bool moduleSetsChanged() const = 0;
  virtual void setModuleSetsChanged(bool changed) = 0;

  /** Scope that contains . */
  virtual scope::ModulePathScope* modulePathScope() const = 0;

  /** Registry for derived types. */
  virtual TypeStore* typeStore() const = 0;
};

}}

#endif
