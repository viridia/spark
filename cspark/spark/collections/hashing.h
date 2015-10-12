// ============================================================================
// Coda Object
// ============================================================================

#ifndef SPARK_COLLECTIONS_HASHING_H
#define SPARK_COLLECTIONS_HASHING_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#if SPARK_HAVE_FUNCTIONAL
  #include <functional>
#endif

#if SPARK_HAVE_MATH_H
  #include <math.h>
#endif

#if SPARK_HAVE_STDINT_H
  #include <stdint.h>
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref"
#endif

namespace std {

/** Combine two hash vaues. */
inline void hash_combine(std::size_t& lhs, std::size_t rhs) {
  lhs ^= rhs + 0x9e3779b9 + (lhs<<6) + (lhs>>2);
}

/** Compute a hash for a StringRef. */
template<>
struct hash<spark::collections::StringRef> {
  inline std::size_t operator()(const spark::collections::StringRef& value) const {
    std::size_t seed = 0;
    for (char c : value) {
      hash_combine(seed, std::hash<char>()(c));
    }
    return seed;
  }
};

}

#endif
