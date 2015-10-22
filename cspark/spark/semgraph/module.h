// ============================================================================
// semgraph/module.h: Semantic graph nodes for modules.
// ============================================================================

#ifndef SPARK_SEMGRAPH_MODULE_H
#define SPARK_SEMGRAPH_MODULE_H 1

#ifndef SPARK_SEMGRAPH_DEFN_H
  #include "spark/semgraph/defn.h"
#endif

#ifndef SPARK_SUPPORT_ARENA_H
  #include "spark/support/arena.h"
#endif

#ifndef SPARK_SUPPORT_PATH_H
  #include "spark/support/path.h"
#endif

namespace spark {
namespace semgraph {

/** Semantic graph node for a module. */
class Module : public Member {
public:
  Module(const source::ProgramSource* source, const StringRef& name, Member* definedIn = NULL)
    : Member(Kind::MODULE, name, definedIn)
    , _source(source)
    , _memberScope(new scope::StandardScope(scope::SymbolScope::DEFAULT))
    , _importScope(new scope::StandardScope(scope::SymbolScope::DEFAULT))
  {}

  /** Source file of this module. */
  const source::ProgramSource* source() { return _source; }

  /** Members of this module. */
  MemberList& members() { return _members; }
  const MemberList& members() const { return _members; }

  /** Symbol scope for this module's members. */
  scope::StandardScope* memberScope() const { return _memberScope.get(); }

  /** Symbol scope for this module's imports. */
  scope::StandardScope* importScope() const { return _importScope.get(); }

  /** The absolute path to this module on disk. May be empty if the package was loaded from
      a library. */
  support::Path& path() { return _path; }
  const support::Path& path() const { return _path; }

  support::Arena& astArena() { return _astArena; }
  support::Arena& sgArena() { return _sgArena; }

  void format(std::ostream& out) const {}

private:
  const source::ProgramSource* _source;
  MemberList _members;
  MemberList _imports;
  std::auto_ptr<scope::StandardScope> _memberScope;
  std::auto_ptr<scope::StandardScope> _importScope;
  support::Path _path;

  support::Arena _astArena;
  support::Arena _sgArena;
};

}}

#endif
