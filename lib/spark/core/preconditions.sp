/** Ensure that the given expression is true, otherwise throw an IllegalArgumentException. */
def checkArgument(expr:bool) {
  if not expr {
//    failArgument(Debug.stringify(expr));
  }
}

/** Ensure that the given expression is true, otherwise throw an IllegalArgumentException. */
def checkState(expr:bool) {
  if not expr {
//    failArgument(Debug.stringify(expr));
  }
}

/** Ensure that the given expression is true, otherwise throw an IndexOutOfRangeException. */
def checkIndex(expr:bool) {
  if not expr {
//    failIndex(Debug.stringify(expr));
  }
}

def checkIndex(index:int, begin:int, end:int) {
  if index < begin or index >= end {
//    failIndex(Debug.stringify(expr));
  }
}
