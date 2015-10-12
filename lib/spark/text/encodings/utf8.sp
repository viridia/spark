import spark.collections.Slice;

object UTF_8 : Codec {

  /** Name of this codec. */
  override name:String { get => "UTF8"; }

  override encodedLength(src:const Slice[char], errAction:ErrorAction = HALT) -> Result {
    var state = CodecState.OK;
    var length = 0;
    var index = 0;
    while index < src.size {
      let c = src(index++);
      switch c {
        0 .. 0x7f => { length += 1 }
        0x80 .. 0x7ff => { length += 2 }
        0x800 .. 0xffff => { length += 3 }
        0x10000 .. 0x10ffff => { length += 4; }
        else => {
          if errAction == ErrorAction.REPLACE {
            length += 1
          } else if errAction == ErrorAction.HALT {
            state = CodecState.INVALID_CHAR;
            break
          } // Else ignore
        }
      }
    }

    return Result(length, index, state);
  }

  override decodedLength(src:const Slice[u8], errAction:ErrorAction = HALT) -> Result {
    var state = CodecState.OK;
    var length = 0;
    var index = 0;
    while index < src.size {
      let c = src(index++);
      switch c {
        0 .. 0x7f => { length += 1; }
        0xc2 .. 0xdf => { index += 1; length += 1; }
        0xe0 .. 0xef => { index += 2; length += 1; }
        0xf0 .. 0xf4 => { index += 3; length += 1; }
        else => {
          if errAction == ErrorAction.REPLACE {
            length += 1
          } else if errAction == ErrorAction.HALT {
            state = CodecState.INVALID_CHAR;
            break
          } // Else ignore
        }
      }
    }
    return Result(length, index, state);
  }

  override encode(dst:Slice[u8], src:const Slice[char], errAction:ErrorAction = HALT) -> Result {
    var state = CodecState.OK;
    var srcIndex = 0;
    var dstIndex = 0;
    while srcIndex < src.size and dstIndex < dst.size {
      let c = src(srcIndex++);
      switch c {
        0 .. 0x7f => {
          dst(dstIndex++) = u8(c);
        }
        0x80 .. 0x7ff => {
          if dstIndex + 2 > dst.size { break; }
          dst(dstIndex++) = u8(c >> 6) | 0xc0;
          dst(dstIndex++) = u8(c) & 0x3f | 0x80;
        }
        0x800 .. 0xffff => {
          if dstIndex + 3 > dst.size { break; }
          dst(dstIndex++) = u8(c >> 12) | 0xe0;
          dst(dstIndex++) = u8(c >>  6) & 0x3f | 0x80;
          dst(dstIndex++) = u8(c) & 0x3f | 0x80;
        }
        0x10000 .. 0x10ffff => {
          if dstIndex + 4 > dst.size { break; }
          dst(dstIndex++) = u8(c >> 18) | 0xf0;
          dst(dstIndex++) = u8(c >> 12) & 0x3f | 0x80;
          dst(dstIndex++) = u8(c >>  6) & 0x3f | 0x80;
          dst(dstIndex++) = u8(c) & 0x3f | 0x80;
        }
        else => {
          if errAction == ErrorAction.REPLACE {
            if dstIndex >= dst.size { break; }
            dst(dstIndex++) = u8('?');
          } else if errAction == ErrorAction.HALT {
            state = CodecState.INVALID_CHAR;
            break;
          } // Else ignore
        }
      }
    }

    return Result(dstIndex, srcIndex, CodecState.OK);
  }

  override decode(dst:Slice[char], src:const Slice[u8], errAction:ErrorAction = HALT) -> Result {
    var state = CodecState.OK;
    var srcIndex = 0;
    var dstIndex = 0;
    while srcIndex < src.size and dstIndex < dst.size {
      var charVal:char = 0;
      let b = src(srcIndex);
      switch b {
        0 .. 0x7f => {
          charVal = b;
          srcIndex += 1;
        }
        0xc2 .. 0xdf => {
          if srcIndex + 2 > src.size {
            state = CodecState.INCOMPLETE;
            break
          }
          charVal = (char(b & 0x3f) << 6)
                  | (char(src(srcIndex + 1)) & 0x7f);
          srcIndex += 2;
        }
        0xe0 .. 0xef => {
          if srcIndex + 3 > src.size {
            state = CodecState.INCOMPLETE;
            break
          }
          charVal = (char(b & 0x1f) << 12)
                  | (char(src(srcIndex + 1) & 0x7f) << 6)
                  | (char(src(srcIndex + 2) & 0x7f));
          srcIndex += 3;
        }
        0xf0 .. 0xf4 => {
          if srcIndex + 4 > src.size {
            state = CodecState.INCOMPLETE;
            break
          }
          charVal = (char(b & 0x0f) << 18)
                  | (char(src(srcIndex + 1) & 0x7f) << 12)
                  | (char(src(srcIndex + 2) & 0x7f) << 6)
                  | (char(src(srcIndex + 3) & 0x7f));
          srcIndex += 4;
        }
        else => {
          if errAction == ErrorAction.HALT {
            state = CodecState.MALFORMED_INPUT;
            break
          }
          if errAction == ErrorAction.REPLACE {
            charVal = '?';
          }
          srcIndex++;
        }
      }
      dst(dstIndex++) = charVal;
    }
    return Result(dstIndex, srcIndex, state);
  }
}
