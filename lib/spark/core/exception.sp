class Throwable {
}

class Exception : Throwable {
  let message:String;
  let cause:Exception?;

  /** Default constructor. */
  def new() {
    .message = "";
    .cause = null;
  }

  /** Message constructor. */
  def new(message:String) {
    .message = message;
//    .cause = null;
  }

  /** Constructor that accepts both a message and a cause. */
  def new(message:String, cause:Exception) {
    .message = message;
    .cause = cause;
  }

/*  override String() -> String {
    if message.size > 0 {
      return String.concat(__typeName, ": ", _message);
    } else {
      return __typeName;
    }
  } */
}

class ArgumentError : Exception {}

class AssertionError : Exception {}

class IndexError : Exception {}

class OverflowError : Exception {}

class NotImplementedError : Exception {}

class NotSupportedError : Exception {}

class TypecastError : Exception {
  // Utility function used by generated code.
  static def fail() { throw TypecastError() }
}

class UnsupportedOperationError : Exception {}
