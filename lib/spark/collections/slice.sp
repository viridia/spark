import spark.collections.Array;
import spark.core.memory;
import spark.core.math.min;
import spark.gc.trace;

/** A `Slice` represents a subrange of an `Array`. Slices are both readable and writeable; write
    operations modify the underlying `Array` object. */
final struct Slice[Element] : Sequence[Element] {
  friend spark.collections;
  friend spark.core;
  friend spark.io;

  private {
    let objref:const? Object;
    let offset:int;
    let sz:int;
  }

/*  def new(objref:const? Array[Element]) {
    .objref = objref;
    .offset = 0;
    .sz = array.sz;
  } */

  def new(objref:const? Object, offset:int, size:int) {
    .objref = objref;
    .offset = offset;
    .sz = size;
  }

  /** Number of elements in the slice. */
  override size:int { get => .sz; }

  /** True if the slice is zero size. */
  override empty:bool { get => sz == 0; }

  /** Array element access. */
  override (index:int):Element {
    get {
      preconditions.checkIndex(index >= 0 and index < sz);
      let addr:memory.Address[Element] = memory.internalPtr(.objref, .offset);
      addr(index);
    }
    set {
      preconditions.checkArgument(index >= 0 and index < sz);
      let addr:memory.Address[Element] = memory.internalPtr(.objref, .offset);
      addr(index) = value;
    }
  }

  /** Get a sub-slice of this slice. Can expand the slice. Will be clamped to the extent
      of the underlying array. */
  def (start:int, end:int):const? Slice {
    get {
      start = math.clamp(start, 0, .sz);
      end = math.clamp(end, start, .sz);
      return Slice(.objref, offset + start * memory.fieldSize(Element), end - start)
    }
  }

  /** Copy elements from the specified slice into the backing array.
      @param dstIndex The destination index.
      @param src The slice to copy from.
  */
  def copyFrom(dstIndex:int, src:Slice) {
    preconditions.checkIndex(dstIndex >= 0 and dstIndex <= sz);
    preconditions.checkIndex(dstIndex + src.sz <= sz);
    memory.copy(.address(dstIndex), src.address(0), src.sz);
  }

  /** Move elements around in the array slice.
      @param dstIndex The offset where the elements will be moved to.
      @param srcIndex The offset where the elements will be moved from.
      @param count The number of elements to move.
   */
  def moveElements(dstIndex:int, srcIndex:int, count:int) {
    preconditions.checkIndex(count >= 0);
    preconditions.checkIndex(srcIndex >= 0 and srcIndex + count <= sz);
    preconditions.checkIndex(dstIndex >= 0 and dstIndex + count <= sz);
    if count > 0 {
      memory.move(.address(dstIndex), .address(srcIndex), count);
    }
  }

  /** Iterate over the elements of the array slice. */
  override iterate(self:const) -> Iterator[Element] { SliceIterator(self) }

  /** Returns the memory address of the Nth element of this slice. */
  __unsafe__ private def address(index:int) -> memory.Address[Element] {
    memory.internalPtr(.objref, .offset + index * memory.fieldSize(Element))
  }

  private final class SliceIterator : Iterator[Element] {
    private {
      var slice:const Slice;
      var index:int;
      var end:int;
    }

    def new(slice:const Slice) {
      .slice = slice;
      .index = 0;
      .end = slice.sz;
    }

    override next() -> Element or void {
      if index < end {
        slice(index++)
      }
    }

    def length:int { get => end - index; }
    def size:int { get => end - index; }
  }
}
