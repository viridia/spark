// ============================================================================
// sema/pass/passid.h: Defines a unique id for each pass.
// ============================================================================

#ifndef SPARK_SEMA_PASSES_PASSID_H
#define SPARK_SEMA_PASSES_PASSID_H 1

namespace spark {
namespace sema {
namespace passes {

enum PassId {
  BUILD_GRAPH,
  NAME_RESOLUTION,

  PASS_COUNT,
};

}}}

#endif
