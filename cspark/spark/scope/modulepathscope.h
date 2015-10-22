// ============================================================================
// compiler/rootscope.h: Top-level scope.
// ============================================================================

#ifndef SPARK_SCOPE_MODULEPATHSCOPE_H
#define SPARK_SCOPE_MODULEPATHSCOPE_H 1

#ifndef SPARK_SCOPE_SCOPE_H
  #include "spark/scope/scope.h"
#endif

namespace spark {
namespace semgraph {
class Defn;
}
namespace scope {
using collections::StringRef;
using semgraph::Defn;

/** Represents a storage location for module definitions, either in source format or compiled. */
class Importer {
public:
  /** Attempt to locate all symbols under this package with the name 'name'. */
  virtual void lookupName(const StringRef& name, std::vector<Member*> &result) = 0;
};

/** A virtual scope that looks for top-level symbols via the module path list. */
class ModulePathScope : public scope::SymbolScope {
public:
  ScopeType scopetype() const { return DEFAULT; }
  void lookupName(const StringRef& name, std::vector<Member*> &result) const;
  void forAllNames(NameFunctor& nameFn) const {}
  void describe(std::ostream& strm) const;

  /** Add an importer to the root scope. */
  void addImporter(Importer* imp) {
    _importers.push_back(imp);
  }

  /** Overridden - we don't allow members to be added directly. */
  void addMember(semgraph::Member* m);

private:
  std::vector<Importer*> _importers;
};

}}

#endif
