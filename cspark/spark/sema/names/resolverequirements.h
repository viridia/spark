// ============================================================================
// sema/names/resolverequirements.h.
// ============================================================================

#ifndef SPARK_SEMA_NAMES_RESOLVEREQUIREMENTS_H
#define SPARK_SEMA_NAMES_RESOLVEREQUIREMENTS_H 1

#ifndef SPARK_SEMGRAPH_EXPRVISITOR_H
  #include "spark/semgraph/exprvisitor.h"
#endif

#include "spark/semgraph/requirement.h"

#ifndef SPARK_SEMA_TYPES_APPLYENV_H
  #include "spark/sema/types/applyenv.h"
#endif

namespace spark {
namespace ast {
  class Oper;
}
namespace error { class Reporter; }
namespace scope { class ScopeStack; }
namespace sema {
namespace types {
class TypeStore;
}
namespace names {
using error::Reporter;
using namespace semgraph;

/** Name resolver specialized for resolving types. */
class ResolveRequirements {
public:
  ResolveRequirements(
      Reporter& reporter,
      Subject& subject,
      scope::ScopeStack* scopeStack,
      sema::types::TypeStore* typeStore,
      support::Arena& arena)
    : _reporter(reporter)
    , _subject(subject)
    , _scopeStack(scopeStack)
    , _typeStore(typeStore)
    , _arena(arena)
    , _apply(typeStore)
  {
    assert(_typeStore != nullptr);
  }

  bool exec(const ast::Node* node);

private:
  bool visitCallRequired(const ast::Oper* ast);
  bool visitEqual(const ast::Oper* ast);
  bool visitRefEqual(const ast::Oper* ast);
  bool visitNotEqual(const ast::Oper* ast);
  bool visitLessThan(const ast::Oper* ast);
  bool visitGreaterThan(const ast::Oper* ast);
  bool visitLessThanOrEqual(const ast::Oper* ast);
  bool visitGreaterThanOrEqual(const ast::Oper* ast);
  void relationalOp(
      const source::Location& loc,
      const StringRef& name,
      const ast::Node* left,
      const ast::Node* right);
  void addTargetRequirement(semgraph::RequiredFunction* reqFunc);
  bool resolveLookupContexts(
      const ast::Node* node,
      collections::SmallSetBase<semgraph::Member*>& result);
  bool getFunctionParams(
      const ast::NodeList& astParams,
      semgraph::Defn* definedIn,
      std::vector<semgraph::Parameter*>& result);
  void getFunctionParams(
      semgraph::FunctionType* ft,
      semgraph::Defn* definedIn,
      std::vector<semgraph::Parameter*>& result);
  Type* resolveType(const ast::Node* node);

  Reporter& _reporter;
  Subject& _subject;
  scope::ScopeStack* _scopeStack;
  sema::types::TypeStore* _typeStore;
  support::Arena& _arena;
  types::ApplyEnv _apply;
};

}}}

#endif
