// ============================================================================
// compiler.h: Main compiler class.
// ============================================================================

#ifndef SPARK_COMPILER_COMPILER_H
#define SPARK_COMPILER_COMPILER_H 1

#include <spark/config.h>

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

#ifndef SPARK_ERROR_REPORTER_H
  #include "spark/error/reporter.h"
#endif

#ifndef SPARK_SUPPORT_PATH_H
  #include "spark/support/path.h"
#endif

#if SPARK_HAVE_ALGORITHM
  #include <algorithm>
#endif

#if SPARK_HAVE_UNORDERED_SET
  #include <unordered_set>
#endif

#if SPARK_HAVE_VECTOR
  #include <vector>
#endif

#if SPARK_HAVE_MEMORY
  #include <memory>
#endif

namespace spark {
namespace support {
class Path;
}
namespace semgraph {
class Module;
}
namespace sema {
class Pass;
}
namespace compiler {
using collections::StringRef;
using error::Reporter;
using support::Path;

class Context;
class ContextImpl;
class FileSystemImporter;
class Phase;

typedef std::vector<semgraph::Module*> ModuleList;
typedef std::unordered_set<semgraph::Module*> ModuleSet;

class Compiler {
public:
  Compiler(Reporter& reporter);

  /** The error reporter for this compiler instance. */
  Reporter& reporter() const { return _reporter; }

  /** Root directory for the source tree. Input source patterns are resolved relative to this. */
  const Path& sourceRoot() const { return _sourceRoot; }
  void setSourceRoot(const StringRef& path);

  /** List of source files or directories containing source files to compile.
      If a sourceRoot has been specified, these paths will be resolved relative to that
      path, otherwise they are resolved relative to the current directory. */
  const std::vector<Path>& sources() const { return _sources; }
  void addSource(const StringRef& path);

  /** List of module search paths. */
  const std::vector<Path>& modulePaths() const { return _modulePaths; }
  void addModulePath(const StringRef& path);

  /** Output directory. */
  const Path& outputDir() const { return _outputDir; }
  void setOutputDir(const StringRef& path);

  void compile();

private:
  friend class spark::compiler::ContextImpl;

  Reporter& _reporter;
  Path _sourceRoot;
  std::vector<Path> _sources;
  std::vector<Path> _modulePaths;
  Path _outputDir;
  support::Path _currentDir;

  std::auto_ptr<Context> _context;
  FileSystemImporter* _fsImporter; // This is actually owned by the module path scope
  std::vector<Phase*> _phases;
  Phase* _importGraphBuilder;

  void parseSource(const support::Path& sourcePath);
  semgraph::Module* parseImportSource(const Path& path);
  void processDir(const support::Path& path, ModuleList& modules);
  void processFile(const support::Path& path, ModuleList& modules);
  bool shortPath(support::Path& path);

  void runPhases();

//   def addModulePath(self, path):
//     self.basePaths.append(path)
//     self.fsPaths.addPath(path)

//     super().__init__(errorReporter)
//     self.packageMgr = PackageMgr()
//     self.spcparser = spcparse.SpcParser(self)
//     self.fsPaths = FileSystemImporter(self)
//     self.packageMgr.addImporter(self.fsPaths)
//     self.typeStore = TypeStore()
//     self.requiredMethodCache = requiredmethodcache.RequiredMethodCache()
//     self.sourcePaths = []
//     self.sourceModules = []
//     self.sourceImportModules = []
//     self.compiledImportModules = []
//     self.sourceModulesByPath = {}
//     self.outputModulesByPath = {}
//     self.moduleSetsChanged = False
//     self.basePaths = []
//     self.outputDir = None
//     self.packageAliases = defaultdict(list)
//     self.passesRun = defaultdict(set)
//     self.passGroups = []
};

}}

#endif
