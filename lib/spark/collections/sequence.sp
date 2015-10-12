/** Sequence represents a collection of elements which are numbered sequentially, starting from
    zero up to one less than the size of the collection. */
interface Sequence[Element] : Collection[Element] {
  /** Element access. */
  def (index:int):Element { get; }
}
