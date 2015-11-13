// ============================================================================
// sema/types/applyenv.h: transform a type expression through an environment.
// ============================================================================

#ifndef SPARK_SEMA_TYPES_APPLYENV_H
#define SPARK_SEMA_TYPES_APPLYENV_H 1

#ifndef SPARK_SEMGRAPH_TYPEVISITOR_H
  #include "spark/semgraph/typevisitor.h"
#endif

#ifndef SPARK_COLLECTIONS_MAP_H
  #include "spark/collections/map.h"
#endif

#ifndef SPARK_SEMA_TYPES_TRANSFORM_H
  #include "spark/sema/types/transform.h"
#endif

namespace spark {
namespace sema {
namespace types {

/** Class that transforms a type expression, using an environment map to substitute type
    variables. */
class ApplyEnv : public Transform<const EnvMap&> {
public:
  ApplyEnv(TypeStore* typeStore) : Transform<const EnvMap&>(typeStore) {}

  Member* specializeMember(Member* m, const EnvMap& env);
};

}}}

#endif
