// ============================================================================
// orderedset.h.
// ============================================================================

#ifndef SPARK_COLLECTIONS_ORDEREDSET_H
#define SPARK_COLLECTIONS_ORDEREDSET_H 1

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

/** A set which remembers the order in which elements were inserted. */
template<class T, class Hasher = std::hash<T> >
class OrderedSet {
public:
  typedef T value_type;
  typedef T* iterator;
  typedef T* const_iterator;
  typedef T& reference;
  typedef T& const_reference;

  /** Return the size of the string in bytes. */
  size_t size() const { return _order.size(); }

  /** True if the length is zero. */
  bool empty() const { return _order.size() == 0; }

  // Iterators

  iterator begin() const { return _order.begin(); }
  iterator end() const { return _order.end(); }

  // Methods

  void clear() {
    _set.clear();
    _order.clear();
  }

  /** Read-only random access. */
  T operator[](int index) const {
    return _order[index];
  }

  /** Insert an element into the set. Does nothing if the element is already present*/
  bool insert(const T& value) {
    if (_set.find(value) == _set.end()) {
      _set.insert(value);
      _order.push_back(value);
      return true;
    }
    return false;
  }

private:
  std::vector<T, Hasher> _order;
  std::unordered_set<T> _set;
};

}}

#endif
