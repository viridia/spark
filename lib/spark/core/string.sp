import spark.collections.Array;
import spark.collections.Iterator;
import spark.collections.Slice;
import spark.collections.hashing;
import spark.core.memory.addressOf;
import spark.core.memory;
import spark.text.encodings;

/** Interface for types that know how to convert themselves into strings. */
interface Formattable {
  /** Produce a string representation of this object. The formatSpec contains formatting
      options which are specific to the type being formatted. */
  def format(formatSpec:String="") -> String;
}

/** The built-in string class. Strings are immutable. */
final class String : spark.collections.Iterable[char], Formattable, hashing.Hashable {
  friend spark.io;

  /** Standard formatter for strings. */
  static let FORMATTER = StringFormatter();

  let size:int;
  private {
    var data:memory.FlexArray[u8];

    /** String allocator function */
    static def alloc(size:int) -> String {
      memory.FlexArray[u8].alloc(size);
    }
  }

  /** Construct a string from a sequence of bytes. The sequence is presumed to contain character
      data encoded as UTF-8.
      @param bytes The byte array.
   */
  static def new(bytes:const Slice[u8]) -> String {
    let s = alloc(bytes.size);
    memory.copy(s.data.address(0), bytes.address(0), bytes.size);
    s.size = bytes.size;
    s
  }

  /** Construct a string from an sequence of characters.
      @param chars The character array.
      @throws tart.text.encodings.InvalidCharacterError if an unencodable character was encountered.
   */
  static def new(chars:const Slice[char]) -> String {
    let result = encodings.UTF_8.encodedLength(chars);
    if result.state != encodings.Codec.CodecState.OK {
      throw encodings.InvalidCharacterError();
    }
    let s = alloc(result.dstCount);
    s.size = result.dstCount;
    encodings.UTF_8.encode(Slice(s, s.data.offset(s, 0), s.size), chars);
    return s;
  }

  static def new(format:String, args:Any...) -> String {
    return FORMATTER.vformat(format, args);
  }

/*  static def new(obj:Object) -> String {
  }

  static def new[T](value:T) -> String {
  } */

  private static def new(size:int) -> String {
    let s = alloc(size);
    s.size = size;
    s;
  }

  /** Concatenate two strings. */
  static def concat(s0:String, s1:String) -> String {
    let s = String(s0.size + s1.size);
    memory.copy(s.data.address(0), s0.data.address(0), s0.size);
    memory.copy(s.data.address(s0.size), s1.data.address(0), s1.size);
    s
  }

  /** Concatenate two strings. */
  static def +(s0:String, s1:String) -> String => concat(s0, s1);

  /** Default value for String. */
  static def defaultValue -> String => "";

  /** Return the value of the byte at index `index` in this string. */
  def byteAt(index:int) -> u8 {
    preconditions.checkIndex(index >= 0 and index < size);
    .data(index)
  }

  /** Return the character starting at byte index `index`, and also return the index of the
      following character (or end of the string). */
  def nextChar(index:int) -> (char, int) {
    if index >= .size {
      throw IndexError();
    }
    let b = .data(index);
    // UTF-8 decoding logic is inlined here to avoid overhead.
    switch b {
      0 .. 0x7f => {
        return char(b), index + 1;
      }
      0xc2 .. 0xdf => {
        if index + 2 > .size {
          throw IndexError();
        }
        let charVal = (char(b & 0x3f) << 6)
                    | (char(.data(index + 1)) & 0x7f);
        return charVal, index + 2;
      }
      0xe0 .. 0xef => {
        if index + 3 > .size {
          throw IndexError();
        }
        let charVal = (char(b & 0x1f) << 12)
                    | (char(.data(index + 1) & 0x7f) << 6)
                    | (char(.data(index + 2) & 0x7f));
        return charVal, index + 2;
      }
      0xf0 .. 0xf4 => {
        if index + 4 > .size {
          throw IndexError();
        }
        let charVal = (char(b & 0x0f) << 18)
                    | (char(.data(index + 1) & 0x7f) << 12)
                    | (char(.data(index + 2) & 0x7f) << 6)
                    | (char(.data(index + 3) & 0x7f));
        return charVal, index + 2;
      }
      else => {
        throw encodings.MalformedInputError();
      }
    }
  }

  /** Return a new string containing a range of bytes from this string.
      @param start The starting index of the substring range. This is clamped to the bounds of the
          string.
      @param end The ending index of the substring range. This is clamped to the bounds of the
          string.
      @returns A new string consisting of the specified portion of the original string.
   */
  def substr(start:int, end:int) -> String {
    start = math.clamp(start, 0, size);
    end = math.clamp(end, start, size);
    let result = String(end - start);
    memory.copy(result.data.address(0), self.data.address(start), result.size);
    result;
  }

  /** Return the character starting at index `index` in this string, and also the index of
      the following character. */
  def charAt(index:int) -> (char, int) {
    preconditions.checkIndex(index >= 0 and index < size);
    (.data(index), index + 1)
  }

  /** Return a Slice representing the sequence of bytes that make up this string. */
  def toBytes() -> const Slice[u8] { Slice(self, .data.offset(self, 0), .size); }

