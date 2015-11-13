// ============================================================================
// semgraph/types.h: Semantic graph nodes for types.
// ============================================================================

#ifndef SPARK_SEMGRAPH_TYPES_H
#define SPARK_SEMGRAPH_TYPES_H 1

#ifndef SPARK_SEMGRAPH_TYPE_H
  #include "spark/semgraph/type.h"
#endif

namespace spark {
namespace semgraph {
namespace types {

/** Strip off modifiers and specializations. */
Type* raw(Type*);

}}}

#endif
