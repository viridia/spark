/** A quadratically-probed hash map. */
final class HashMap[
    Key,
    Value,
    Comparator <: hashing.HashComparator[Key] = hashing.DefaultHashComparator[Key]]
  : Map[Key, Value]
    where Comparator.new()
{
  private {
    // Hash function and comparator
    static let comparator = Comparator();

    // Special sentinel hash values for empty and deleted slots.
    static let EMPTY:u32 = u32(-1);
    static let DELETED:u32 = u32(-2);
    static def occupied(hashVal:u32) -> bool => hashVal != EMPTY and hashVal != DELETED;

    struct Entry {
      let key:Key;
      let value:Value;
      let hash:u32;

      def new() {
        .key = memory.defaultValue[Key]();
        .value = memory.defaultValue[Value]();
        .hash = EMPTY;
      }
      def new(key:Key, value:Value, hash:u32) {
        .key = key;
        .value = value;
        .hash = hash;
      }
    }

    var data:Array[Entry] = Array[Entry].EMPTY;
    var sz:int;
    // var version:uint = 0;
    // var mutated = false;
    var modified:bool = false;
    // var iterating = false;

    /** Returns the array index for the element with the specified key, or -1 if there is
        no such element. */
    def findEntry(key:Key) -> int {
      if data.size > 16 {
        let mask = data.size - 1;
        var index = int(comparator.hash(key)) & mask;
        var probe = 1;

        // First, see if the item is already in the table
        while probe <= mask {
          let hash = self.data(index).hash;
          if hash == EMPTY {
            return -1;
          }
          if hash != DELETED and comparator.equals(key, self.data(index).key) {
            return index;
          }
          index = (index + probe) & mask;
          probe += 1;
        }
      } else if data.size > 0 {
        for i = 0; i < size; ++i {
          if comparator.equals(key, data(i).key) {
            return i;
          }
        }
      }
      return -1;
    }

    def rehash(oldItems:Array[Entry], oldLength:int, newLength:int) {
      let mask = newLength - 1;
      self.data = Array[Entry](newLength);
      for i = 0; i < oldLength; ++i {
        if occupied(oldItems(i).hash) {
          let key = oldItems(i).key;
          var index = oldItems(i).hash & mask;
          var probe = 1;
          // Rehashing presumes that all keys are already unique.
          while occupied(self.data(index).hash) {
            index = (index + probe) & mask;
            probe += 1;
            if probe > 4 and newLength < sz * 8 {
              // If it takes more than 4 probes, then expand the table again and
              // start the rehashing over.
              rehash(oldItems, oldLength, newLength * 2);
              return;
            }
          }

          data(index) = oldItems(i);
        }
      }
    }

    def addEntry(key:Key, value:Value) -> bool {
      modified = true;
      let hashValue = comparator.hash(key);
      loop {
        if data.size > 16 {
          // Compute the hash of the item
          let mask = data.size - 1;
          var index = int(hashValue) & mask;
          var probe:int = 1;

          // First, see if the item is already in the table
          while probe <= 4 or data.size >= sz * 8 {
            if not occupied(self.data(index).hash) {
              data(index) = Entry(key, value, hashValue);
              sz++;
              return false;
            }

            if self.data(index).hash == hashValue and comparator.equals(self.data(index).key, key) {
              data(index) = Entry(key, value, hashValue);
              return true;
            }

            index = (index + probe) & mask;
            probe += 1;
          }

          // Fall through and rehash
        } else if data.size > 0 {
          for i = 0; i < sz; ++i {
            if data(i).hash == hashValue and comparator.equals(data(i).key, key) {
              data(i) = Entry(key, value, hashValue);
              return true;
            }
          }

          if sz < 16 {
            data(sz) = Entry(key, value, hashValue);
            sz++;
            return false;
          }
        } else {
          data = Array[Entry](16);
          data(0) = Entry(key, value, hashValue);
          sz = 1;
          return false;
        }

        // Rehash and try again
        rehash(data, data.size, data.size * 2);
      }
      false // TODO: Compiler isn't smart enough to detect that loop never falls through.
    }
  }

  def new() {}
  def new(entries:(Key, Value)...) {
    addAll(entries);
  }

  /** Access a value by key. */
  override (key:Key):Value {
    get {
      let index:int = findEntry(key);
      if index < 0 {
        throw KeyError();
      }

      return data(index).value;
    }

    set {
      addEntry(key, value);
    }
  }

  override get(key:Key, default:Value = memory.defaultValue[Value]()) -> Value {
    let index:int = findEntry(key);
    if index < 0 { default } else { data(index).value }
  }

  override size:int { get => sz; }

  override empty:bool { get => sz == 0; }

  override contains(key:Key) -> bool => findEntry(key) >= 0;

  override clear() {
    data = Array(0);
    sz = 0;
  }

  override keys:Iterator[Key] { get => KeyIterator(self); }
  override values:Iterator[Value] { get => ValueIterator(self); }
  override iterate(self:const) -> Iterator[(Key, Value)] => ItemsIterator(self);

  override add(entry:(Key, Value)) {
    addEntry(entry._0, entry._1);
  }

  override add(entries:(Key, Value)...) {
    addAll(entries);
  }

  override addAll(entries:Iterable[(Key, Value)]) {
    for key, value in entries {
      addEntry(key, value);
    }
  }

  // To implement
  undef remove(key:Key) -> bool;
  undef remove(keys:Key...);
  undef removeAll(keys:Iterable[Key]);

  private class IteratorBase {
    protected {
      let map:HashMap;
      var index:int;
      var maxIndex:int;
    }

    protected def new(map:HashMap) {
      .map = map;
      .index = 0;
      .maxIndex = if map.data.size <= 16 { map.sz } else { map.data.size }
    }
  }

  final class KeyIterator : IteratorBase, Iterator[Key] {
    override next -> Key or void {
      while index < maxIndex {
        let pos = index++;
        if occupied(map.data(pos).hash) {
          return map.data(pos).key;
        }
      }
    }
  }

  final class ValueIterator : IteratorBase, Iterator[Value] {
    override next -> Value or void {
      while index < maxIndex {
        let pos = index++;
        if occupied(map.data(pos).hash) {
          return map.data(pos).value;
        }
      }
    }
  }

  final class ItemsIterator : IteratorBase, Iterator[(Key, Value)] {
    override next -> (Key, Value) or void {
      while index < maxIndex {
        let pos = index++;
        if occupied(map.data(pos).hash) {
          var entry = map.data(pos);
          return (entry.key, entry.value);
        }
      }
    }
  }
}
