// ============================================================================
// sema/typestore.h: Class for creating and managing derived types.
// ============================================================================

#ifndef SPARK_SEMA_TYPES_TYPESTORE_H
#define SPARK_SEMA_TYPES_TYPESTORE_H 1

#ifndef SPARK_SEMGRAPH_EXPR_H
  #include "spark/semgraph/expr.h"
#endif

#ifndef SPARK_SEMGRAPH_TYPE_H
  #include "spark/semgraph/type.h"
#endif

#ifndef SPARK_SEMA_TYPES_ORDERING_H
  #include "spark/sema/types/ordering.h"
#endif

#ifndef SPARK_COLLECTIONS_MAP_H
  #include "spark/collections/map.h"
#endif

#if SPARK_HAVE_UNORDERED_SET
  #include <unordered_set>
#endif

#if SPARK_HAVE_UTILITY
  #include <utility>
#endif

namespace spark {
namespace sema {
namespace types {
using semgraph::ConstType;
using semgraph::Env;
using semgraph::Parameter;
using semgraph::Type;
using semgraph::TypeKey;

class TypeStore {
public:
//   TypeStore(support::Arena& arena) : _arena(arena) {}

  /** TypeStore has its own arena. */
  support::Arena& arena() { return _arena; }

  /** Return the type of the specified member. If the member is a specialized member, it may
      be necessary to construct a specialized type. */
  Type* memberType(Member* m);

  /** Create an environment object from a set of type mappings. */
  Env createEnv(const semgraph::EnvMap& env);

  /** Create a union type from the given type key. */
  UnionType* createUnionType(const TypeArray& members);

  /** Create a tuple type from the given type key. */
  TupleType* createTupleType(const TypeArray& members);

  /** Create a function type from a return type and parameter types. */
  FunctionType* createFunctionType(Type* returnType, const TypeArray& paramTypes);

  /** Create a function type from a return type and a parameter list. */
  FunctionType* createFunctionType(Type* returnType, const ArrayRef<Parameter*>& params);

  /** Create a const type. */
  ConstType* createConstType(Type* base, bool provisional);

private:
  typedef std::pair<Type*, bool> ConstKey;
  struct ConstKeyHash {
    inline std::size_t operator()(const ConstKey& value) const {
      std::size_t result = std::hash<Type*>()(value.first);
      std::hash_combine(result, std::hash<bool>()(value.second));
      return result;
    }
  };
  struct ConstKeyEqual {
    inline bool operator()(const ConstKey& key0, const ConstKey& key1) const {
      return key0.first == key1.first && key0.second == key1.second;
    }
  };

  support::Arena _arena;
  std::unordered_set<semgraph::EnvMap> _envs;
//     self.uniqueEnvs = {}
//     self.uniqueTypes = {}
//     self.phiTypes = {}
  std::unordered_map<TypeKey, UnionType*> _unionTypes;
  std::unordered_map<TypeKey, TupleType*> _tupleTypes;
  std::unordered_map<TypeKey, FunctionType*> _functionTypes;
  std::unordered_map<ConstKey, ConstType*, ConstKeyHash> _constTypes;
  std::unordered_set<Type*> _addressTypes;
//     self.valueRefTypes = {}
};

}}}

#endif
