#include "spark/compiler/context.h"
#include "spark/compiler/fsimport.h"
#include "spark/semgraph/module.h"
#include "spark/support/arena.h"

#if SPARK_HAVE_FSTREAM
  #include <fstream>
#endif

namespace spark {
namespace compiler {
using namespace semgraph;

DirectoryScope::DirectoryScope(const Path& path, semgraph::Package* parent, Context& context)
  : _context(context)
  , _path(path)
  , _parent(parent)
{
  assert(path.isDir());
  Path packageOpts(path, "package.txt");
  auto& arena = _context.arena();
  if (packageOpts.isFile()) {
    std::ifstream strm(packageOpts.c_str());
    std::string line;
    std::vector<StringRef> parts;
    while (std::getline(strm, line)) {
      if (line.empty()) {
        continue;
      }
      StringRef lineStr(line);
      parts.clear();
      size_t pos = 0;
      // TODO: Do we need a general string split function?
      while (pos < line.size()) {
        size_t end = line.find('.', pos);
        if (end == std::string::npos) {
          end = line.size();
        }
        parts.push_back(arena.copyOf(lineStr.substr(pos, end)));
        pos = end + 1;
      }
      if (parts.size() < 2) {
        context.reporter().fatal() << "Invalid alias in package.txt: " << line;
      } else {
        _aliases[parts.back()] = parts;
      }
    }
  }
  (void)_parent;

  // Cache the directory listing. We need this for case-insensitive file systems.
  auto iter = _path.iterate();
  StringRef name;
  while (iter.next(name)) {
    if (name != "." && name != "..") {
      _filenames.insert(arena.copyOf(name));
    }
  }
}

scope::SymbolScope::ScopeType DirectoryScope::scopetype() const {
  return DEFAULT;
}

void DirectoryScope::addMember(Member* m) {
  _entries[m->name()].push_back(m);
}

void DirectoryScope::lookupName(const StringRef& name, std::vector<Member*> &result) const {
  // See if the name is an alias for a longer name.
  if (lookupAliasName(name, result)) {
    return;
  }

  // Otherwise, check the filesystem.
  lookupFsName(name, result);
}

bool DirectoryScope::lookupAliasName(const StringRef& name, std::vector<Member*> &result) const {
  auto it = _aliases.find(name.str());
  if (it != _aliases.end()) {
    std::vector<Member*> members;
    std::vector<Member*> nextMembers;
    bool first = true;
    for (StringRef part : it->second) {
      if (first) {
        first = false;
        // Check the filesystem for the expansion of the alias.
        lookupFsName(part, members);
      } else {
        nextMembers.clear();
        for (Member* m : members) {
          if (m->kind() == Member::Kind::PACKAGE) {
            static_cast<const Package*>(m)->memberScope()->lookupName(part, nextMembers);
          } else if (m->kind() == Member::Kind::MODULE) {
            static_cast<const Module*>(m)->memberScope()->lookupName(part, nextMembers);
          } else if (m->kind() == Member::Kind::TYPE) {
            static_cast<const TypeDefn*>(m)->memberScope()->lookupName(part, nextMembers);
          }
        }
        members.swap(nextMembers);
      }
    }
    result.insert(result.end(), members.begin(), members.end());
    return true;
  }
  return false;
}

bool DirectoryScope::lookupFsName(
    const StringRef& name, std::vector<Member*> &result) const {
  auto it = _entries.find(name);
  if (it != _entries.end()) {
    result.insert(result.end(), it->second.begin(), it->second.end());
    return true;
  }

  Path entryPath(_path, name);
  if (entryPath.isDir()) {
    auto package = new semgraph::Package(name, _parent);
    package->setMemberScope(new DirectoryScope(entryPath, package, _context));
    package->path() = entryPath;
    _entries[package->name()].push_back(package);
    result.push_back(package);
  } else {
//     Path clsPath = entryPath.withSuffix(".spc");
    Path srcPath = entryPath.withSuffix(".sp");
#if 0
    if (fileExistsWithSameCase(clsPath)) {
      assert(false && "implement class import loading.");
      if clsPath in self.compiler.outputModulesByPath:
        # Don't load class files that correspond to source files we are compiling.
        return
      if srcPath.is_file():
        # TODO: We have both a source and class file - choose the more recent?
        debug.write('source path and output path:', srcPath)
        pass
      modules = self.compiler.loadClass(clsPath)
      # The compiler should have added the module to this package.
      assert name in self.entries
      results.extend(modules)
    } else if (fileExistsWithSameCase(srcPath)) {
#endif
    if (fileExistsWithSameCase(srcPath)) {
      Module* module = _context.importModuleFromSource(srcPath);
      if (module != nullptr) {
        result.push_back(module);
        // The compiler should have added the module to this package.
        auto it = _entries.find(name);
        assert(it != _entries.end());
      }
    }
  }
  return !result.empty();
}

void DirectoryScope::forAllNames(scope::NameFunctor& nameFn) const {
  for (auto name : _filenames) {
    Path filePath(name);
    nameFn(filePath.stem());
  }
  for (auto it : _aliases) {
    nameFn(it.first);
  }
}

void DirectoryScope::describe(std::ostream& strm) const {
  strm << "directory scope";
}

bool DirectoryScope::fileExistsWithSameCase(const Path& path) const {
  if (path.isFile()) {
    return _filenames.find(path.name()) != _filenames.end();
  }
  return false;
}

FileSystemImporter::~FileSystemImporter() {
  for (semgraph::Package* root : _roots) {
    delete root;
  }
}

void FileSystemImporter::addPath(const Path& path) {
  auto package = new semgraph::Package(path.name(), nullptr);
  package->setMemberScope(new DirectoryScope(path, package, _context));
  package->path() = path;
  _roots.push_back(package);
}

void FileSystemImporter::lookupName(
    const StringRef& name, std::vector<Member*> &result) {
  for (semgraph::Package* root : _roots) {
    root->memberScope()->lookupName(name, result);
  }
}

semgraph::Package* FileSystemImporter::getPackageForPath(const Path& path) {
  std::vector<StringRef> pathParts;
  std::vector<StringRef> rootParts;
  // For each root.
  for (semgraph::Package* root : _roots) {
    pathParts = path.parts();
    rootParts = root->path().parts();
    // Strip common prefix.
    while (pathParts.size() > 0 && rootParts.size() > 0 && pathParts.front() == rootParts.front()) {
      pathParts.erase(pathParts.begin());
      rootParts.erase(rootParts.begin());
      // If root is shorter, then what's left is the path relative to the root.
      if (rootParts.empty()) {
        semgraph::Package* pkg = root;
        // Now use the remaining parts to drill down into the package hierarchy.
        for (StringRef name : pathParts) {
          std::vector<Member*> packages = pkg->memberScope()->lookupName(name);
          assert(packages.size() == 1);
          assert(packages.front()->kind() == Member::Kind::PACKAGE);
          pkg = static_cast<semgraph::Package*>(packages.front());
        }
        return pkg;
      }
    }
  }
  return nullptr;
}

}}
