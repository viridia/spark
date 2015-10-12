/** Collection represents the concept of a container of elements which has a known size and
    which is iterable. */
interface Collection[Element] : spark.collections.Iterable[Element] {
  // Add typedef for element type
  // typalias Element = Element

  /** Return the number of items in the collection. */
  def size:int { get; }

  /** Return true if this collection has no items. */
  def empty:bool { get; }

  // TODO: Should we move these to Iterable?

  /** Return true if the predicate function returns true for all elements in the collection. */
//  def all(pred:fn(Element) -> bool) -> bool;

  /** Return true if the predicate function returns true for any elements in the collection. */
//  final def any(pred:fn(Element) -> bool) -> bool;

  /** Count the number of items in the collection for which the predicate function returns true. */
//  final def count(pred:fn(Element) -> bool) -> int;

  /** Call a function for each element in the collection. */
//  def each[ReturnType](f:fn(Element) -> ReturnType) {
//    for item in self {
//      f(item);
//    }
//  }

  /** Transform this collection through a mapping function. */
  // TODO: rename to transform?
//  def map[CollectionType <: Collection[Element], ReturnType](
//      self:CollectionType, f:fn(Element) -> ReturnType);

// clone
// chain?
}
