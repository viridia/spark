// ============================================================================
// A map optimized for small numbers of elements.
// ============================================================================

#ifndef SPARK_COLLECTIONS_MAP_H
#define SPARK_COLLECTIONS_MAP_H 1

#ifndef SPARK_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

#if SPARK_HAVE_UTILITY
  #include <utility>
#endif

namespace spark {
namespace collections {

/** Abstract base class for small maps. These maps don't use a hashing algorithm, just a linear
    O(n) search. These classes are intended for very small collections (<8), where the cost
    of hashing is greater than the cost of simple equality-testing of every element. These maps
    also provide a predictable iteration order. */
template <typename K, typename V>
class ReadableMap {
public:
  typedef K key_type;
  typedef V value_type;
  typedef std::pair<K, V> element_type;
  typedef element_type* iterator;
  typedef const element_type* const_iterator;

  ReadableMap() : _data(nullptr), _size(0) {}

  /** The number of elements in the set. */
  size_t size() const { return _size; }

  /** Pointer to the element data. */
  const element_type* data() const { return _data; }

  /** Find an element in the set. */
  const_iterator find(const K& key) const {
    for (int i = 0; i < _size; ++i) {
      if (_data[i].first == key) {
        return &_data[i];
      }
    }
    return &_data[_size];
  }

  /** Compare two maps. */
  bool operator==(const ReadableMap<K, V>& other) const {
    if (other.size() != size()) {
      return false;
    }
    for (auto entry : *this) {
      auto it = other.find(entry.first);
      if (it == other.end() || it->second != entry.second) {
        return false;
      }
    }
    return true;
  }

  /** Iterators. Items will be interated in order of insertion. */
  const_iterator begin() const { return _data; }
  const_iterator end() const { return _data + _size; }

protected:
  // Constructor that allows the map to be pre-populated.
  ReadableMap(element_type* data, size_t size) : _data(data), _size(size) {}

  iterator findEntry(const K& key) const {
    for (int i = 0; i < _size; ++i) {
      if (_data[i].first == key) {
        return &_data[i];
      }
    }
    return &_data[_size];
  }

  element_type* _data;
  std::size_t _size;
};

/** An immutable map. */
template <typename K, typename V>
class ImmutableMap : public ReadableMap<K, V> {
public:
  typedef typename ReadableMap<K, V>::element_type element_type;
  typedef typename ReadableMap<K, V>::const_iterator const_iterator;

  ImmutableMap() : ReadableMap<K, V>(nullptr, 0) {}
  ImmutableMap(element_type* data, size_t size) : ReadableMap<K, V>(data, size) {}
  // I hate using const_cast, but in this case we know ReadableMap is never going to mutate it.
  ImmutableMap(const element_type* data, size_t size)
    : ReadableMap<K, V>(const_cast<element_type*>(data), size) {}
//   ImmutableMap(const ArrayRef<element_type>& src) : ReadableMap<K, V>(src.begin(), src.size()) {}
  ImmutableMap(const ImmutableMap& src) : ReadableMap<K, V>(src._data, src.size()) {}

  ImmutableMap& operator=(const ImmutableMap& src) {
    this->_data = src._data;
    this->_size = src._size;
    return *this;
  }

  /** Array access operator. */
  const V& operator[](const K& key) const {
    const_iterator it = findEntry(key);
    if (it != this->end()) {
      return it->second;
    }
    assert(false && "Entry not found");
  }
};

/** An abstract map that can allocate additional capacity as elements are inserted. Elements
    are always kept in insertion order. */
template <typename K, typename V>
class SmallMapBase : public ReadableMap<K, V> {
public:
  typedef typename ReadableMap<K, V>::element_type element_type;
  typedef typename ReadableMap<K, V>::iterator iterator;
  typedef typename ReadableMap<K, V>::const_iterator const_iterator;

  SmallMapBase() : _alloc(nullptr), _capacity(0) {}
  ~SmallMapBase() {
    if (_alloc != nullptr) {
      delete _alloc;
    }
  }

  /** Insert a key and value into the map. */
  iterator insert(const element_type& element) {
    iterator it = findEntry(element.key);
    if (it == this->end()) {
      return append(element);
    } else {
      it->second = element.second;
      return it;
    }
  }

  /** Insert a key and value into the map. */
  iterator insert(const K& key, const V& value) {
    return insert(std::pair<K, V>(key, value));
  }

  /** Array access operator (mutable version). */
  V& operator[](const K& key) {
    iterator it = this->findEntry(key);
    if (it != this->end()) {
      return it->second;
    }
    it = append(std::pair<K, V>(key, V()));
    return it->second;
  }

  /** Array access operator (const version). */
  const V& operator[](const K& key) const {
    iterator it = this->findEntry(key);
    if (it != this->end()) {
      return it->second;
    }
    assert(false && "Entry not found");
  }

protected:
  SmallMapBase(element_type* data, size_t capacity)
    : ReadableMap<K, V>(data, 0)
    , _alloc(nullptr)
    , _capacity(capacity)
  {}

  iterator append(const element_type& element) {
    if (this->_size + 1 >= _capacity) {
      resize();
    }
    iterator it = &this->_data[this->_size++];
    *it = element;
    return it;
  }

  void resize() {
    _capacity = this->_size * 2;
    // TODO: This calls constructors, which we should avoid. Should use uninitialized_copy here.
    element_type* data = new element_type[_capacity];
    std::copy(this->_data, &this->_data[this->_size], data);
    if (_alloc != nullptr) {
      delete _alloc;
    }
    this->_data = _alloc = data;
  }

  element_type* _alloc;
  size_t _capacity;
};

/** A map class optimized for small numbers of elements. Maps less than size N will not allocate
    any memory on the heap. Items are kept in insertion order. Does not require a hash function. */
template <typename K, typename V, int N>
class SmallMap : public SmallMapBase<K, V> {
public:
  typedef typename ReadableMap<K, V>::element_type element_type;

  SmallMap() : SmallMapBase<K, V>(_elements, N) {}

private:
  element_type _elements[N];
};

}}

#endif
