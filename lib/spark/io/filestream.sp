import spark.core.memory.Address;
import spark.collections.Array;
import spark.collections.Slice;
import spark.text.encodings.Codec;

/** A stream which provides basic read/write operations on file handle.
    This class does no buffering, and does not handle character encodings.
  */
final class FileStream : IOStream {
  friend spark.io;

  enum AccessFlags : FlagsEnum {
    READ,
    WRITE,
  }

  private {
    var fileDesc:i32;

    @Native("FileStream_open")
    static def _open(path:String, access:int) -> i32;

    @Native("FileStream_read_byte")
    static def _readByte(fileDesc:i32) -> i32;

    @Native("FileStream_read_bytes")
    static def _readBytes(fileDesc:i32, buffer:Address[u8], count:i64) -> i64;

    @Native("FileStream_write_byte")
    static def _writeByte(fileDesc:i32, value:u8);

    @Native("FileStream_write_bytes")
    static def _writeBytes(fileDesc:i32, buffer:Address[u8], count:i64);

    @Native("FileStream_seek")
    static def _seek(fileDesc:i32, from:int, offset:i64) -> i64;

    @Native("FileStream_canRead") static def _canRead(fileDesc:i32) -> i32;
    @Native("FileStream_canWrite") static def _canWrite(fileDesc:i32) -> i32;
    @Native("FileStream_canSeek") static def _canSeek(fileDesc:i32) -> i32;
    @Native("FileStream_isTerminal") static def _isTerminal(fileDesc:i32) -> i32;

    @Native("FileStream_position") static def _position(fileDesc:i32) -> i64;
    @Native("FileStream_length") static def _length(fileDesc:i32) -> i64;
    @Native("FileStream_flush") static def _flush(fileDesc:i32);
    @Native("FileStream_close") static def _close(fileDesc:i32);
  }

  /** Construct a FileStream from an existing file descriptor. */
  private def new(fileDesc:i32) {
    self.fileDesc = fileDesc;
  }

  /** Constructs an FileStream for the file at 'path'.
      @throws FileNotFoundError if the specified file could not be found.
      @throws UnauthorizedAccessError if the user does not have permission to access this file.
      @throws IOError for other i/o errors.
  */
  def new(path:String, access:int = AccessFlags.READ.mask) {
    self.fileDesc = _open(path, access);
  }

  /** Read a single byte from the stream.
      @returns The byte read.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override read -> i32 {
    return _readByte(fileDesc);
  }

  /** Read a bytes from the stream into a byte array.
      @param buffer The byte array where the bytes are to be placed.
      @param start The starting position in the buffer.
      @param count How many bytes to read.
      @returns The actual number of bytes read.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override read(buffer:Array[u8], start:int = 0, count:int = int.maxVal) -> int {
    preconditions.checkArgument(count >= 0);
    start = math.min(start, buffer.size);
    count = math.min(count, buffer.size - start);
    if count > 0 {
      // TODO: Add GC.suspend calls here (using 'with' statement), and pin the buffer.
      return _readBytes(fileDesc, buffer.address(start), count);
    }
    return 0;
  }

  /** Read the entire contents of the fileDesc, starting from the current read position,
      and return it as a byte array.
      @returns A byte array containing the contents of the file.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
    */
  override readAll -> Array[u8] {
    let length = size - position;
    let result = Array[u8](length);
    read(result, 0, length);
    return result;
  }

  /** Write a single byte to the stream.
      @param value The byte to write.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override write(value:u8) {
    _writeByte(fileDesc, value);
  }

  /** Write the contents of a byte array to the stream.
      @param buffer The byte array containing the bytes to be written.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override write(buffer:Slice[u8]) {
    if buffer.size > 0 {
      _writeBytes(fileDesc, buffer.address(0), buffer.size);
    }
  }

  /** Change the current read/write position of the stream.
      @param from The reference point (start, end or current).
      @param offset The offset from the reference point.
      @returns the current stream position.
      @throws NotSupportedError if the stream does not support seeking (pipe or socket).
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override seek(from:SeekFrom, offset:i64) -> i64 {
    return _seek(fileDesc, from.value, offset);
  }

  /** True if this stream supports reading.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override canRead:bool { get { return _canRead(fileDesc) != 0; } }

  /** True if this stream supports writing.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override canWrite:bool { get { return _canWrite(fileDesc) != 0; } }

  /** True if this stream supports seek operations.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override canSeek:bool { get { return _canSeek(fileDesc) != 0; } }

  /** Returns the current position in the stream.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override position:i64 { get { return _position(fileDesc); } }

  /** Return the length of the stream, or 0 if the length could not be determined.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override size:i64 { get { return _length(fileDesc); } }

  /** Flush any pending writes.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override flush {
    _flush(fileDesc);
  }

  /** Close the stream.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  override close {
    _close(fileDesc);
  }

  /** True if this stream is connected to a terminal device.
      @throws ArgumentError if the file descriptor was invalid.
      @throws IOError for other i/o errors.
   */
  def isTerminal:bool { get { return _isTerminal(fileDesc) != 0; } }

  // ScopedObject

  def exit() { close(); }
}
