// ============================================================================
// compiler.cpp
// ============================================================================

#include "spark/compiler/compiler.h"
#include "spark/source/programsource.h"
#include "spark/parse/parser.h"
#include "spark/support/arena.h"
#include "spark/support/path.h"

#if SPARK_HAVE_DIRENT_H
  #include <dirent.h>
#endif

#if SPARK_HAVE_SYS_STAT_H
  #include <sys/stat.h>
#endif

namespace spark {
namespace compiler {
using spark::collections::StringRef;
using spark::support::Path;
using spark::support::PathIterator;

void Compiler::compile() {
  for (StringRef sourcePath : _sourcePaths) {
    parseSource(sourcePath);
  }
//     self.runPassGroups()
//     if self.outputDir:
//       self.writePackageAliases()
}

void Compiler::parseSource(const StringRef& sourcePath) {
  Path path(sourcePath);
  if (!path.exists()) {
    _reporter.error() << "File not found: " << sourcePath;
  } else if (path.isDir()) {
    processDir(path /*, NULL*/);
  } else {
    processFile(path /*, NULL*/);
  }
}

#if 0
  def parseSource(self, sourcePath):
    path = Path(sourcePath)
    if not path.exists():
      self.errorReporter.errorFmt('File not found: {0}', path)
    elif path.is_dir():
      self._processDir(path, self.sourceModules)
    else:
      self._processFile(path, self.sourceModules)

  def parseImportSource(self, sourcePath):
    path = Path(sourcePath)
    modules = []
    if not path.exists():
      self.errorReporter.errorFmt('File not found: {0}', path)
    elif path.is_dir():
      self.errorReporter.errorFmt('Attempt to import a directory: {0}', path)
    else:
      self._processFile(path, modules)
    self.sourceImportModules.extend(modules)
    self.moduleSetsChanged = True
    if len(self.typeStore.essentialDefns) > 0:
      self.sourceScopeGroup.run()
#     self.runPasses(modules, self.initialPasses)
    return modules
#endif

void Compiler::processDir(Path& path /*, modules*/) {
  PathIterator iter = path.iterate();
  StringRef name;
  while (iter.next(name)) {
    if (_reporter.errorCount() > 10) {
      break;
    }
    Path entry(path, name);
    if (entry.isDir() && name != "." && name != "..") {
      processDir(entry);
    } else if (entry.isFile() && name.endsWith(".sp")) {
      processFile(entry);
    }
  }
}

void Compiler::processFile(Path& path /*, modules */) {
  source::ProgramSource* src = new source::FileSource(path, path.str());
  if (!src->valid()) {
    if (!path.exists()) {
      _reporter.error() << "File '" << path << "' not found.\n";
    } else {
      _reporter.error() << "Unable to open file '" << path << "' for reading.\n";
    }
    return;
  }
  _reporter.debug() << "Parsing: " << path;
  support::Arena arena;
  parse::Parser parser(_reporter, src, arena);
  ast::Module* mod = parser.module();
  (void)mod;
//   mod = self.parser.parse(src, location.SourceFile(relPath, src))
}

#if 0
  def _processFile(self, path, modules):
    relPath = self.shortPath(path)
#     sys.stdout.write('Compiling: {0}\n'.format(relPath))
    try:
      fh = path.open('r')
    except FileNotFoundError:
      sys.stderr.write("Error: file '{0}' not found.\n".format(path))
      sys.exit(-1)
    try:
      src = fh.read()
    finally:
      fh.close()
    mod = self.parser.parse(src, location.SourceFile(relPath, src))
    if mod:
      mod.setName(path.stem)
      mod.setPath(str(relPath))
      pkg = self.fsPaths.getPackageForPath(path.parent)
      if pkg:
        names = []
        for p in defns.ancestors(pkg):
          names = [p.getName()] + names
        if names:
          mod.setPackage('.'.join(names))
        if not isinstance(pkg, fsimporter.RootPackage):
          mod.setDefinedIn(pkg)
        pkg.addMember(mod)
      modules.append(mod)
      outputPath = self.outputDir / relPath.with_suffix('.spc')
      self.sourceModulesByPath[path.resolve()] = mod
      self.outputModulesByPath[outputPath] = mod
      self.moduleSetsChanged = True

#endif

}}
