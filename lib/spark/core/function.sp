/** Interface that defines a value of function type. */
interface Function[ReturnType, ParamTypes...] {
  def (params:...ParamTypes) -> ReturnType;
}
