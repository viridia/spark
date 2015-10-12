/** Class used to store a boxed value type (struct or tuple). */
class ValueRef[T] {
  let value:T;

  def new(value:T) {
    .value = value;
  }
}
