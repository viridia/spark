import spark.text.encodings.UTF_8;

private {
  // POSIX mandates these numbers
//  let FILEDESC_STDIN:int = 0;
//  let FILEDESC_STDOUT:int = 1;
//  let FILEDESC_STDERR:int = 2;

//  var _cin:TextReader?;
  var _cout:TextWriter?;
  var _cerr:TextWriter?;
}

/** The input TextReader for stdin. */
/*def cin:TextReader {
  get {
    return lazyEval(_cin, StreamTextReader(FileStream(FILEDESC_STDIN), Codecs.UTF_8));
  }
}*/

/** The TextWriter for stdout. */
def cout:TextWriter {
  get {
    if _cout == null {
      _cout = StreamTextWriter(FileStream(1), UTF_8);
    }
    // TODO: Use flow analysis to make this cast unnecessary
    _cout as TextWriter
  }
}

/** The TextWriter for stderr. */
def cerr:TextWriter {
  get {
    if _cerr == null {
      _cerr = StreamTextWriter(FileStream(2), UTF_8);
    }
    // TODO: Use flow analysis to make this cast unnecessary
    _cerr as TextWriter
  }
}
