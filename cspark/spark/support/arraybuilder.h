// ============================================================================
// builder.h: Array builder
// ============================================================================

#ifndef SPARK_SUPPORT_ARRAYBUILDER_H
#define SPARK_SUPPORT_ARRAYBUILDER_H 1

#ifndef SPARK_SUPPORT_ARENA_H
  #include "spark/support/arena.h"
#endif

#ifndef SPARK_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

namespace spark {
namespace support {

/** Helper for building arrays in an arena. */
template<class T>
class ArrayBuilder {
public:
  typedef T value_type;

  ArrayBuilder(support::Arena& arena) : _arena(arena) {}
  ArrayBuilder(support::Arena& arena, const collections::ArrayRef<T>& init)
    : _arena(arena)
    , _elements(init.begin(), init.end())
  {}
  ArrayBuilder(support::Arena& arena, const std::vector<value_type>& init)
    : _arena(arena)
    , _elements(init)
  {}
  ArrayBuilder(support::Arena& arena, const std::vector<value_type>&& init)
    : _arena(arena)
    , _elements(init)
  {}

  /** Append an element to the array. */
  ArrayBuilder& append(const T& elt) {
    _elements.push_back(elt);
    return *this;
  }

  /** The number of elements in the array. */
  size_t size() const {
    return _elements.size();
  }

  const T& operator[](int index) const {
    return _elements[index];
  }

  /** Iterate over the collection. */
  typename std::vector<value_type>::const_iterator begin() const { return _elements.begin(); }
  typename std::vector<value_type>::const_iterator end() const { return _elements.end(); }

  /** Allocate an array in the arena and return a reference to it. */
  collections::ArrayRef<T> build() const {
    if (_elements.empty()) {
      return collections::ArrayRef<value_type>();
    }
    T* data = reinterpret_cast<T*>(_arena.allocate(sizeof(T) * _elements.size()));
    collections::ArrayRef<value_type> result(data, _elements.size());
    std::copy(_elements.begin(), _elements.end(), data);
    return result;
  }
private:
  support::Arena& _arena;
  std::vector<value_type> _elements;
};

}}

#endif
