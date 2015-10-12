/** Interface that represents any type. */
interface Any {
  __intrinsic__ static def ==(a0:Any, a1:Any) -> bool;
  __intrinsic__ static def ==(a0:Object, a1:Any) -> bool;
  __intrinsic__ static def ==(a0:Any, a1:Object) -> bool;
  __intrinsic__ static def !=(a0:Any, a1:Any) -> bool;
  __intrinsic__ static def !=(a0:Object, a1:Any) -> bool;
  __intrinsic__ static def !=(a0:Any, a1:Object) -> bool;
}
