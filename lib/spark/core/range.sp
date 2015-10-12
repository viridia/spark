import spark.collections.Iterable;
import spark.collections.Iterator;

// TODO: Consider creating an interface Interval[T] which Range implements.
// Interval would work with any type that has < defined.

/** A class representing a closed interval of integers. */
/*immutable*/ final struct Range : Iterable[int] {
  /** The lower bound of the range. */
  let lower:int;

  /** The upper bound of the range. */
  let upper:int;

  /** Construct a new range. */
  def new(lower:int, upper:int) {
    .lower = lower;
    .upper = upper;
  }

  /** The length of the interval. */
  def size:int { get => upper - lower + 1; }

  /** Returns true if `value` is included within the range. */
  def contains(value:int) -> bool { value >= lower and value <= upper; }

  /** Returns an iterator over the set of integers in the range. */
  override iterate(self:const) -> Iterator[int] { RangeIterator(self); }

  class RangeIterator : Iterator[int] {
    var value:int;
    let limit:int;

    def new(range:Range) {
      .value = range.lower;
      .limit = range.upper;
    }

    override next() -> int or void {
      if value <= limit {
        return value++;
      }
    }
  }
}
