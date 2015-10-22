// ============================================================================
// semgraph/type.h: Semantic graph nodes for types.
// ============================================================================

#ifndef SPARK_SEMGRAPH_TYPE_H
#define SPARK_SEMGRAPH_TYPE_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

namespace spark {
namespace semgraph {
using collections::ArrayRef;
using collections::StringRef;
class Defn;

/** Base class for all types. */
class Type {
public:
  enum class Kind {
    INVALID = 0,
    NO_RETURN,      // Sentinel type used to indicate expression never returns

    // Nominal types
    VOID,
    NIL,            // Null pointer type
    BOOLEAN, INTEGER, FLOAT,    // Primitives
    CLASS, STRUCT, INTERFACE, EXTENSION, ENUM,  // Composites
    TYPE_VAR,       // Reference to a template parameter
    ALIAS,          // An alias for another type

    // Derived types
    UNION,          // Disjoint types
    TUPLE,          // Tuple of types
    FUNCTION,       // Function type
    MODIFIED,       // Const or mutable type
    INSTANTIATED,   // Instantiation of a type
    VALUE_PARAM,    // A type parameter bound to an immutable value

    // Types used internally during compilation
//     TYPE_EXPR,  // A type represented as an expression
//     AMBIGUOUS,  // Ambiguous type
//     PHI,        // Represents multiple types produced by different code paths.

    // Type names - used during parsing
//     SPECIALIZE,
//     TYPESET,

    // Backend - used during code generation
//     VALUE_REF,
//     ADDRESS,
  };

  Type(Kind kind) : _kind(kind) {}

  /** What kind of type this is. */
  Kind kind() const { return _kind; }

  static Type ERROR;
private:
  const Kind _kind;
};

typedef ArrayRef<Type*> TypeArray;

class PrimitiveType : public Type {
public:
  PrimitiveType(Kind kind, const StringRef& name, Defn* defn)
    : Type(kind), _name(name), _defn(defn) {}

  /** Name of this primitive type. */
  const StringRef& name() const { return _name; }

  /** Type definition for this primitive type. */
  Defn* defn() { return _defn; }
  const Defn* defn() const { return _defn; }

private:
  StringRef _name;
  Defn* _defn;

//   # Used internally by compiler to store methods for builtin types
//   memberScope: defn.SymbolScope = 10;
};

// struct VoidType(PrimitiveType) = TypeKind.VOID {}
// struct NullType(PrimitiveType) = TypeKind.NULL {}
// struct BooleanType(PrimitiveType) = TypeKind.BOOLEAN {}

/** An integer type. */
class IntegerType : public PrimitiveType {
public:
  IntegerType(Defn* defn, StringRef name, int32_t bits, bool isUnsigned, bool isPositive)
    : PrimitiveType(Kind::INTEGER, name, defn)
  {}

  /** Number of bits in this integer type. */
  int32_t bits() const { return _bits; }

  /** If true, this is an unsigned integer type. */
  bool isUnsigned() const { return _unsigned; }

  /** Used in type inference: if true, it means that this is the type of an integer constant
      whose signed/unsigned state is not known. For example, the number 127 could be either
      a 8-bit signed number or an 8-bit unsigned number. We represent it as a 7-bit positive
      number until the exact type can be inferred. */
  bool isPositive() const { return _positive; }

private:
  int32_t _bits;
  bool _unsigned;
  bool _positive;
};

/** A floating-point type. */
class FloatType : public PrimitiveType {
public:
  FloatType(Defn* defn, StringRef name, int32_t bits) : PrimitiveType(Kind::FLOAT, name, defn) {}

  /** Number of bits in this floating-point type. */
  int32_t bits() const { return _bits; }

private:
  int32_t _bits;
};

// class TypeVar(Type) = TypeKind.TYPE_VAR {
//   param: defn.TypeParameter = 1;
//   index: i32 = 2; # Disambiguate between type params of the same name.
// }

/** Disjoint type. */
class UnionType : public Type {
public:
  UnionType(const TypeArray& members) : Type(Kind::UNION), _members(members) {}
  const TypeArray& members() const { return _members; }

private:
  const TypeArray _members;
};

/** Tuple type. */
class TupleType : public Type {
public:
  TupleType(const TypeArray& members) : Type(Kind::TUPLE), _members(members) {}
  const TypeArray& members() const { return _members; }

private:
  const TypeArray _members;

//   # Used internally by compiler to lookup members by name. Constructed lazily.
//   memberScope: defn.SymbolScope = 2;
};

/** Type of a function. */
class FunctionType : public Type {
public:
  FunctionType(Type* returnType, const TypeArray& paramTypes, bool isConstSelf)
    : Type(Kind::FUNCTION)
    , _returnType(returnType)
    , _paramTypes(paramTypes)
    , _constSelf(isConstSelf)
  {}

  /** The return type. */
  Type* returnType() const { return _returnType; }

  /** The type of this function's parameters. */
  const TypeArray& paramTypes() const { return _paramTypes; }

  /** True if this function cannot modify its context. */
  bool isConstSelf() const { return _constSelf; }
private:
  Type* _returnType;
  TypeArray _paramTypes;
  bool _constSelf;
};

/** A type that represents a unique constant value. */
// class ValueParamType : public Type {
//   value : expr.Expr = 1;
// }

// class ModifiedType : public Type {
//   enum Modifiers {
//     CONST = 1;
//     TRANSITIVE_CONST = 2;
//     VARIADIC = 3;
//     REF = 4;
//   }
//
//   base : Type = 1;              # Type being modifiers
//   modifiers: set[Modifiers] = 2;# Set of modifiers applied to the type
//
// #  'const' : bool = 2;
// #  transitiveConst : bool = 3;
// #  variadic : bool = 4;
// #  ref : bool = 5;
// };

class Composite : public Type {
public:
  Composite(Kind kind) : Type(kind), _defn(NULL), _superType(NULL) {}

  /** Definition for this type. */
  Defn* defn() const { return _defn; }
  void setDefn(Defn* defn) { _defn = defn; }

  /** Primary base type. */
  Type* superType() const { return _superType; }
  void setSuperType(Type* s) { _superType = s; }

  /** Explicitly supported interfaces. */
  const TypeArray& interfaces() const { return _interfaces; }
  void setInterfaces(const TypeArray& ifaces) { _interfaces = ifaces; }

private:
  Defn* _defn;
  //bases : list[ast.Node] = 2;   # List of base types
  Type* _superType;
  TypeArray _interfaces;

//   # Table of instance methods for this type, including base types. The entries in the table are
//   # pairs of integers containing (typeindex, methodindex). Typeindex is 0 for methods defined in
//   # this class, 1 for methods defined in the superclass, and 2+n for the Nth interface. The method
//   # index corresponds to the methodIndex property of the method.
//   methodTable: list[i32] = 5;
//
//   # Instance methods after template expansion
//   methods : list[defn.Function] = 7;

//   struct InterfaceMethodTable {
//     interface: Type = 1;        # The interface being inherited
//     # Mapping from interface method slots to concrete methods.
//     methodTable: list[i32] = 2;
//     # Concrete methods after template expansion
//     methods : list[defn.Function] = 3;
//   }
//   # This table includes all interfaces inherited recursively.
//   interfaceMethods: list[InterfaceMethodTable] = 8;
//
//   # Table of known, cached conversions
//   conversions : map[Type, defn.Function] = 9;
//
//   # Map of methods by method index - used during template expansion phase.
//   methodsByIndex : map[i32, defn.Function] = 10;
};

}}

#endif
