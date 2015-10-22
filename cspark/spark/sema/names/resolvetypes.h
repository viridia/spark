// ============================================================================
// sema/names/resolvetypes.h.
// ============================================================================

#ifndef SPARK_SEMA_NAMES_RESOLVETYPES_H
#define SPARK_SEMA_NAMES_RESOLVETYPES_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#include <../lib/spark/core/string.sp>
#endif

namespace spark {
namespace ast { class Node; }
namespace compiler { class TypeStore; }
namespace error { class Reporter; }
namespace semgraph { class Type; }
namespace scope { class ScopeStack; }
namespace sema {
namespace names {
using error::Reporter;
using semgraph::Type;

/** Name resolver specialized for resolving types. */
class ResolveTypes {
public:
  ResolveTypes(Reporter& reporter, compiler::TypeStore* typeStore, scope::ScopeStack* scopeStack)
    : _reporter(reporter)
    , _typeStore(typeStore)
    , _scopeStack(scopeStack)
  {}

  Type* exec(const ast::Node* node);

private:
  Type* visitIdent(const ast::Ident* ident);

  Reporter& _reporter;
  compiler::TypeStore* _typeStore;
  scope::ScopeStack* _scopeStack;
};

}}}

#endif
