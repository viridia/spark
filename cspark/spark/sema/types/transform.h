// ============================================================================
// sema/types/applyenv.h: transform a type expression through an environment.
// ============================================================================

#ifndef SPARK_SEMA_TYPES_TRANSFORM_H
#define SPARK_SEMA_TYPES_TRANSFORM_H 1

#ifndef SPARK_SEMGRAPH_ENV_H
  #include "spark/semgraph/env.h"
#endif

#ifndef SPARK_SEMGRAPH_TYPEVISITOR_H
  #include "spark/semgraph/typevisitor.h"
#endif

#ifndef SPARK_COLLECTIONS_MAP_H
  #include "spark/collections/map.h"
#endif

#ifndef SPARK_SEMA_TYPES_TYPESTORE_H
  #include "spark/sema/types/typestore.h"
#endif

namespace spark {
namespace sema {
namespace types {
using semgraph::EnvMap;

/** Base class that performs an arbitrary transformation on a type exprpession. */
template<typename... Args>
class Transform : public semgraph::TypeVisitor<Type*, Args...> {
public:
  Transform(TypeStore* typeStore) : _typeStore(typeStore) {}

  Type* visitType(Type* t, Args&&... args) {
    return t;
  }

  Type* visitComposite(Composite* t, Args&&... args) {
    return t;
  }

//   Type* visitClass(Composite* t, Args&&... args);
//   Type* visitStruct(Composite* t, Args&&... args);
//   Type* visitInterface(Composite* t, Args&&... args);
//   Type* visitEnum(Composite* t, Args&&... args);
//   Type* visitExtension(Composite* t, Args&&... args);

  Type* visitUnionType(UnionType* t, Args&&... args) {
    std::vector<Type*> members = transform(t->members(), std::forward<Args>(args)...);
    return _typeStore->createUnionType(members);
  }

  Type* visitTupleType(TupleType* t, Args&&... args) {
    std::vector<Type*> members = transform(t->members(), std::forward<Args>(args)...);
    return _typeStore->createTupleType(members);
  }

  /** Call the appropriate visitor functions for a collection of definitions. */
  std::vector<Type*> transform(const collections::ArrayRef<Type*>& types, Args&&... args) {
    std::vector<Type*> result;
    result.reserve(types.size());
    for (Type* t : types) {
      result.push_back(this->exec(t, std::forward<Args>(args)...));
    }
    return result;
  }

private:
  TypeStore* _typeStore;
};

}}}

#endif
