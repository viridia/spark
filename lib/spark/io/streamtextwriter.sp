import spark.collections.Array;
import spark.collections.Slice;
import spark.text.encodings.Codec;
import spark.text.encodings.UTF_8;
import spark.text.encodings.InvalidCharacterError;

/** Class that writes text to an output stream. */
final class StreamTextWriter : TextWriter {
  private {
    var stream:IOStream;
    var codec:Codec;
    var buffer:Array[u8];
    var bufferPos:int;
    var highWaterMark:int;
  }

  def new(stream:IOStream, codec:Codec = UTF_8) {
    self.stream = stream;
    self.codec = codec;
    self.buffer = Array(512);
    self.bufferPos = 0;
    self.highWaterMark = buffer.size * 3 / 4;
  }

  override encoder:Codec {
    get { return codec; }
    set { codec = value; }
  }

  private def writeImpl(chars:Array[char]) {
    var start = 0;
    var count = chars.size;
    while count > 0 {
      let encodeResult = codec.encode(buffer(bufferPos, buffer.size), chars(start, count));
      preconditions.checkState(encodeResult.srcCount >= 0); // Did we make progress?
      start += encodeResult.srcCount;
      count -= encodeResult.srcCount;
      bufferPos += encodeResult.dstCount;

      if encodeResult.state != Codec.CodecState.OK {
        throw InvalidCharacterError();
      }

      // Empty the buffer if it's more than 3/4 full.
      if bufferPos >= highWaterMark {
        stream.write(buffer(0, bufferPos));
        bufferPos = 0;
      }
    }
  }

  private def writeLineBreak() {
    if bufferPos == buffer.size {
      stream.write(buffer(0, bufferPos));
      bufferPos = 0;
    }
    buffer(bufferPos++) = 0xa; // u8('\n');
    stream.write(buffer(0, bufferPos));
    bufferPos = 0;
  }

  override write(chars:Slice[char]) -> TextWriter {
    if chars.size == 0 {
      return self;
    }
    var count = chars.size;
    var start = 0;
    while count > 0 {
      let encodeResult = codec.encode(buffer(bufferPos, buffer.size), chars(start, start + count));
      preconditions.checkState(encodeResult.srcCount >= 0); // Did we make progress?
      start += encodeResult.srcCount;
      count -= encodeResult.srcCount;
      bufferPos += encodeResult.dstCount;

      if encodeResult.state != Codec.CodecState.OK {
        throw InvalidCharacterError();
      }

      // Empty the buffer if it's more than 3/4 full.
      if bufferPos >= highWaterMark {
        stream.write(buffer(0, bufferPos));
        bufferPos = 0;
      }
    }
    return self;
  }

  override write(text:String) -> TextWriter {
    if .codec == UTF_8 {
      if text.size == 0 {
        return self;
      }

      // Copy the string data into the buffer.
      var count = text.size;
      var start = 0;
      while count > 0 {
        let actual = math.min(count, buffer.size - bufferPos);
        memory.copy(buffer.data.address(bufferPos), text.data.address(start), actual);
        preconditions.checkState(actual >= 0); // Did we make progress?
        start += actual;
        count -= actual;
        bufferPos += actual;

        // Empty the buffer if it's more than 3/4 full.
        if bufferPos >= highWaterMark {
          stream.write(buffer(0, bufferPos));
          bufferPos = 0;
        }
      }
    } else {
      writeImpl(text.toChars());
    }
    return self;
  }

  override write(text:String...) -> TextWriter {
    for s in text {
      write(s);
    }
    return self;
  }

  override writeLn(text:String) -> TextWriter {
    write(text);
    writeLineBreak();
    return self;
  }

  override writeLn(text:String...) -> TextWriter {
    for s in text {
      write(s);
    }
    writeLineBreak();
    return self;
  }

  override writeFmt(format:String, values:Object...) -> TextWriter {
//    let sb = StringFormatter(format, values).toBuilder();
//    write(sb.chars, 0, sb.size);
    return self;
  }

  override writeLnFmt(format:String, values:Object...) -> TextWriter {
//    let sb = StringFormatter(format, values).toBuilder();
//    write(sb.chars, 0, sb.size);
//    writeLineBreak();
    return self;
  }

  override close() {
    flush();
    stream.close();
  }

  override flush() {
    if bufferPos > 0 {
      stream.write(buffer(0, bufferPos));
      stream.flush();
      bufferPos = 0;
    }
  }
}
