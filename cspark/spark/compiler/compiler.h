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

#if SPARK_HAVE_ALGORITHM
  #include <algorithm>
#endif

namespace spark {
namespace support {
class Path;
}
namespace compiler {
using spark::collections::StringRef;
using spark::error::Reporter;

class Compiler {
public:
  Compiler(Reporter& reporter) : _reporter(reporter) {}
  
  Reporter& reporter() const { return _reporter; }

  /** List of source files or directories containing source files. */
  const std::vector<StringRef>& sourcePaths() const { return _sourcePaths; }
  void addSourcePath(const StringRef& srcPath) {
    _sourcePaths.push_back(srcPath);
  }

  /** List of modukle search paths. */
  const std::vector<StringRef>& modulePaths() const { return _modulePaths; }
  void addModulePath(const StringRef& modPath) {
    _modulePaths.push_back(modPath);
  }

  /** Output directory. */
  const StringRef& outputDir() const { return _outputDir; }
  void setOutputDir(const StringRef& outputDir) {
    _outputDir = outputDir;
  }
  
  void compile();

private:
  Reporter& _reporter;
  std::vector<StringRef> _sourcePaths;
  std::vector<StringRef> _modulePaths;
  std::vector<StringRef> _sourceModules;
  StringRef _outputDir;
  
  void parseSource(const StringRef& sourcePath);
  void processDir(support::Path& path /*, modules*/);
  void processFile(support::Path& path /*, modules */);
  bool shortPath(support::Path& path);

//   def setOutputDir(self, out):
//     assert not self.outputDir
//     self.outputDir = Path(out)
// 
//   def addModulePath(self, path):
//     self.basePaths.append(path)
//     self.fsPaths.addPath(path)

//     self.parser = Parser(errorReporter)
//     self.packageMgr = None

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
