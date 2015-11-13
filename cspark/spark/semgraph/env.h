// ============================================================================
// semgraph/type.h: Semantic graph nodes for types.
// ============================================================================

#ifndef SPARK_SEMGRAPH_ENV_H
#define SPARK_SEMGRAPH_ENV_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_COLLECTIONS_MAP_H
  #include "spark/collections/map.h"
#endif

#ifndef SPARK_COLLECTIONS_HASHING_H
  #include "spark/collections/hashing.h"
#endif

#ifndef SPARK_SUPPORT_ARENA_H
  #include "spark/support/arena.h"
#endif

namespace spark {
namespace semgraph {
using collections::ArrayRef;
class Type;
class TypeVar;

typedef collections::ReadableMap<TypeVar*, Type*> EnvMap;

/** An environment containing a set of bindings between type parameters and types.
    Environments are immutable once created. */
class Env {
public:
  typedef collections::ImmutableMap<TypeVar*, Type*> Bindings;
  typedef typename Bindings::element_type Binding;
  typedef typename Bindings::const_iterator const_iterator;

  Env() {}
  Env(const Env& env) : _bindings(env._bindings) {}
  Env(const Bindings& bindings) : _bindings(bindings) {}
  Env(const Bindings&& bindings) : _bindings(bindings) {}

  std::size_t size() const { return _bindings.size(); }
  const Bindings& bindings() const { return _bindings; }

//   /** Assignment operator. */
//   Env& operator=(const Env& env) {
//     _bindings = env._bindings;
//     return *this;
//   }

  /** Return the value bound to the given key, or null if there is no binding for it. */
  Type* get(TypeVar* key) const {
    const_iterator it = _bindings.find(key);
    if (it != _bindings.end()) {
      return it->second;
    }
    return nullptr;
  }

  /** Iterators. */
  Bindings::const_iterator begin() const { return _bindings.begin(); }
  Bindings::const_iterator end() const { return _bindings.end(); }

private:
  Bindings _bindings;
};

}}

namespace std {

/** Compute a hash for an environment map. */
template<>
struct hash<spark::semgraph::EnvMap> {
  inline std::size_t operator()(const spark::semgraph::EnvMap& value) const {
    std::size_t seed = 0;
    for (auto entry : value) {
      std::size_t entryHash = std::hash<spark::semgraph::TypeVar*>()(entry.first);
      hash_combine(seed, std::hash<spark::semgraph::Type*>()(entry.second));
      seed ^= entryHash;
    }
    return seed;
  }
};

}

#endif
