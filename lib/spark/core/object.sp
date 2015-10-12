/** Supertype of all reference types. */
class Object : string.Formattable {
  __intrinsic__ static def ==(a0:Object, a1:Object) -> bool;
  __intrinsic__ static def !=(a0:Object, a1:Object) -> bool;

  override format(formatSpec:String) -> String {
    return _format(self, formatSpec);
  }

  @Native("Object_format")
  static def _format(obj:Object, formatSpec:String) -> String;
}
