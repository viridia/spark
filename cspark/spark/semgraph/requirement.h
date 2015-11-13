// ============================================================================
// semgraph/requirement.h: Template requirement clauses.
// ============================================================================

#ifndef SPARK_SEMGRAPH_REQUIREMENT_H
#define SPARK_SEMGRAPH_REQUIREMENT_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_SUPPORT_ARRAYBUILDER_H
  #include "spark/support/arraybuilder.h"
#endif

#if SPARK_HAVE_MEMORY
  #include <memory>
#endif

namespace spark {
namespace semgraph {
using collections::ArrayRef;

/** Type constrains on a template come in two forms: simple subtype / supertype constraints
    on individual template parameters, and required functions. A required function is a constraint
    that says that in order for a template to be specialized, a function must exist which has
    the given name and type signature. If the list of lookup contexts is non-empty, then the
    function must exist within one of those contexts. If the list is empty, then the function
    will be searched for using the normal procedure for unqualified function names, i.e. argument-
    dependent lookup (ADL).
  */
class RequiredFunction {
public:
  void* operator new(size_t size, support::Arena& arena) {
    return arena.allocate(size);
  }

  RequiredFunction(Function* method, const ArrayRef<Member*>& lookupContexts)
    : _method(method)
    , _lookupContexts(lookupContexts)
  {}
  RequiredFunction(Function* method)
    : _method(method)
  {}

  /** The function name and the type signature to search for. */
  Function* method() const { return _method; }

  /** Contexts where we should search for the required method. */
  const ArrayRef<Member*>& lookupContexts() { return _lookupContexts; }
  void setLookupContexts(const ArrayRef<Member*>& members) { _lookupContexts = members; }

private:
  Function* _method;
  ArrayRef<Member*> _lookupContexts;
};

}}

#endif
