import spark.collections.Slice;
import spark.text.encodings.Codec;

/** Interface for writing text to a file or output device. */
interface TextWriter {
  /** The current text codec for this writer. */
  def encoder:Codec { get; set; }

  /** Write an array of characters to an output stream.
      @param chars The array of characters to write.
      @returns The writer, for chaining.
      @throws IOError If there was an i/o error.
      @throws InvalidCharacterError If there was a character encoding error.
   */
  def write(chars:Slice[char]) -> TextWriter;

  /** Write a string of text to the output stream.
      @param text The text to write.
      @returns The writer, for chaining.
      @throws IOError If there was an i/o error.
      @throws InvalidCharacterError If there was a character encoding error.
   */
  def write(text:String) -> TextWriter;

  /** Concatenate strings and write them to the output stream.
      @param text The list of text strings to write.
      @returns The writer, for chaining.
      @throws IOError If there was an i/o error.
      @throws InvalidCharacterError If there was a character encoding error.
   */
  def write(text:String...) -> TextWriter;

  /** Write a string of text to the output stream followed by a line break.
      @param text The text to write.
      @returns The writer, for chaining.
      @throws IOError If there was an i/o error.
      @throws InvalidCharacterError If there was a character encoding error.
   */
  def writeLn(text:String) -> TextWriter;

  /** Concatenate strings and write them to the output stream, followed by a line break.
      @param text The list of text strings to write.
      @returns The writer, for chaining.
      @throws IOError If there was an i/o error.
      @throws InvalidCharacterError If there was a character encoding error.
   */
  def writeLn(text:String...) -> TextWriter;

  /** Write values to the output stream using a format string.
      @param format The format string.
      @param values The field values for the format string.
      @returns The writer, for chaining.
      @throws IOError If there was an i/o error.
      @throws InvalidCharacterError If there was a character encoding error.
   */
  def writeFmt(format:String, values:Object...) -> TextWriter;

  /** Write values to the output stream using a format string, followed by a line break.
      @param format The format string.
      @param values The field values for the format string.
      @returns The writer, for chaining.
      @throws IOError If there was an i/o error.
      @throws InvalidCharacterError If there was a character encoding error.
   */
  def writeLnFmt(format:String, values:Object...) -> TextWriter;

  /** Close the reader and release any resources held by the reader instance. */
  def close();

  /** Flush any pending writes. */
  def flush();

  // For ScopedObject
  def exit() {
    close();
  }
}
