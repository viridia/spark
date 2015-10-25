// ============================================================================
// Agenda.
// ============================================================================

#ifndef SPARK_COLLECTIONS_AGENDA_H
#define SPARK_COLLECTIONS_AGENDA_H 1

#if SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#if SPARK_HAVE_UNORDERED_SET
  #include <unordered_set>
#endif

#if SPARK_HAVE_VECTOR
  #include <vector>
#endif

namespace spark {
namespace collections {

template<class T, class Hasher = std::hash<T> >
class Agenda {
public:
  typedef T* iterator;
  typedef T* const_iterator;
  typedef T& reference;
  typedef T& const_reference;

  /** Return the size of the string in bytes. */
  size_t size() const { return _size; }

  /** True if the length is zero. */
  bool empty() const { return _size == 0; }

  /** Equality comparison. */
  friend bool operator==(const StringRef& lhs, const StringRef& rhs) {
    if (lhs._size == rhs._size) {
      return std::memcmp(lhs._data, rhs._data, lhs._size) == 0;
    }
    return false;
  }

  /** Equality comparison. */
  friend bool operator!=(const StringRef& lhs, const StringRef& rhs) {
    return !operator==(lhs, rhs);
  }

  // Iterators

  iterator begin() const { return _data; }
  iterator end() const { return _data + _size; }

  // Methods

  /** Read-only random access. */
  char operator[](int index) const {
    assert(index >= 0 && index < _size);
    return _data[index];
  }

private:
  std::unordered_set<T> _contains;
  std::vector<T> _order;
};

}}

#endif
