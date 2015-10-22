// ============================================================================
// scope/scope.h: Symbol table scopes.
// ============================================================================

#ifndef SPARK_SCOPE_SCOPESTACK_H
#define SPARK_SCOPE_SCOPESTACK_H 1

#ifndef SPARK_SCOPE_SCOPE_H
  #include "spark/scope/scope.h"
#endif

namespace spark {
namespace semgraph {
class Expr;
}
namespace scope {
using semgraph::Expr;

/** The result of a name lookup operation. */
struct NameLookupResult {
  NameLookupResult() : scope(NULL), base(NULL) {}
  NameLookupResult(const NameLookupResult& src)
    : members(src.members)
    , scope(src.scope)
    , base(src.base)
  {}
  NameLookupResult(const NameLookupResult&& src)
    : members(std::move(src.members))
    , scope(src.scope)
    , base(src.base)
  {}

  /** List of members found. */
  std::vector<Member*> members;

  /** Scope in which the members were found. */
  SymbolScope* scope;

  /** Expression representing the object containing the members. */
  Expr* base;
};

/** Represents the set of nested lookup scopes for the current lookup context. */
class ScopeStack {
public:
  struct Entry {
    Entry() : scope(NULL), base(NULL) {}
    Entry(const Entry& src) : scope(src.scope), base(src.base) {}
    Entry(SymbolScope* s, Expr* b) : scope(s), base(b) {}

    Entry& operator=(const Entry& src) {
      scope = src.scope;
      base = src.base;
      return *this;
    }

    SymbolScope* scope;
    Expr* base;
  };

  /** Push a new scope onto the stack. The optional 'base' expression is a reference to the
      object whose type defines the scope. Most often, 'base' will be a 'self' expression. */
  void push(SymbolScope* scope, Expr* base = NULL) {
    _stack.push_back(Entry(scope, base));
  }

  /** Remove the top-most scope from the stack. */
  void pop() {
    _stack.pop_back();
  }

  NameLookupResult find(const StringRef& name) {
    NameLookupResult result;
    auto it = _stack.end();
    while (it != _stack.begin()) {
      it->scope->lookupName(name, result.members);
      if (!result.members.empty()) {
        result.scope = it->scope;
        result.base = it->base;
        return result;
      }
    }
    return result;
  }

  /** The current size of the stack. */
  size_t size() const { return _stack.size(); }

  /** Resize the stack to the given size. This is only allowed to shrink the stack. This is
      primarily used to restore the stack to a previous state, popping a bunch of entries in a
      single operation. */
  void resize(size_t newSize) {
    assert(newSize <= _stack.size());
    _stack.resize(newSize);
  }

  /** Empty the stack. */
  void clear() {
    _stack.clear();
  }

private:
  std::vector<Entry> _stack;
};

}}

#endif
