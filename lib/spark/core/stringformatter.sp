import spark.collections.Array;

/** StringFormatter accepts a format string containing substitution fields, a list of arguments
    to be substituted into those fields. For example::

      let formatter = StringFormatter();
      console.write(formatter("Hello, {0}!", "World"));

    Substitution fields are delimited by braces; To output a normal brace character use a double
    brace '{{'. Within the braces are an optional field index and an optional format specifier.

    The field index indicates which argument in the argument array to use as the value of the field.
    If not present, the field index used will be the index of the previous field, plus 1.

    The format specifier, if present, must begin with a colon. All of the characters after the colon
    (up to the closing brace) will be passed to the 'format' method of the argument value. How the
    format string is interpreted is specific to the type of the argument, but typically a decimal
    number is used to indicate the field width, that is the minimum number of characters that the
    output field should occupy.

    All of the following are valid substitution fields::

      {10}
      {3:12}
      {:12}
 */
class StringFormatter {
  private enum State {
    TEXT,             // Within a literal text run.
    FIELD_START,      // At the start of a replacement field
    FIELD_INDEX,      // Within the field
    FORMAT_SPEC,      // Within a field conversion specifier
  }

  /** Format a string using the given arguments.
      @param format The format string.
      @param args The values to be substituted into the string.
   */
  def (format:String, args:Any...) -> String {
    return vformat(format, args);
  }

  /** Format a string using the given arguments, which are passed in as an array.
      @param format The format string.
      @param args The array of values to be substituted into the string.
   */
  def vformat(format:String, args:Array[Any]) -> String {
    var out = StringBuilder();
    var formatSpec = StringBuilder();
    var state = State.TEXT;
    var nextFieldIndex = 0;
    var fieldIndex = 0;
    for ch in format {
      switch state {
        TEXT => {
          if ch == '{' {
            fieldIndex = nextFieldIndex;
            formatSpec.clear();
            state = State.FIELD_START;
          } else {
            out.append(ch);
          }
        }

        FIELD_START => {
          if ch == '{' {
            // Double-brace escape.
            out.append('{');
            state = State.TEXT;
          } else if ch == '}' {
            out.append(formatArg(args, "", fieldIndex));
            nextFieldIndex = fieldIndex + 1;
            state = State.TEXT;
          } else if ch == ':' {
            state = State.FORMAT_SPEC;
          } else if ch >= '0' and ch <= '9' {
            fieldIndex = int(ch - '0');
          } else {
            throw ArgumentError("Invalid field index");
          }
        }

        FIELD_INDEX => {
          if ch == '}' {
            out.append(formatArg(args, "", fieldIndex));
            nextFieldIndex = fieldIndex + 1;
            state = State.TEXT;
          } else if ch == ':' {
            state = State.FORMAT_SPEC;
          } else if ch >= '0' and ch <= '9' {
            fieldIndex = (fieldIndex * 10) + int(ch - '0');
          } else {
            throw ArgumentError("Invalid field index");
          }
        }

        FORMAT_SPEC => {
          if ch == '}' {
            out.append(formatArg(args, formatSpec.getValue(), fieldIndex));
            nextFieldIndex = fieldIndex + 1;
            state = State.TEXT;
          } else {
            formatSpec.append(ch);
          }
        }
      }
    }

    if state != State.TEXT {
      throw ArgumentError("Unterminated format field");
    }

    return out.getValue();
  }

  /** Convert a single argument to a string using the specified format spec. Can be overridden
      by subclasses to customize the way values are formatted.
   */
  private def formatArg(args:Array[Any], formatSpec:String, index:int) -> String {
    if index < 0 or index >= args.size {
      throw IndexError("Invalid field index");
    }
    return formatField(args(index), formatSpec);
  }

  /** Convert a single argument to a string using the specified format spec. Can be overridden
      by subclasses to customize the way values are formatted.
   */
  protected def formatField(arg:Any, formatSpec:String) -> String {
    return String.valueOf(arg, formatSpec);
  }
}
