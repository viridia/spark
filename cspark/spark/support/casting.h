// ============================================================================
// casting.h: Dynamic casts.
// ============================================================================

#ifndef SPARK_SUPPORT_CASTING_H
#define SPARK_SUPPORT_CASTING_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#include <../lib/spark/collections/array.sp>
#endif

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace support {

template<typename T>
struct SimplifyType {
  typedef T value_type;
};

template<typename T>
struct SimplifyType<T*> {
  typedef T value_type;
};

/** Returns true if the runtime type of the argument is 'To'. */
template<typename To, typename From>
inline bool isa(const From& val) {
  return SimplifyType<To>::value_type::classof(val);
}

/** Returns true if the runtime type of the argument is 'To'. */
template<typename To, typename From>
inline To dyn_cast(const From& val) {
  assert(val != nullptr);
  return isa<To>(val) ? static_cast<To>(val) : nullptr;
}

/** Returns true if the runtime type of the argument is 'To'. */
template<typename To, typename From>
inline To dyn_cast_or_null(const From& val) {
  return isa<To>(val) ? static_cast<To>(val) : nullptr;
}

}}

#endif
