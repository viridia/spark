// ============================================================================
// sema/types/ordering.h: Class for creating and managing derived types.
// ============================================================================

#ifndef SPARK_SEMA_TYPES_ORDERING_H
#define SPARK_SEMA_TYPES_ORDERING_H 1

#ifndef SPARK_SEMGRAPH_EXPR_H
  #include "spark/semgraph/expr.h"
#endif

#ifndef SPARK_SEMGRAPH_TYPE_H
  #include "spark/semgraph/type.h"
#endif

#if SPARK_HAVE_UNORDERED_SET
  #include <unordered_set>
#endif

namespace spark {
namespace sema {
namespace types {

struct Ordering {
};

}}}

#endif
