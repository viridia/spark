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
  NameLookupResult() : scope(nullptr), stem(nullptr) {}
  NameLookupResult(const NameLookupResult& src)
    : members(src.members)
    , scope(src.scope)
    , stem(src.stem)
  {}
  NameLookupResult(const NameLookupResult&& src)
    : members(std::move(src.members))
    , scope(src.scope)
    , stem(src.stem)
  {}
  NameLookupResult& operator=(const NameLookupResult&& src) {
    members = std::move(src.members);
    scope = src.scope;
    stem = src.stem;
    return *this;
  }

  /** List of members found. */
  std::vector<Member*> members;

  /** Scope in which the members were found. */
  SymbolScope* scope;

  /** Expression representing the object containing the members. */
  Expr* stem;
};

/** Represents the set of nested lookup scopes for the current lookup context. */
class ScopeStack {
public:
  struct Entry {
    Entry() : scope(nullptr), stem(nullptr) {}
    Entry(const Entry& src) : scope(src.scope), stem(src.stem) {}
    Entry(SymbolScope* s, Expr* b) : scope(s), stem(b) {}

    Entry& operator=(const Entry& src) {
      scope = src.scope;
      stem = src.stem;
      return *this;
    }

    SymbolScope* scope;
    Expr* stem;
  };

  ScopeStack() {}
  ScopeStack(const ScopeStack& src) : _stack(src._stack) {}

  /** Push a new scope onto the stack. The optional 'stem' expression is a reference to the
      object whose type defines the scope. Most often, 'stem' will be a 'self' expression. */
  void push(SymbolScope* scope, Expr* stem = nullptr) {
    _stack.push_back(Entry(scope, stem));
  }

  /** Remove the top-most scope from the stack. */
  void pop() {
    _stack.pop_back();
  }

  /** Find a symbol on the closest enclosing scope. */
  NameLookupResult find(const StringRef& name) {
    NameLookupResult result;
    auto it = _stack.end();
    while (it != _stack.begin()) {
      --it;
      it->scope->lookupName(name, result.members);
      if (!result.members.empty()) {
        result.scope = it->scope;
        result.stem = it->stem;
        return result;
      }
    }
    return result;
  }

  bool find(const StringRef& name, NameLookupResult& result) {
    auto it = _stack.end();
    while (it != _stack.begin()) {
      --it;
      it->scope->lookupName(name, result.members);
      if (!result.members.empty()) {
        result.scope = it->scope;
        result.stem = it->stem;
        return true;
      }
    }
    return false;
  }

  /** Call the specified functor for all names defined in this scope. */
  void forAllNames(NameFunctor& nameFn) const {
    auto it = _stack.end();
    while (it != _stack.begin()) {
      --it;
      it->scope->forAllNames(nameFn);
    }
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

  /** Scope stacks can be copied. */
  ScopeStack& operator=(const ScopeStack& src) {
    _stack = src._stack;
    return *this;
  }

  /** Empty the stack. */
  void clear() {
    _stack.clear();
  }

  void validate() {
    auto it = _stack.end();
    while (it != _stack.begin()) {
      --it;
      it->scope->validate();
    }
  }

private:
  std::vector<Entry> _stack;
};

}}

#endif
