import spark.collections.Array;
import spark.collections.Iterator;
import spark.text.encodings.Codec;

/** Interface for writing text to a file or output device. */
interface TextReader {
  /** Character constant that signals no more characters can be read. */
  static let EOF = char(-1);

  /** The current text codec for this writer. */
  def decoder:Codec { get; set; }

  /** Read a single character from the stream. Return `EOF` if no more characters can be read.
      Line breaks are not altered by this method - whatever line-break characters
      are present in the original input stream are returned unchanged.
      @returns The next character read from the input.
      @throws IOError If there was an i/o error.
   */ 
  def readCh -> char;

  /** Read up to `count` characters into the array `buffer`, starting at position `start`.
      returns the actual number of characters read, or 0 if there were no characters remaining.
      Line breaks are not altered by this method - whatever line-break characters
      are present in the original input stream are written to the output buffer unchanged.
      @param buffer The buffer to read characters into.
      @param start The index within the buffer to put the first character.
      @param count The maximum number of characters to read.
      @returns The number of characters actually read.
      @throws IOError If there was an i/o error.
   */
  def read(buffer: Array[char], start:int = 0, count:int = int.maxVal) -> int;

  /** Read characters until the next line-end delimiter. The delimiter is not included
      in the result string. Returns an empty string if there are no more characters to be read.
      Note that a line break may consist of a linefeed, carriage-return, or CR LF pair.
      Returns: The next line in the stream, or `void` if we've reached the end.
      @throws IOError If there was an i/o error.
    */
  def readLn -> String or void;

  /** Read all remaining characters in the stream. Returns an empty string if there are
      no more characters to be read.
      Line breaks are not altered by this method - whatever line-break characters
      are present in the original input stream are returned unchanged.
      @throws IOError If there was an i/o error.
    */
  def readAll -> String;

  /** Returns an iterator over the lines of the stream. This repeatedly calls readLn()
      until there are no more lines. */
  def lines -> Iterator[String];

  /** Skip over `count` characters. If there are fewer than `count` characters remaining in
      the stream, then skip to the end. */
  def skip(count: i64);

  /** True if we've reached the end of available input. */
  def atEnd: bool { get; }

  /** Close the reader and release any resources held by the reader instance. */
  def close();
}
