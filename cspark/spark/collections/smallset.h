// ============================================================================
// A set optimized for small numbers of elements.
// ============================================================================

#ifndef SPARK_COLLECTIONS_SMALLSET_H
#define SPARK_COLLECTIONS_SMALLSET_H 1

#ifndef SPARK_SEMA_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

namespace spark {
namespace collections {

/** Base class for SmallSet. Small sets don't use hash functions and are O(n), and are thus only
    useful for small collections, but the most important feature is predicatbility: elements are
    always kept in insertion order. (Having a compiler that processes symbols in different order
    depending on the memory layout of the executable is a rich source of frustration.)
  */
template <class T>
class SmallSetBase {
public:
  typedef T value_type;
  typedef const T* iterator;
  typedef const T* const_iterator;

  SmallSetBase() : _data(nullptr), _alloc(nullptr),  _size(0), _capacity(0) {}
  ~SmallSetBase() {
    if (_alloc != nullptr) {
      delete _alloc;
    }
  }

  /** The number of elements in the set. */
  size_t size() const { return _size; }

  /** True if the container is empty. */
  bool empty() const { return _size == 0; }

  /** Find an element in the set. */
  const_iterator find(const T& element) const {
    for (int i = 0; i < _size; ++i) {
      if (_data[i] == element) {
        return &_data[i];
      }
    }
    return &_data[_size];
  }

  void insert(const T& element) {
    if (find(element) == end()) {
      if (_size + 1 >= _capacity) {
        resize();
      }
      _data[_size++] = element;
    }
  }

  void insert(const_iterator first, const_iterator last) {
    while (first != last) {
      insert(*first++);
    }
  }

  /** Iterators. Items will be interated in order of insertion. */
  const_iterator begin() const { return _data; }
  const_iterator end() const { return _data + _size; }

  /** Cast to array ref. */
  operator const ArrayRef<T>(){ return ArrayRef<T>(_data, _size); }

protected:
  SmallSetBase(T* data, size_t capacity)
    : _data(data)
    , _alloc(nullptr)
    , _size(0)
    , _capacity(capacity)
  {}

  void resize() {
    _capacity = _size * 2;
    T* data = new T[_capacity];
    std::copy(_data, &_data[_size], data);
    if (_alloc != nullptr) {
      delete _alloc;
    }
    _data = _alloc = data;
  }

  T* _data;
  T* _alloc;
  size_t _size;
  size_t _capacity;
};

/** A set class optimized for small numbers of elements. Sets less than size N will not allocate
    any memory on the heap. Items are kept in insertion order. Does not require a hash function. */
template <class T, int N>
class SmallSet : public SmallSetBase<T> {
public:
  SmallSet() : SmallSetBase<T>(_elements, N) {}

private:
  T _elements[N];
};

}}

#endif
