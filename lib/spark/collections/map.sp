import spark.core.memory;

/** Map interface */
interface Map[Key, Value] : Collection[(Key, Value)] {

  /** Access a value by key. */
  def (key:Key):Value { get; set; }

  /** Access a value by key, returning a default value if not found. */
  def get(key:Key, default:Value = memory.defaultValue[Value]()) -> Value;

  /** Remove all entries from the map. */
  def clear();

  /** Return true if the map contains the specified key. */
  def contains(key:Key) -> bool;

  /** Return an iterator over all keys in the map. */
  def keys:Iterator[Key] { get; }

  /** Return an iterator over all values in the map. */
  def values:Iterator[Value] { get; }

  /** Add the specified key/value pair to the map. */
  def add(entry:(Key, Value));
  def add(entries:(Key, Value)...);

  /** Add to the map all of the key value pairs in 'entries'. */
  def addAll(entries:Iterable[(Key, Value)]);

  /** Remove an entry from the collection by key. Returns 'true' if the item was in the
      collection and was removed, 'false' if the item wasn't in the collection to begin with. */
  def remove(key:Key) -> bool;
  def remove(keys:Key...);

  /** Remove all of the entries having a key contained in 'keys'. */
  def removeAll(keys:Iterable[Key]);

  /** Produce a filtered version of this map. */
//  def filter[ReturnType](f:fn((Key, Value)) -> ReturnType) {
//    for item in self {
//      f(item);
//    }
//  }

// equals
// update
// pop
// setdefault
}
