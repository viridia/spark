/** Return the smaller of the two input values, where "smaller" is defined by the less-than
    operator. */
def min[T](a0:T, a1:T) -> T where T < T {
  if a0 < a1 {
    return a0;
  }
  return a1;
}

/** Return the smallest of the input values, where "smallest" is defined in terms of the
    less-than operator. */
def min[T](a0:T, a1:T, a2:T) -> T where T < T {
  return min(a0, min(a1, a2));
}

/** Return the smallest of the input values, where "smallest" is defined in terms of the
    less-than operator. */
def min[T](a0:T, a1:T, a2:T, a3:T) -> T where T < T {
  return min(a0, min(a1, min(a2, a3)));
}

/** Return the larger of the input values, where "larger" is defined in terms of the
    less-than operator. */
def max[T](a0:T, a1:T) -> T where T < T {
  if a0 < a1 {
    return a1;
  }
  return a0;
}

/** Return the largest of the input values, where "largest" is defined in terms of the
    less-than operator. */
def max[T](a0:T, a1:T, a2:T) -> T where T < T {
  return max(a0, max(a1, a2));
}

/** Return the largest of the input values, where "largest" is defined in terms of the
    less-than operator. */
def max[T](a0:T, a1:T, a2:T, a3:T) -> T where T < T {
  return max(a0, max(a1, max(a2, a3)));
}

/** Clamp the input value between the lower and upper bound. */
def clamp[T](value:T, minVal:T, maxVal:T) -> T where T < T {
  if value < minVal {
    return minVal;
  }
  if maxVal < value {
    return maxVal;
  }
  return value;
}
