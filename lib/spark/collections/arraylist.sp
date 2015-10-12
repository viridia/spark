/** Array-backed list type. */
final class ArrayList[Element] : List[Element] where Element == Element {
  private {
    var data:Array[Element];
    var sz:int;

    def grow(amount:int) {
      let nsize = self.sz + amount;
      if data.size < nsize {
        let ndata = Array[Element](nsize + nsize / 2 + 16);
        ndata.copyFrom(0, data(0, .sz));
        .data = ndata;
      }

      .sz = nsize;
    }
  }

  /** Construct a new empty ArrayList.
      @param capacity This optional parameter, if present, indicates how much initial space to
          reserve.
   */
  def new(; capacity:int = 0) {
    data = Array[Element](capacity);
    .sz = 0;
  }

  /** Construct a new ArrayList from a variable number of input arguments.
      @param data The list of values to store in the ArrayList.
      @param capacity This optional parameter, if present, indicates how much initial space to
          reserve.
   */
  def new(data:Element...; capacity:int = 0) {
    capacity = math.max(capacity, data.size);
    .data = Array[Element](capacity);
    .data.copyFrom(0, data);
    .sz = data.size;
  }

  override add(e:Element) {
    let index = .sz;
    grow(1);
    data(index) = e;
  }

  override addAll(src:const Slice[Element]) {
    var index = .sz;
    grow(src.size);
    data.copyFrom(index, src);
  }

  override addAll(src:const Collection[Element]) {
    var index = .sz;
    grow(src.size);
    data.copyFrom(index, src);
  }

  override insert(index:int, e:Element) {
    preconditions.checkIndex(index >= 0 and index <= .sz);
    grow(1);
    data.moveElements(index + 1, index, .sz - index - 1);
    data(index) = e;
  }

  override insertAll(index:int, src:const Slice[Element]) {
    preconditions.checkIndex(index >= 0);
    preconditions.checkIndex(index <= .sz);
    let count = src.size;
    grow(count);
    data.moveElements(index + count, index, .sz - index - count);
    data.copyFrom(index, src);
  }

  override insertAll(index:int, src:const Collection[Element]) {
    preconditions.checkIndex(index >= 0);
    preconditions.checkIndex(index <= .sz);
    let count = src.size;
    preconditions.checkState(count >= 0);
    grow(count);
    data.moveElements(index + count, index, .sz - index - count);
    data.copyFrom(index, src);
  }

  override replace(index:int, count:int, src:const Slice[Element]) {
    preconditions.checkIndex(index >= 0 and index <= .sz);
    preconditions.checkIndex(count >= 0);
    count = math.min(count, .sz - index);
    let prevSize = .sz;
    if src.size > count {
      grow(src.size - count);
    } else {
      .sz += src.size - count;
    }
    data.moveElements(index + src.size, index + count, prevSize - index - count);
    data.copyFrom(index, src);
  }

  override replace(index:int, count:int, src:const Collection[Element]) {
    preconditions.checkIndex(index >= 0 and index <= .sz);
    preconditions.checkIndex(count >= 0);
    count = math.min(count, .sz - index);
    let prevSize = .sz;
    if src.size > count {
      grow(src.size - count);
    } else {
      .sz += src.size - count;
    }
    data.moveElements(index + src.size, index + count, prevSize - index - count);
    data.copyFrom(index, src);
  }

  override remove(index:int) {
    preconditions.checkIndex(index >= 0);
    preconditions.checkIndex(index < .sz);
    data.moveElements(index, index + 1, .sz - index - 1);
    --.sz;
  }

  override clear() {
    .sz = 0;
  }

  override (index:int):Element {
    get {
      preconditions.checkIndex(index >= 0);
      preconditions.checkIndex(index < .sz);
      return data(index);
    }

    set {
      preconditions.checkIndex(index >= 0);
      preconditions.checkIndex(index < .sz);
      data(index) = value;
    }
  }

  override size:int {
    get { return sz; }
    set {
      // We can't make the collection larger by setting the size.
      preconditions.checkIndex(value >= 0 and value <= size);
      .sz = value;
    }
  }

  override empty:bool {
    get { return .sz == 0; }
  }

  override contains(self:const, e:Element) -> bool {
    for i = 0; i < .sz; ++i {
			if data(i) == e {
			  return true;
			}
    }
    return false;
  }

  /** The amount of space currently reserved. */
  def capacity:int {
    get { return data.size; }
  }

  /** Construct a new ArrayList from a collection.
      @param src The collection containing the values to store in the ArrayList.
      @param capacity This optional parameter, if present, indicates how much initial space to
          reserve.
   */
  static def copyOf(src:Collection[Element]; capacity:int = 0) -> ArrayList {
    let size = src.size;
    capacity = math.max(capacity, size);
    let result = ArrayList(capacity = capacity);
    result.sz = size;
    result.data.copyFrom(0, src);
    return result;
  }

  /** Return an array containing the elements in this ArrayList. */
  def toArray() -> Array[Element] => Array.copyOf(data(0, sz));

  override iterate(self:const) -> Iterator[Element] {
    return ArrayListIterator(self);
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

  /** Iterator class for ArrayList. */
  private final class ArrayListIterator : Iterator[Element] {
    private let list:ArrayList;
    private var index:int;

    def new(list:ArrayList) {
      .list = list;
      .index = 0;
    }

    override next -> Element or void {
      if (.index < .list.size) {
        return .list.data(.index++);
      }
    }
  }
}
