// ============================================================================
// compiler/rootscope.h: Top-level scope.
// ============================================================================

#ifndef SPARK_COMPILER_FSIMPORTER_H
#define SPARK_COMPILER_FSIMPORTER_H 1

#ifndef SPARK_SCOPE_MODULEPATHSCOPE_H
  #include "spark/scope/modulepathscope.h"
#endif

#ifndef SPARK_SEMGRAPH_PACKAGE_H
  #include "spark/semgraph/package.h"
#endif

#ifndef SPARK_SUPPORT_PATH_H
  #include "spark/support/path.h"
#endif

#if SPARK_HAVE_UNORDERED_MAP
  #include <unordered_map>
#endif

#if SPARK_HAVE_UNORDERED_SET
  #include <unordered_set>
#endif

namespace spark {
namespace compiler {

class Context;

using collections::ArrayRef;
using collections::StringRef;
using support::Path;

/** A scope which is backed by a directory in the local file system. This is used to search
    for source files. */
class DirectoryScope : public scope::SymbolScope {
public:
  DirectoryScope(const Path& path, semgraph::Package* parent, Context& context);
  ScopeType scopetype() const;
  void lookupName(const StringRef& name, std::vector<semgraph::Member*> &result) const;
  void forAllNames(scope::NameFunctor& nameFn) const;
  void describe(std::ostream& strm) const;

  /** Add a member to this scope. */
  void addMember(semgraph::Member* m);
private:
  bool lookupAliasName(const StringRef& name, std::vector<semgraph::Member*> &result) const;
  bool lookupFsName(const StringRef& name, std::vector<semgraph::Member*> &result) const;

  // Returns true if the given file exists in this directory and has the same case. This is
  // a workaround for case-insensitive but case-preserving file systems.
  bool fileExistsWithSameCase(const Path& path) const;

  typedef std::unordered_map<StringRef, std::vector<semgraph::Member*>> EntryMap;

  Context& _context;
  mutable EntryMap _entries;
  std::unordered_map<StringRef, std::vector<StringRef> > _aliases;
  std::unordered_set<StringRef> _filenames;
  const Path _path;
  semgraph::Package* _parent;
};

/** An importer that reads modules from the local file system. */
class FileSystemImporter : public scope::Importer {
public:
  FileSystemImporter(Context& context) : _context(context) { (void) _context; }
  ~FileSystemImporter();

  /** Add a root path. */
  void addPath(const Path& path);

  /** Given an absolute file path pointing to a source directory, return the package that
      corresponds to that directory. */
  semgraph::Package* getPackageForPath(const Path& path);

  void lookupName(const StringRef& name, std::vector<semgraph::Member*> &result);

private:
  std::vector<semgraph::Package*> _roots;
  Context& _context;
};

}}

#endif
