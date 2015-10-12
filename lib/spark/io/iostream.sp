import spark.collections.Array;
import spark.collections.Slice;

/** `IOStream` defines a low-level, sequential data source such as a file or network socket.
    The interface defines methods for both reading and writing, which operate on bytes
    and/or arrays of bytes. (For operating on strings or arrays of characters, see
    `TextReader` / `TextWriter`.)

    Individual implementations of this interface may choose whether to support
    only reading, only writing, or both reading and writing.
*/
interface IOStream {
  enum SeekFrom : IntEnum {
    CURRENT = 0,
    START = 1,
    END = 2
  }

  static let EOF = i32(-1);

  /** Change the current read/write position of the stream.
    */
  def seek(from:SeekFrom, offset:i64) -> i64;

  /** True if the stream supports reading. */
  def canRead:bool { get; }

  /** True if the stream supports writing. */
  def canWrite:bool { get; }

  /** True if this stream supports seek operations. */
  def canSeek:bool { get; }

  /** Returns the current position in the stream.
      Returns: The current stream position.
   */
  def position:i64 { get; }

  /** Return the length of the stream, or -1 if indeterminate. */
  def size:i64 { get; }

  /** Read a single byte from the stream. This method will block until a character
      is available, an i/o error occurrs, or the end of the stream is reached.
      @returns `EOF` if the end of the stream has been reached, otherwise
          a number in the range 0..255.
      @throws IOError if there was a problem reading the data.
  */
  def read -> i32;

  /** Read up to `count` bytes from the stream and place them into the array `buffer`,
      starting at position `start`:
      @param buffer Where to put the characters being read.
      @param start Offset at which to start storing characters.
      @param count The maximum number of characters to read.
      @returns The actual number of characters read.
      @throws IOError If an i/o error occurs.
   */
  def read(buffer:Array[u8], start:int = 0, count:int = int.maxVal) -> int;

  /** Read the entire contents of the file, starting from the current read position,
      and return it as a byte array.
      @returns A byte array containing the contents of the file.
      @throws IOError If an i/o error occurs.
    */
  def readAll -> Array[u8];

  /** Write a single byte to the stream. */
  def write(value:u8);

  /** Write a buffer of bytes to the stream. */
  def write(buffer:Slice[u8]);

  /** Flush any pending writes. */
  def flush;

  /** Close the stream. */
  def close;
}
