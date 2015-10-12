import spark.core.memory.addressOf;
import spark.core.memory;
import spark.core.math.min;
import spark.gc.trace;

/** Array is a sequence of elements whose length is fixed at the time of creation. */
final class Array[Element] : Sequence[Element] {
  friend spark.collections;
  friend spark.core;
  friend spark.io;

  private {
    let sz:int;
    var data:memory.FlexArray[Element];

    /** Array allocator function */
    static def alloc(size:int) -> Array {
      memory.FlexArray[Element].alloc(size);
    }

    /** Custom trace method for garbage collection to handle variable-length object. */
    __tracemethod__ def __trace(action:trace.TraceAction) {
      for i = 0; i < sz; ++i {
        action.trace(.data(i));
      }
    }
  }

  static var EMPTY = Array(0);

  /** Construct an array of a given length */
  static def new(size:int, default:Element = memory.defaultValue[Element]()) -> Array {
    preconditions.checkArgument(size >= 0);
    if (size == 0) {
      return of(); // Compiler optimizes empty arrays as static singletons
    }

    let array:Array = alloc(size);
    memory.fill(addressOf(array.data, 0), size, default);
    array.sz = size;
    array
  }

  /** Static factory function which takes advantage of the built-in behavior of variadic parameters
      to build an Array. */
  static def of(elements:Element...) -> Array { elements }

  /** Static factory function which builds a new Array from a Slice. */
  static def copyOf(src:const Slice[Element]) -> Array {
    let size = src.size;
    if size == 0 {
      return EMPTY;
    }

    let result:Array = Array(size);
    memory.copy(result.data.address(0), src.address(0), size);
    result
  }

  /** Static factory function which builds a new Array from a Collection. */
  static def copyOf(src:const Collection[Element]) -> Array {
    let size = src.size;
    if size == 0 {
      return of(); // Compiler optimizes empty arrays as static singletons
    }

    let result:Array = Array(size);
    var index = 0;
    for el in src {
      result.data(index++) = el;
    }

    result
  }

  /** True if the array is zero size. */
  override empty:bool { get => sz == 0; }

  /** Number of elements in the array. */
  override size:int { get => .sz; }

  /** Array element access. */
  override (index:int):Element {
    get {
      preconditions.checkIndex(index >= 0 and index < sz);
      .data(index)
    }
    set {
      preconditions.checkArgument(index >= 0 and index < sz);
      .data(index) = value;
    }
  }

  /** Obtain a reference to a sub-range of this array. This creates a `Slice` object
      which refers to the elements of the original array. Modifications to the elements
      of the slice will affect the parent array, and vice versa.
      @param start The starting index of the sub-range. This is clamped to the bounds of the array.
      @param end The ending index of the sub-range. This is clamped to the bounds of the array.
      @return The array slice.
   */
  def (start:int, end:int):Slice[Element] {
    get {
      start = math.clamp(start, 0, sz);
      end = math.clamp(end, start, sz);
      Slice(self, .data.offset(self, start), end - start)
    }
  }

  /** Convert this array to an array slice representing all the elements in the array. */
  def coerce -> Slice[Element] {
    Slice(self, .data.offset(self, 0), sz)
  }

  // TODO: Should be override
  def coerce -> String {
    let sb = StringBuilder();
    sb.append("[");
    for item in self {
      if sb.size > 1 {
        sb.append(", ");
      }
// TODO: Doesn't work because we don't know if Element can be converted to a string.
// String(x) needs to always work.
//      sb.append(String.valueOf(item));
    }
    sb.append("]");
    sb.getValue()
  }

  /** Move elements around in the array. */
  def moveElements(dstOffset:int, srcOffset:int, count:int) {
    preconditions.checkIndex(count >= 0);
    preconditions.checkIndex(srcOffset >= 0 and srcOffset + count <= sz);
    preconditions.checkIndex(dstOffset >= 0 and dstOffset + count <= sz);
    if count > 0 {
      memory.move(.data.address(dstOffset), .data.address(srcOffset), count);
    }
  }

  /** Copy elements from the specified slice into this array.
      @param dstOffset The destination index in this array.
      @param src The slice to copy from.
  */
  def copyFrom(dstOffset:int, src:const Slice[Element]) {
    preconditions.checkIndex(dstOffset >= 0 and dstOffset <= sz);
    preconditions.checkIndex(dstOffset + src.size <= sz);
    memory.copy(.data.address(dstOffset), src.address(0), src.size);
  }

  /** Copy elements from the source array into this array.
      @param dstOffset The destination index in this array.
      @param src The array to copy from.
  */
  def copyFrom(dstOffset:int, src:const Array) {
    preconditions.checkIndex(dstOffset >= 0 and dstOffset <= sz);
    preconditions.checkIndex(dstOffset + src.size <= sz);
    memory.copy(.data.address(dstOffset), src.data.address(0), src.size);
  }

  /** Copy elements from the specified collection into this array.
      @param dstOffset The destination index in this array.
      @param src The collection to copy from.
  */
  def copyFrom(dstOffset:int, src:const Collection[Element]) {
    preconditions.checkIndex(dstOffset >= 0 and dstOffset <= sz);
    preconditions.checkIndex(dstOffset + src.size <= sz);
    var index = dstOffset;
    for item in src {
      .data(index++) = item;
    }
  }

  /** Iterate over the elements of the array. */
  override iterate(self:const) -> Iterator[Element] { return ArrayIterator(self) }

  /** Return an array containing all of the elements for which the predicate function `pred`
      returns true. */
  def filter(self:const, pred:fn(Element) -> bool) -> Array {
    let buf = Array(sz);
    var out = 0;
    for i = 0; i < sz; ++i {
      if pred(data(i)) {
        buf(out++) = data(i);
      }
    }
    return Array.copyOf(buf(0, out));
  }

  /** Return an array containing all of the elements whose type is a subtype of the given type. */
  def filterType[T](self:const) -> Array[T] {
    let buf = Array[T](sz);
    var out = 0;
    for i = 0; i < sz; ++i {
      if data(i) is T {
        buf(out++) = data(i) as T;
      }
    }
    return Array[T].copyOf(buf(0, out));
  }

/*  override all(pred:fn(Element) -> bool) -> bool {
    for item in self {
      if not pred(item) { return false; }
    }
    return true;
  }

  override any(pred:fn(Element) -> bool) -> bool {
    for item in self {
      if pred(item) { return true; }
    }
    return false;
  }

  override count(pred:fn(Element) -> bool) -> int {
    var count = 0;
    for item in self {
      if pred(item) { count++; }
    }
    count
  } */

  /** Returns the memory address of the Nth element of this slice. */
  __unsafe__ private def address(index:int) -> memory.Address[Element] {
    preconditions.checkIndex(index >= 0 and index <= sz);
    .data.address(index);
  }

  private final class ArrayIterator : Iterator[Element] {
    private {
      var array:const Array;
      var index:int;
      var end:int;
    }

    def new(array:const Array) {
      .array = array;
      .index = 0;
      .end = array.sz;
    }

    def new(array:const Array, start:int, end:int) {
      .array = array;
      .index = start;
      .end = end;
    }

    override next() -> Element or void {
      if index < end {
        array.data(index++)
      }
    }

    def length:int { get => end - index; }
    def size:int { get => end - index; }
  }
}
