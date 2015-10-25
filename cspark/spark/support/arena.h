// ============================================================================
// arena.h: Memory allocation pools.
// ============================================================================

#ifndef SPARK_SUPPORT_ARENA_H
#define SPARK_SUPPORT_ARENA_H 1

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

#ifndef SPARK_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

#if SPARK_HAVE_NEW
  #include <new>
#endif

namespace spark {
namespace support {

/** Memory arena. */
class Arena {
private:
  struct Block {
    Block* _next;
  };

  static const std::size_t BLOCK_HEADER_SIZE = (sizeof(Block) + 7) & ~7;
public:
  typedef uint8_t value_type;
  /** Default block size. */
  static const std::size_t DEFAULT_BLOCK_SIZE = 0x10000 - BLOCK_HEADER_SIZE; // 64K

  Arena(std::size_t blockSize = DEFAULT_BLOCK_SIZE)
    : _head(nullptr)
    , _pos(nullptr)
    , _end(nullptr)
    , _blockSize(blockSize)
  {}

  ~Arena() {
    clear();
  }

  /** Allocate a block of at least size `size`. */
  value_type* allocate(std::size_t n) {
    n = (n + 7) & ~7; // Round up to nearest 8.
    size_t freeSpace = _end - _pos;
    if (freeSpace < n) {
      // TODO: In cases where the new block will be fuller than the previous block, we could
      // play games with keeping the previous block at the head of the list.
      size_t blkSize = std::max(n + BLOCK_HEADER_SIZE, _blockSize);
      uint8_t* blk = new uint8_t[blkSize];
      _pos = &blk[BLOCK_HEADER_SIZE];
      _end = &blk[blkSize];
      reinterpret_cast<Block*>(blk)->_next = _head;
      _head = reinterpret_cast<Block*>(blk);
    }
    uint8_t* result = _pos;
    _pos += n;
    return result;
  }

  /** Deallocate does nothing. */
  void deallocate(value_type* p, std::size_t n) {}

  /** Free all memory. */
  void clear() {
    while (_head) {
      Block* blk = _head;
      _head = _head->_next;
      delete reinterpret_cast<uint8_t*>(blk);
    }
  }

  /** Make a long-lived copy of this StringRef. */
  collections::StringRef copyOf(const collections::StringRef& str) {
    value_type* data = allocate(str.size());
    std::copy(str.begin(), str.end(), data);
    return collections::StringRef((char*) data, str.size());
  }

  /** Make a long-lived copy of this ArrayRef. */
  template<class T>
  collections::ArrayRef<T> copyOf(const collections::ArrayRef<T>& array) {
    T* data = reinterpret_cast<T*>(allocate(array.size() * sizeof(T)));
    std::copy(array.begin(), array.end(), data);
    return collections::ArrayRef<T>((T*) data, array.size());
  }
private:
  Block* _head;
  value_type *_pos;
  value_type *_end;
  size_t _blockSize;

  // Hide copy constructor
  Arena(const Arena& a);
  Arena& operator=(const Arena& a);
};

/** Allocator compatible with C++ std containers. */
class ArenaRef {
public:
  typedef uint8_t value_type;

  ArenaRef(Arena& arena) : _arena(arena) {}
  ArenaRef(ArenaRef& ref) : _arena(ref._arena) {}

  friend bool operator==(const ArenaRef& l, const ArenaRef& r) {
    return &l._arena == &r._arena;
  }
  friend bool operator!=(const ArenaRef& l, const ArenaRef& r) {
    return &l._arena != &r._arena;
  }

  /** Allocate a block of at least size `size`. */
  value_type* allocate(std::size_t n) {
    return _arena.allocate(n);
  }

  /** Deallocate does nothing. */
  void deallocate(value_type* p, std::size_t n) {}

  /** Reference to the arena. */
  Arena& arena() const { return _arena; }

private:
  Arena& _arena;
};

}}

inline void* operator new(size_t sz, spark::support::Arena& a) {
  return a.allocate(sz);
}

inline void* operator new(size_t sz, spark::support::ArenaRef& ar) {
  return ar.allocate(sz);
}

#endif
