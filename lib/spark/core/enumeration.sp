import spark.collections.Array;

/** Base type for all enumerations. */
__intrinsic__ abstract enum Enum {
  let ordinal:int;
  let name:String;

  def coerce -> String { self.name; }

  override format(formatSpec:String) -> String {
    return name.format(formatSpec);
  }

  /** All of the values defined by this enumeration. */
  static def values[T](class:T):const Array[T] { get; }
}

/** Base type for enumerations that have an interger value. */
__intrinsic__ abstract enum IntEnum : Enum {
  let value:int;

  def new();
  def new(value:int);

  /** Given an integer value, return the enumeration instance with that value. Throws an
      ArgumentError if the value cannot be found.
      @param value The integer value to search for.
      @returns The enumeration instance with the given value.
      @throws ArgumentError if the value could not be found.
  */
  static def forValue[EnumType](class:EnumType, value:int) -> EnumType {
    for v in EnumType.values {
      if v.value == value {
        return v;
      }
    }
    // TODO: Replace with Strformat
    throw ArgumentError("No enumeration constant with value N");
  }
}

/** An enumeration type representing bit masks. Enumeration values may be manually assigned
    an arbitrary mask holding multiple bits; Automatically-assigned entries will use the next
    available unused bit. */
__intrinsic__ abstract enum FlagsEnum : Enum {
  let mask:int;

  /** Construct a `FlagsEnum` value with a bit mask containing the next unused bit. */
  def new();

  /** Construct a `FlagsEnum` value with an explicitly-assigned bit mask. */
  def new(mask:int);

/*  static def |[T <: FlagsEnum](left:T, right:T) -> T {
    return FlagsEnum(left.value | right.value);
  }*/
}
