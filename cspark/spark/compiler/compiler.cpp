#include "spark/ast/module.h"
#include "spark/compiler/compiler.h"
#include "spark/compiler/contextimpl.h"
#include "spark/compiler/fsimport.h"
#include "spark/compiler/phase.h"
#include "spark/source/programsource.h"
#include "spark/parse/parser.h"
#include "spark/support/arena.h"
#include "spark/support/path.h"
#include "spark/sema/passes/buildgraph.h"
#include "spark/sema/passes/nameresolution.h"
#include "spark/semgraph/module.h"

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

Compiler::Compiler(Reporter& reporter) : _reporter(reporter) {
  _context.reset(new ContextImpl(reporter, *this));
  _fsImporter = new FileSystemImporter(*_context.get());
  _currentDir = support::Path::curdir();

  _context->modulePathScope()->addImporter(_fsImporter);

  // Build graph phase (sources)
  compiler::Phase* phase = new compiler::Phase(
      _context.get(), "build source graph", _context->sourceModules());
  phase->addPass(new spark::sema::passes::BuildGraphPass(_context.get()));
  _phases.push_back(phase);

  // Build graph phase (source imports). This phase is special because it needs to be run
  // immediately after a source module has been imported. This is so that the import statement
  // that caused the module to load can be resolved.
  _importGraphBuilder = new compiler::Phase(
      _context.get(), "build source input graph", _context->sourceImportModules());
  _importGraphBuilder->addPass(new spark::sema::passes::BuildGraphPass(_context.get()));
  _phases.push_back(_importGraphBuilder);

  // Name resolution phase
  phase = new compiler::Phase(
      _context.get(), "name resolution", _context->sourceModules());
  phase->addPass(new spark::sema::passes::NameResolutionPass(_context.get()));
  _phases.push_back(phase);
}

void Compiler::setSourceRoot(const StringRef& path) {
  _sourceRoot = Path(_currentDir, path);
}

void Compiler::addSource(const StringRef& path) {
  _sources.push_back(Path(_currentDir, path));
}

void Compiler::addModulePath(const StringRef& path) {
  _modulePaths.push_back(Path(_currentDir, path));
}

void Compiler::setOutputDir(const StringRef& path) {
  _outputDir = Path(_currentDir, path);
}

void Compiler::compile() {
  if (!_sourceRoot.empty()) {
    // If a source root has been specified, then use that as the root directory for sources.
    _fsImporter->addPath(_sourceRoot);
  } else {
    // Otherwise, use the input source args as source roots.
    for (Path& path : _sources) {
      _fsImporter->addPath(path);
    }
  }

//   for (Path path : _modulePaths) {
//     if (path.isDir()) {
//       _fsImporter->addModulePath(path);
//     }
//   }

  for (Path& path : _sources) {
    parseSource(path);
  }
  runPhases();
//     if self.outputDir:
//       self.writePackageAliases()
}

void Compiler::parseSource(const Path& path) {
  if (!path.exists()) {
    _reporter.error() << "File not found: " << path;
  } else if (path.isDir()) {
    processDir(path, _context->sourceModules());
  } else {
    processFile(path, _context->sourceModules());
  }
}

semgraph::Module* Compiler::parseImportSource(const Path& path) {
  ModuleList modules;
  if (!path.exists()) {
    _reporter.error() << "File not found: " << path;
  } else if (path.isDir()) {
    _reporter.error() << "Attempt to import a directory: " << path;
  } else {
    // Parse the file and add to the 'modules' set.
    processFile(path, modules);
    // Also add to the sourceImportModules set.
    _context->sourceImportModules().insert(
        _context->sourceImportModules().end(), modules.begin(), modules.end());
    // Run the graph builder immediately, don't wait for regularly scheduled pass. This needs
    // to be done because the caller is probably going to start looking up symbols within the
    // module.
    _importGraphBuilder->run();
  }
  assert(modules.size() <= 1);
  if (modules.size() == 1) {
    return modules.front();
  }
  return nullptr;
}

void Compiler::processDir(const Path& path, ModuleList& modules) {
  PathIterator iter = path.iterate();
  StringRef name;
  while (iter.next(name)) {
    if (_reporter.errorCount() > 10) {
      break;
    }
    Path entry(path, name);
    if (entry.isDir() && name != "." && name != "..") {
      processDir(entry, modules);
    } else if (entry.isFile() && name.endsWith(".sp")) {
      processFile(entry, modules);
    }
  }
}

void Compiler::processFile(const Path& path, ModuleList& modules) {
  source::ProgramSource* src = new source::FileSource(path, path.str());
  if (!src->valid()) {
    if (!path.exists()) {
      _reporter.error() << "File '" << path << "' not found.\n";
    } else {
      _reporter.error() << "Unable to open file '" << path << "' for reading.\n";
    }
    return;
  }

  // What we want to avoid here is double-loading of a source module, once because it's
  // in the list of sources, and again as the result of an import. More specifically, we need
  // to treat imported modules differently depending on whether they are within the source tree
  // or not. Modules outside the source tree will get loaded and parsed on demand, whereas
  // importing a module that's inside the source tree should find just the module that has already
  // been read in.
  // This lazily constructs the package tree if it doesn't already exist.
  semgraph::Package* package = _fsImporter->getPackageForPath(path.parent());
  assert(package != nullptr);
  semgraph::Module* module = new semgraph::Module(src, path.stem(), package);
  parse::Parser parser(_reporter, src, module->astArena());
  const ast::Module* modAst = parser.module();
  if (modAst != nullptr) {
    module->setAst(modAst);
    module->path() = path;
    modules.push_back(module);
    package->addModule(module);
    _context->setModuleSetsChanged(true);
  }
}

void Compiler::runPhases() {
  // Attempt to run all phases to completion. If at any point the set of modules that need to
  // be compiled changes (because we encountered an import statement for example), then start
  // the whole process over again (note that phases won't re-process a module that has already
  // been processed).
  while (_context->moduleSetsChanged() && _reporter.errorCount() == 0) {
    _context->setModuleSetsChanged(false);
    for (Phase* phase : _phases) {
      if (_reporter.errorCount() > 0) {
        break;
      }
      phase->run();
      // If running the phase caused additional modules to be added, then start over.
      if (_context->moduleSetsChanged()) {
        break;
      }
    }
  }
}

}}
