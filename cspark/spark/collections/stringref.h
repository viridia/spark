// ============================================================================
// Efficient polymorphic strings.
// ============================================================================

#ifndef SPARK_COLLECTIONS_STRINGREF_H
#define SPARK_COLLECTIONS_STRINGREF_H 1

#include <spark/config.h>

#if SPARK_HAVE_STDDEF_H
  #include <stddef.h>
#endif

#if SPARK_HAVE_STRING
  #include <string>
#endif

#if SPARK_HAVE_CSTRING
  #include <cstring>
#endif

#if SPARK_HAVE_OSTREAM
  #include <ostream>
#endif

#if SPARK_HAVE_FUNCTIONAL
  #include <functional>
#endif

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace collections {

class StringRef {
public:
  typedef const char* iterator;
  typedef const char* const_iterator;

  /// Construct an empty StringRef.
  StringRef() : _data(NULL), _size(0) {}

  /** Construct a StringRef from a C string. */
  template <size_t Size>
  StringRef(const char (&array)[Size]) : _data(array), _size(Size - 1) {}

  /** Construct a StringRef from a null-terminated C string. */
  StringRef(const char* str) : _data(str), _size(std::strlen(str)) {}

  /** Construct a StringRef from an STL string. */
  StringRef(const std::string& str) : _data(str.data()), _size(str.size()) {}

  /** Construct a StringRef from a character array with explicit size. */
  StringRef(const char* str, size_t size) : _data(str), _size(size) {}

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

  /** Return true if this string starts with `prefix`. */
  bool startsWith(const StringRef& prefix) const {
    if (prefix._size <= _size) {
      return std::memcmp(prefix._data, _data, prefix._size) == 0;
    }
    return false;
  }

  /** Return true if this string ends with `suffix`. */
  bool endsWith(const StringRef& suffix) const {
    if (suffix._size <= _size) {
      return std::memcmp(suffix._data, &_data[_size - suffix._size], suffix._size) == 0;
    }
    return false;
  }

  /** Read-only random access. */
  char operator[](int index) const {
    assert(index >= 0 && index < _size);
    return _data[index];
  }

  /** Return a substring of this string. */
  StringRef substr(int start, int end = 0x7fff) const {
    assert(start >= 0 && start <= _size);
    assert(end >= start);
    if (end > _size) {
      end = _size;
    }
    return StringRef(&_data[start], end - start);
  }

  /** Convert this StringRef to an STL string. */
  std::string str() const { return std::string(_data, _size); }

private:
  const char* _data;
  size_t _size;
};

// How to print a token type.
inline ::std::ostream& operator<<(::std::ostream& os, const StringRef& str) {
  os.write(str.begin(), str.size());
  return os;
}

}}

#endif // SPARK_COMMON_STRINGREF_H