  /** Convert this string to a character array.
      @param start The starting byte index of the substring to convert.
      @param count The length, in bytes, of the substring to convert.
      @returns A character array containing the characters of this string.
   */
  def toChars(start:int = 0, count:int = int.maxVal) -> Array[char] {
    preconditions.checkIndex(start >= 0 and count >= 0);
    start = math.min(start, .size);
    count = math.min(count, .size - start);
    let bytes = Slice[u8](self, .data.offset(self, start), count);
    let decodeResult = encodings.UTF_8.decodedLength(bytes);
    if decodeResult.state != encodings.Codec.CodecState.OK {
      throw encodings.MalformedInputError();
    }
    let result = Array[char](decodeResult.dstCount);
    encodings.UTF_8.decode(result, bytes);
    return result;
  }

  def startsWith(prefix:String) -> bool {
    if (prefix.size <= size) {
      for i = 0; i < prefix.size; ++i {
        if .data(i) != prefix.data(i) {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  def endsWith(suffix:String) -> bool {
    if (suffix.size <= size) {
      let offset = size - suffix.size;
      for i = 0; i < suffix.size; ++i {
        if .data(offset + i) != suffix.data(i) {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  static def ==(s0:String, s1:String) -> bool {
    if s0.size != s1.size {
      return false;
    }
    for i = 0; i < s0.size; ++i {
      if s0.data(i) != s1.data(i) {
        return false;
      }
    }
    return true;
  }

  def lines:Iterator[String] { get => LinesIterator(self); }

  override format(formatSpec:String="") -> String {
    if formatSpec.size == 0 { return self }
    if formatSpec == "r" {
      return "\"" + self + "\"";
    }
    self
  }

  static def valueOf(value:Any, formatSpec:String="") -> String {
    if value == null {
      return "null";
    }
    match value {
      bool => _formatBool(value, formatSpec);
      i8  => _formatI32(value, formatSpec);
      u8  => _formatU32(value, formatSpec);
      i16 => _formatI32(value, formatSpec);
      u16 => _formatU32(value, formatSpec);
      i32 => _formatI32(value, formatSpec);
      u32 => _formatU32(value, formatSpec);
      i64 => _formatI64(value, formatSpec);
      u64 => _formatU64(value, formatSpec);
      f32 => _formatF32(value, formatSpec);
      f64 => _formatF64(value, formatSpec);
      Formattable => value.format(formatSpec);
      else => "<unknown>";
    }
  }

  static def valueOf(value:bool, formatSpec:String="") -> String {
    if value { "true" } else { "false" }
  }
  static def valueOf(value:i64, formatSpec:String="") -> String => _formatI64(value, formatSpec);
  static def valueOf(value:u64, formatSpec:String="") -> String => _formatU64(value, formatSpec);
  static def valueOf(value:f32, formatSpec:String="") -> String => _formatF32(value, formatSpec);
  static def valueOf(value:f64, formatSpec:String="") -> String => _formatF64(value, formatSpec);

  override hashCode() -> u32 => hashing.hash(.data.address(0), .data.address(.size));

  private {
    @Native("String_formatI32")
    static def _formatI32(value:i32, formatSpec:String="") -> String;

    @Native("String_formatI64")
    static def _formatI64(value:i64, formatSpec:String="") -> String;

    @Native("String_formatU32")
    static def _formatU32(value:u32, formatSpec:String="") -> String;

    @Native("String_formatU64")
    static def _formatU64(value:u64, formatSpec:String="") -> String;

    @Native("String_formatF32")
    static def _formatF32(value:f32, formatSpec:String="") -> String;

    @Native("String_formatF64")
    static def _formatF64(value:f64, formatSpec:String="") -> String;

    static def _formatBool(value:bool, formatSpec:String="") -> String {
      if value { "true" } else { "false" }
    }
  }

  /** Iterate over the characters in this string. */
  override iterate(self:const) -> Iterator[char] {
    return StringIterator(self);
  }

  /** String iterator class. */
  private final class StringIterator : Iterator[char] {
    private {
      let str:String;
      var index:int;
    }

    def new(str:String) {
      .str = str;
      .index = 0;
    }

    override next -> char or void {
      if index < .str.size {
        var ch:char;
        ch, index = str.nextChar(index);
        return ch;
      }
    }
  }

  /** String iterator class. */
  private final class LinesIterator : Iterator[String] {
    private {
      let str:String;
      var index:int;
    }

    def new(str:String) {
      .str = str;
      .index = 0;
    }

    override next -> String or void {
      if index < str.size {
        var nextIndex = index;
        while nextIndex < str.size
            and str.byteAt(nextIndex) != '\n'
            and str.byteAt(nextIndex) != '\r' {
          ++nextIndex;
        }
        let result = str.substr(index, nextIndex);
        index = nextIndex;
        if index < str.size and str.byteAt(index) == '\r' {
          ++index;
        }
        if index < str.size and str.byteAt(index) == '\n' {
          ++index;
        }
        result
      }
    }
  }
}
