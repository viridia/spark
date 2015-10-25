// ============================================================================
// semgraph/type.h: Semantic graph nodes for types.
// ============================================================================

#ifndef SPARK_SEMGRAPH_ENV_H
#define SPARK_SEMGRAPH_ENV_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

#ifndef SPARK_SUPPORT_ARENA_H
  #include "spark/support/arena.h"
#endif

namespace spark {
namespace semgraph {
using collections::ArrayRef;
class Type;

/** A set of bindings between type parameters and types. */
class Env {
public:
  class Binding {
  public:
    Binding() : _key(nullptr), _value(nullptr) {}
    Binding(Type* key, Type* value) : _key(key), _value(value) {}
    Binding(const Binding& src) : _key(src._key), _value(src._value) {}

    Type* key() const { return _key; }
    Type* value() const { return _value; }

    Binding& operator=(const Binding& src) {
      _key = src._key;
      _value = src._value;
      return *this;
    }

  private:
    Type* _key;
    Type* _value;
  };

  Env() {}
  Env(const Env& env) : _bindings(env._bindings) {}
  Env(const collections::ArrayRef<Binding>& bindings) : _bindings(bindings) {}

  /** Assignment operator. */
  Env& operator=(const Env& env) {
    _bindings = env._bindings;
    return *this;
  }

  /** Add a new variable binding to the environment. */
  static Env create(support::Arena& arena, const collections::ArrayRef<Binding>& _bindings) {
    return Env(arena.copyOf(_bindings));
  }

  /** Return the value bound to the given key, or null if there is no binding for it. */
  Type* get(Type* key) const {
    for (auto b : _bindings) {
      if (key == b.key()) {
        return b.value();
      }
    }
    return nullptr;
  }

private:
  collections::ArrayRef<Binding> _bindings;
};

}}

#endif
