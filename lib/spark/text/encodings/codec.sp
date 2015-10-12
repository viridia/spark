import spark.collections.Array;
import spark.collections.Slice;

/** Interface used to encode and decode characters in a particular text format. */
interface Codec {

  /** Action to take upon encountering an error. */
  enum ErrorAction {
    HALT,     //* Stop the operation and return an error code.
    SKIP,     //* Skip over erroneous characters.
    REPLACE,  //* Replace bad characters with the encoder's replacement character.
  }

  /** Indicates the reason why an encode or decode operation halted at a given point. */
  enum CodecState {
    OK,               //* The codec encountered the end of the input or output buffer,
                      //* no errors were detected.
    INCOMPLETE,       //* The input byte stream had only the beginning of a multi-byte char.
    MALFORMED_INPUT,  //* The input byte stream had an invalid sequence of bytes.
    INVALID_CHAR,     //* Attempt to encode an invalid character.
  }

  /** Used to represent the result of an encode or decode operation. */
  struct Result {
    var dstCount:int;           //* Number of elements placed in destination array.
    var srcCount:int;           //* Number of elements consumed from source array.
    var state:CodecState;       //* Reason why the operation halted.

    def new(dstCount:int, srcCount:int, state:CodecState = OK) {
      self.srcCount = srcCount;
      self.dstCount = dstCount;
      self.state = state;
    }
  }

  /** Name of this codec. */
  def name:String { get; }

  /** Calculate the length, in bytes, of the character array when encoded.
      @param src The characters to be encoded.
      @param errAction The action to take upon encountering an error.
      @returns A Result object containing the number of characters read, the number of bytes
        that would have been written, and an error code.
    */
  def encodedLength(src:const Slice[char], errAction:ErrorAction = HALT) -> Result;

  /** Calculate the length, in characters, of the byte array when decoded.
      @param src The bytes to be decoded.
      @param errAction The action to take upon encountering an error.
      @returns A Result object containing the number of bytes read, the number of characters
        that would have been written, and an error code.
    */
  def decodedLength(src:const Slice[u8], errAction:ErrorAction = HALT) -> Result;

  /** Encode the characters in 'src'.
      @param dst The destination byte array.
      @param src The characters to be encoded.
      @param errAction The action to take upon encountering an error.
      @returns A 'Result' object indicating the number of characters read, and the
        number bytes written to 'dst'.
    */
  def encode(dst:Slice[u8], src:const Slice[char], errAction:ErrorAction = HALT) -> Result;

  /** Decode the characters in 'src'.
      @param dst The destination character array.
      @param src The bytes to be decoded.
      @param errAction The action to take upon encountering an error.
      @returns A 'Result' object, indicating the number of bytes read, and the
        number of characters written to 'dst'.
    */
  def decode(dst:Slice[char], src:const Slice[u8], errAction:ErrorAction = HALT) -> Result;
}
