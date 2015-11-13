/** TypeLiteral represents a type name as an expression. This is used when passing a type name
    to a function. The compiler automatically coerces type names into TypeLiteral objects.

    Example:
      // A function that accepts a TypeLiteral argument.
      def makelist[T](type: TypeLiteral[T]) -> List[T] {
        // Note that we don't actually need to reference the 'type' parameter value.
        return ArrayList.of();
      }

      var floatList = makelist(float);
      var stringList = makelist(String);
  */
class TypeLiteral[T] {}
