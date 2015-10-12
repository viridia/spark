/** Memory utility functions. These functions are inherently unsafe and will cause memory
    corruption if misused. */

/** Represents a machine address. */
__unsafe__ __intrinsic__ final class Address[T] {
  undef new();

  /** Dereference the address (treat as start of array). */
  __intrinsic__ def (index:int):T { get; set; }

  /** Return the address of the Nth element. */
  __intrinsic__ def element(offset:int) -> Address;

  /** Comparison operators. */
  __intrinsic__ static def ==(a0:Address, a1:Address) -> bool;
  __intrinsic__ static def !=(a0:Address, a1:Address) -> bool;

  /** Pointer arithmetic. */
  __intrinsic__ static def +(a0:Address, a1:int) -> Address;

  // static def calc(base:Object, offset:int) -> Address;
}

/** Address of a native function pointer. */
__unsafe__ __intrinsic__ final class FunctionAddress[ReturnType, ParamTypes...] {
  undef new();

  /** Call the function. */
  __intrinsic__ def (params:...ParamTypes) -> ReturnType;

  /** Comparison operators. */
  __intrinsic__ static def ==(a0:FunctionAddress, a1:FunctionAddress) -> bool;
  __intrinsic__ static def !=(a0:FunctionAddress, a1:FunctionAddress) -> bool;
}

/** Address of a native function pointer which supports garbage collection. */
__unsafe__ __intrinsic__ final class FunctionAddressGC[ReturnType, ParamTypes...] {
  undef new();

  /** Call the function. */
  __intrinsic__ def (params:...ParamTypes) -> ReturnType;

  /** Comparison operators. */
  __intrinsic__ static def ==(a0:FunctionAddressGC, a1:FunctionAddressGC) -> bool;
  __intrinsic__ static def !=(a0:FunctionAddressGC, a1:FunctionAddressGC) -> bool;
}

/** A native c-style array which occupies the memory space immediately following an
    object. The size of the array is fixed at allocation, and the object and the flexible array
    elements are allocated in a single memory allocation. FlexibleArray fields can only be used in
    reference types, and must be the last declared field in the enclosing type. The 'alloc' method
    calculates the space needed for the object and a specified number of array elements.

    FlexibleArrays are typically used to implement other collection types, such as Array.
    No bounds checking is done on flexible arrays - that is the responsibility of the collection
    classes built on top of FlexibleArray.
 */
__unsafe__ __intrinsic__ final class FlexArray[T] {
  // Allocate an object of type K which has a flex array of type T as its last member.
  static def alloc[K](size:int) -> K;
  undef new();
  def (index:int):T { get; set; }
  def address(index:int) -> Address[T];
  def offset[K](base:K, index:int) -> int;
}

/** A 'raw' c-style arrays with no bounds checking and a fixed length. */
__unsafe__ __intrinsic__ final class NativeArray[T, Size:int] {
  undef new();
  def (index:int):T { get; set; }
  // def address(index:int) -> Address[T];
  // def offset(base:Object, index:int) -> int
}

/** ManagedReference is used to handle internal pointers to objects. It can also be used to
    reference variables on the stack. */
__unsafe__ __intrinsic__ final struct ManagedReference[T] {
  let container:Object;
  let offset:uint;

  def new(container:Object, offset:uint) {
    .container = container;
    .offset = offset;
  }
}

/** Given a type, return the size, in bytes, of a field of that type. For reference types,
    the field size is the size of a pointer. For non-reference types, the field size is the
    size of an instance of that type. */
__intrinsic__ def fieldSize[T](type:TypeLiteral[T]) -> int;

/** Take the address of a variable and return the result as a native
    pointer. 'Value' can be a complex expression that resolves to an l-value.
    Note that if P is a reference type, the result will be the address of a reference of value,
    not the value itself.
*/
//__unsafe__ __intrinsic__ def addressOf[P](value:P) -> Address[P];
__unsafe__ __intrinsic__ def addressOf[P](value:FlexArray[P], index:int) -> Address[P];
//__unsafe__ __intrinsic__ def addressOf[P <: Object](value:FlexArray[P], index:int) -> Address[Address[P]];
//__unsafe__ __intrinsic__ def addressOf[P](value:FlexArray[P], index:uint) -> Address[P];

/** Return the 'default value' for this data type. */
// TODO: Create a specialization of this that allows it to be overridden.
__intrinsic__ def defaultValue[T]() -> T;

/** Calculate an internal pointer into an object. */
__unsafe__ __intrinsic__ def internalPtr[T](baseAddress:const Object, offset:int) -> Address[T];

/** Reinterpret an address. */
__unsafe__ __intrinsic__ def reinterpret[Src, Dst](addr:Address[Src]) -> Address[Dst];

/** Return the numerical difference between two pointers, specifically the number of values of
    type `T` that can be placed between the two pointers. */
__unsafe__ __intrinsic__ def ptrDiff[T](low:Address[T], high:Address[T]) -> int;

/** Fills a region of memory locations with a value. Does not check array bounds.
    The behavior is equivalent to the CLib function memset().
    @param dst The starting address of the memory region to fill.
    @param value The value to fill.
    @param length The number of elements to copy.
 */
__unsafe__ __intrinsic__ def fill[T](dst:Address[T], length:int, value:T);

/** Copies a range of elements from one native array to another. Does not check array bounds.
    Does not guarantee correct behavior if the source and destination overlap.

    The behavior is equivalent to the CLib function memcpy().
    @param dst The array containing the destination range.
    @param src The array containing the source range.
    @param length The number of elements to copy.
 */
__unsafe__ __intrinsic__ def copy[T](dst:Address[T], src:Address[T], length:int);
__unsafe__ __intrinsic__ def copy[T](dst:Address[T], src:Address[T], length:uint);

/** Copies a range of elements from one native array to another. Does not check array bounds.
    Does not guarantee correct behavior if the source and destination overlap.

    The behavior is equivalent to the CLib function memcpy().
    @param dstBegin The start of the destination range.
    @param dstEnd The end of the destination range.
    @param src The start of the source range.
 */
__unsafe__ __intrinsic__ def copy[T](dstBegin:Address[T], dstEnd:Address[T], src:Address[T]);

/** Copies a range of elements from one native array to another. Does not check array bounds.
    Handles the case of overlapping ranges.

    The behavior is equivalent to the CLib function memmove().
    @param dst The destination range.
    @param src The source range.
    @param length The number of elements to copy.
 */
__unsafe__ __intrinsic__ def move[T](dst:Address[T], src:Address[T], length:int);
__unsafe__ __intrinsic__ def move[T](dst:Address[T], src:Address[T], length:uint);

/** Copies a range of elements from one native array to another. Does not check array bounds.
    Handles the case of overlapping ranges.

    The behavior is equivalent to the CLib function memcpy().
    @param dstBegin The start of the destination range.
    @param dstEnd The end of the destination range.
    @param src The start of the source range.
 */
__unsafe__ __intrinsic__ def move[T](dstBegin:Address[T], dstEnd:Address[T], src:Address[T]);
