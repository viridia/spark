// ============================================================================
// semgraph/pacakge.h: Semantic graph nodes for packages.
// ============================================================================

#ifndef SPARK_SEMGRAPH_PACKAGE_H
#define SPARK_SEMGRAPH_PACKAGE_H 1

#ifndef SPARK_SEMGRAPH_DEFN_H
  #include "spark/semgraph/defn.h"
#endif

#ifndef SPARK_SUPPORT_PATH_H
  #include "spark/support/path.h"
#endif

namespace spark {
namespace scope {
class SymbolScope;
}
namespace semgraph {
using scope::SymbolScope;

class Module;

class Package : public Member {
public:
  Package(const StringRef& name, SymbolScope* scope, Member* definedIn)
    : Member(Kind::PACKAGE, name, definedIn)
    , _memberScope(scope)
  {}

  Package(const StringRef& name, Member* definedIn = NULL)
    : Member(Kind::PACKAGE, name, definedIn)
  {}

  /** Add a module to this package. */
  void addModule(Module* module);

  /** Symbol scope for this package's members. */
  SymbolScope* memberScope() const { return _memberScope.get(); }
  void setMemberScope(SymbolScope* scope) { _memberScope.reset(scope); }

  /** The absolute path to this package on disk. May be empty if the package was loaded from
      a library. */
  support::Path& path() { return _path; }
  const support::Path& path() const { return _path; }

  void format(std::ostream& out) const {}

private:
  std::auto_ptr<SymbolScope> _memberScope;
  support::Path _path;
};

}}

#endif
