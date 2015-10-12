/** Used to traverse a sequence of elements that can be accessed in order. */
interface Iterator[T] {
  /** Return the next element in the sequence, or 'void' to indicate the end of the sequence. */
  def next -> T or void;
}

/** Represents a sequence or collection which can be accessed in order. */
interface Iterable[T] {
  /** Return an iterator object for this collection. */
  def iterate(self:const) -> Iterator[T];
}

// Used by the compiler to get an iterator from an iterable.
private def iterate[T](iter:Iterator[T]) -> Iterator[T] => iter;
// TODO: Code generated for this is non-optimal, we should inline and then get rid of the
// interface cast.
private def iterate[T](iter:const Iterable[T]) -> Iterator[T] => iter.iterate();
