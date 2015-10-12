import spark.core.memory;

/** An unordered collection of unique items. */
interface Set[Element] : Collection[Element] {

  /** Remove all entries from the set. */
  def clear();

  /** Return true if the set contains the specified element. */
  def contains(element:Element) -> bool;

  /** Add the specified element pair to the set. */
  def add(element:Element);

  /** Add a collection of elements to the set. */
  def addAll(elements:Element...);
  def addAll(elements:Iterable[Element]);

  /** Remove an entry from the set. Returns `true` if the item was in the set and was removed,
      `false` if the item wasn't in the set to begin with. */
  def remove(element:Element) -> bool;

  /** Remove from this set all items in a collection. */
  def removeAll(elements:Element...);
  def removeAll(elements:Iterable[Element]);

  /** Produce a filtered version of this map. */
//  def filter[ReturnType](f:fn(Element) -> ReturnType) {
//    for item in self {
//      f(item);
//    }
//  }

// equals
// update
// pop
// setdefault
}
