import spark.core.memory;
import spark.core.memory.Address;

/** Interface representing an object that can compute a hash code for itself. */
interface Hashable {
  def hashCode() -> u32;
}

/** Interface that defines hashing and comparison operations for map keys and set elements. */
interface HashComparator[T] {
  def hash(key:T) -> u32;
  def equals(k0:T, k1:T) -> bool;
}

/** Refinement of `HashComparator` that provides implementations for hashing and comparison of keys
    for types that implement the `Hashable` interface. */
final class DefaultHashComparator[T <: Hashable] : HashComparator[T] where T == T {
  override hash(key:T) -> u32 => key.hashCode();
  override equals(k0:T, k1:T) -> bool => k0 == k1;
}

/** Refinement of `HashComparator` that provides implementations for primitive types. */
final class DefaultHashComparator[T] : HashComparator[T] where hashing.hash(T) -> u32 and T == T {
  override hash(key:T) -> u32 => hashing.hash(key);
  override equals(k0:T, k1:T) -> bool => k0 == k1;
}

/** Hash a single 32-bit integer. */
def hash(key:i32) -> u32 {
  return murmurHash(u32(key), 0);
}

/** Hash a single 32-bit integer (unsigned). */
def hash(key:u32) -> u32 {
  return murmurHash(key, 0);
}

/** Hash a single 64-bit integer. */
def hash(key:i64) -> u32 {
  return murmurHash(u64(key), 0);
}

/** Hash a single 64-bit integer (unsigned). */
def hash(key:u32) -> u32 {
  return murmurHash(key, 0);
}

def hash(first:Address[u8], last:Address[u8], seed:u32 = 0) -> u32 {
  return murmurHash(first, last, seed);
}

private let M32:u32 = 0x5bd1e995u;
private let M64:u64 = 0xc6a4a7935bd1e995u;

private def murmurHash(key:u32, seed:u32 = 0) -> u32 {
  var h:u32 = seed ^ 1;
  key *= M32;
  key ^= key >> 24;
  key *= M32;

  h *= M32;
  h ^= key;

  h ^= h >> 13;
  h *= M32;
  h ^= h >> 15;

  return h;
}

private def murmurHash(key:u64, seed:u64 = 0) -> u32 {
  let R:u32 = 47;

  var h:u64 = seed ^ M64;
  key *= M64;
  key ^= key >> R;
  key *= M64;

  h ^= key;
  h *= M64;

  h ^= h >> R;
  h *= M64;
  h ^= h >> R;

  return u32(h);
}

/** Hash a pointer. */
//def murmurHash(key:Address[void]) -> u32 {
//  return murmurHash(Memory.ptrToInt(key));
//}

/** Hash a byte buffer segment. */
//def murmurHash(buffer:Memory.Buffer[ubyte], seed:u32 = 0) -> u32 {
//  return murmurHash(buffer.begin, buffer.end, seed);
//}

/** Hash a region of memory. */
private def murmurHash(first:Address[u8], last:Address[u8], seed:u32 = 0) -> u32 {
  if true {  // TODO: Change to config.allowUnalignedRead
    let R:u64 = 47;
    var len:i64 = memory.ptrDiff(first, last);
    var h:u64 = seed ^ (u64(len) * M64);
    var data:Address[u64] = memory.reinterpret(first);
    var end:Address[u64] = data.element(len / 8);

    while data != end {
      var k:u64 = data(0);
      data += 1;

      k *= M64;
      k ^= k >> R;
      k *= M64;

      h ^= k;
      h *= M64;
    }

    if (len & 7) != 0 {
      var data2:Address[u8] = memory.reinterpret(data);
      if (len & 4) != 0 {
        let d:Address[u32] = memory.reinterpret(data2);
        data2 += 4;
        h ^= u64(d(0)) << 32;
      }

      if (len & 2) != 0 {
        let d:Address[u16] = memory.reinterpret(data2);
        data2 += 2;
        h ^= u64(d(0)) << 16;
      }

      if (len & 1) != 0 {
        let d:Address[u8] = memory.reinterpret(data2);
        data2 += 1;
        h ^= u64(d(0)) << 8;
      }

      h *= M64;
    }

    h ^= h >> R;
    h *= M64;
    h ^= h >> R;

    return u32(h);
  }

  return 0;
}
