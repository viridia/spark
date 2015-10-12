import spark.collections.Array;
import spark.collections.Slice;
import spark.text.encodings;

/** A mutable string-like object intended for constructing strings. */
final class StringBuilder {
  private {
    var value:Array[char];
    var sz:int;

		/** Expand the size of the buffer, leaving room for new elements at the end. */
    def grow(amount:int) {
      let newSize = .sz + amount;
      if value.size < newSize {
        let newValue = Array[char](newSize + newSize / 2 + 16);
        newValue.copyFrom(0, value(0, .sz));
        value = newValue;
      }
      self.sz = newSize;
    }

		/** Expand the size of the buffer, leaving room for new elements at position 'insertPos'. */
    def grow(amount:int, insertPos:int) {
      let remaining = .sz - insertPos;
      // TODO: This could be more efficient. We're copying some chars twice.
      grow(amount);
      value.moveElements(insertPos + amount, insertPos, remaining);
    }
  }

  /** Create an empty StringBuilder. */
  def new {
    value = Array(0);
    self.sz = 0;
  }

  /** Create an empty StringBuilder.
      @param reservedSize The reserved size of the buffer, in characters.
   */
  def new(reservedSize:int) {
    value = Array[char](reservedSize);
    self.sz = 0;
  }

  /** Create a StringBuilder from an existing String. */
  def new(str:String, start:int = 0, count:int = int.maxVal) {
    value = str.toChars(start, count);
    sz = value.size;
  }

  /** Create a StringBuilder from a character array. */
  def new(chars:Slice[char]) {
    value = Array[char](chars.size);
    value.copyFrom(0, chars);
    sz = chars.size;
  }

  /** The current character buffer. */
  def chars:Slice[char] { get { return value(0, sz); } }

  /** The number of characters in the buffer.
      This field can be assigned to, but only to make the string shorter. Attempting to make
      the string longer will have no effect.
   */
  def size:int {
    get { return .sz; }
    set {
      self.sz = math.clamp(value, 0, self.sz);
    }
  }

  /** True if the string is zero size. */
  def empty:bool { get => sz == 0; }

	/** Array element access. */
  def (index:int):char {
    get {
      // Array already checks index >= 0
      preconditions.checkIndex(index < self.sz);
      return .value(index);
    }
    set {
      // Array already checks index >= 0
      preconditions.checkIndex(index < self.sz);
      .value(index) = value;
    }
  }

  /** Append a character to the buffer.
      @param c The character to append.
   */
  def append(c:char) -> StringBuilder {
    let n = .sz;
    grow(1);
    .value(n) = c;
    self
  }

  /** Append a String to the buffer.
      @param s The string to append.
   */
  def append(s:String) -> StringBuilder {
    let insertPos = .sz;
    let bytes = s.toBytes();
    let decodeResult = encodings.UTF_8.decodedLength(bytes);
    if decodeResult.state != encodings.Codec.CodecState.OK {
      throw encodings.MalformedInputError();
    }
    grow(decodeResult.dstCount);
    encodings.UTF_8.decode(value(insertPos, .sz), bytes);
    self
  }

  /** Append a sequence of characters to the buffer.
      @param s The characters to insert.
   */
  def append(s:Slice[char]) -> StringBuilder {
    insert(size, s);
    self
  }

  // TODO: append(Collection) and append(Iterable)

  /** Insert a character at the specified insertion point.
      @param insertPos The index where the new character should be inserted.
      @param c The character to insert.
      @throws IndexError if the insertion point is less than zero or greater than the
          number of characters in the buffer.
   */
  def insert(insertPos:int, c:char) -> StringBuilder {
    preconditions.checkIndex(insertPos >= 0);
    preconditions.checkIndex(insertPos <= self.sz);
    grow(1, insertPos);
    value(insertPos) = c;
    self
  }

  /** Insert a string at the specified insertion point.
      @param insertPos The index where the characters should be inserted.
      @param s The string containing the character to insert.
      @throws IndexError if the insertion point is less than zero or greater than the
          number of characters in the buffer.
   */
  def insert(insertPos:int, s:String) -> StringBuilder {
    preconditions.checkIndex(insertPos >= 0);
    preconditions.checkIndex(insertPos <= self.sz);
    let bytes = s.toBytes();
    let decodeResult = encodings.UTF_8.decodedLength(bytes);
    if decodeResult.state != encodings.Codec.CodecState.OK {
      throw encodings.MalformedInputError();
    }
    grow(decodeResult.dstCount, insertPos);
    encodings.UTF_8.decode(value(insertPos, insertPos + decodeResult.dstCount), bytes);
    self
  }

  /** Insert a sequence of characters at the specified insertion point.
      @param insertPos The index where the characters should be inserted.
      @param s A `Slice` containing the characters to insert.
      @throws IndexError if the insertion point is less than zero or greater than the
          number of characters in the buffer.
   */
  def insert(insertPos:int, s:Slice[char]) -> StringBuilder {
    preconditions.checkIndex(insertPos >= 0);
    preconditions.checkIndex(insertPos <= self.sz);
    grow(s.size, insertPos);
    value.copyFrom(insertPos, s);
    self
  }

  // TODO: insert(Collection) and insert(Iterable)

  /** Remove the character at position 'n'.
      @param n The index of the character to remove.
   */
  def remove(n:int) -> StringBuilder {
    remove(n, 1);
    self
  }

  /** Remove `count` characters starting at position `start`. If there are fewer than `count`
      elements following `start`, then all characters following `start` will be removed.
      @param start The starting index of the range of characters to be removed.
      @param count The number of characters to remove.
      @throws IndexError If `count` or `start` are less than zero.
    */
  def remove(start:int, count:int) -> StringBuilder {
    preconditions.checkIndex(count >= 0);
    preconditions.checkIndex(start >= 0);
    start = math.min(start, sz);
    count = math.min(count, sz - start);
    value.moveElements(start, start + count, sz - start - count);
    sz -= count;
    self
  }

  /** Remove all characters from the buffer. */
  def clear() -> StringBuilder {
    sz = 0;
    self
  }

  /** Return the character buffer as a String. */
  def getValue -> String {
    return String(value(0, self.sz));
  }
}
