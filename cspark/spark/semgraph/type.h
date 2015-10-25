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

#ifndef SPARK_COLLECTIONS_HASHING_H
  #include "spark/collections/hashing.h"
#endif

#ifndef SPARK_SUPPORT_ARENA_H
  #include "spark/support/arena.h"
#endif

#ifndef SPARK_SEMGRAPH_ENV_H
  #include "spark/semgraph/env.h"
#endif

namespace spark {
namespace semgraph {
using collections::ArrayRef;
using collections::StringRef;
class TypeDefn;

/** Base class for all types. */
class Type {
public:
  enum class Kind {
    INVALID = 0,
    NO_RETURN,      // Sentinel type used to indicate expression never returns

    // Nominal types
    VOID,
    NULLPTR,        // Null pointer type
    BOOLEAN, INTEGER, FLOAT,    // Primitives
    CLASS, STRUCT, INTERFACE, EXTENSION, ENUM,  // Composites
    TYPE_VAR,       // Reference to a template parameter
    ALIAS,          // An alias for another type

    // Derived types
    UNION,          // Disjoint types
    TUPLE,          // Tuple of types
    FUNCTION,       // Function type
    MODIFIED,       // Const or mutable type
    SPECIALIZED,    // Instantiation of a type
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

  /** Return true if this type is an error sentinel. */
  static bool isError(const Type* t) {
    return t == nullptr || t->kind() == Kind::INVALID;
  }

  /** Dynamic casting support. */
  static bool classof(const Type* t) { return true; }

  static Type ERROR;
private:
  const Kind _kind;
};

/** Array of types. */
typedef ArrayRef<Type*> TypeArray;

/** Disjoint type. */
class UnionType : public Type {
public:
  UnionType(const TypeArray& members) : Type(Kind::UNION), _members(members) {}
  const TypeArray& members() const { return _members; }

  /** Dynamic casting support. */
  static bool classof(const UnionType* t) { return true; }
  static bool classof(const Type* t) { return t->kind() == Kind::UNION; }

private:
  const TypeArray _members;
};

/** Tuple type. */
class TupleType : public Type {
public:
  TupleType(const TypeArray& members) : Type(Kind::TUPLE), _members(members) {}
  const TypeArray& members() const { return _members; }

  /** Dynamic casting support. */
  static bool classof(const TupleType* t) { return true; }
  static bool classof(const Type* t) { return t->kind() == Kind::TUPLE; }

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

  /** Dynamic casting support. */
  static bool classof(const FunctionType* t) { return true; }
  static bool classof(const Type* t) { return t->kind() == Kind::FUNCTION; }

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

/** Nominal user-defined types such as classes, structs, interfaces and enumerations. */
class Composite : public Type {
public:
  Composite(Kind kind) : Type(kind), _defn(nullptr), _superType(nullptr) {}

  /** Definition for this type. */
  TypeDefn* defn() const { return _defn; }
  void setDefn(TypeDefn* defn) { _defn = defn; }

  /** Primary base type. */
  Type* superType() const { return _superType; }
  void setSuperType(Type* s) { _superType = s; }

  /** Explicitly supported interfaces. */
  const TypeArray& interfaces() const { return _interfaces; }
  void setInterfaces(const TypeArray& ifaces) { _interfaces = ifaces; }

  /** Dynamic casting support. */
  static bool classof(const Composite* t) { return true; }
  static bool classof(const Type* t) { return t->kind() >= Kind::CLASS && t->kind() <= Kind::ENUM; }

private:
  TypeDefn* _defn;
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

/** A specialization of a generic type. */
class SpecializedType : public Type {
public:
  SpecializedType(Type* generic, const Env& env)
    : Type(Kind::SPECIALIZED)
    , _generic(generic)
    , _env(env)
  {}

  /** The generic version of this specialized type. */
  Type* generic() const { return _generic; }
  void setGeneric(Type* t) { _generic = t; }

  /** The set of variable bindings for the generic type. */
  Env& env() { return _env; }
  const Env& env() const { return _env; }

  /** Dynamic casting support. */
  static bool classof(const SpecializedType* t) { return true; }
  static bool classof(const Type* t) { return t->kind() == Kind::SPECIALIZED; }

private:
  Type* _generic;
  Env _env;
};

/** A tuple of types that can be used as a lookup key. */
class TypeKey {
public:
  TypeKey() {}
  TypeKey(const TypeArray& members) : _members(members) {}
  TypeKey(const TypeKey& key) : _members(key._members) {}

  /** Assignment operator. */
  TypeKey& operator=(const TypeKey& key) { _members = key._members; return *this; }

  /** Equality comparison. */
  friend bool operator==(const TypeKey& lhs, const TypeKey& rhs) {
    return lhs._members == rhs._members;
  }

  /** Inequality comparison. */
  friend bool operator!=(const TypeKey& lhs, const TypeKey& rhs) {
    return lhs._members != rhs._members;
  }

  /** Iteration. */
  TypeArray::const_iterator begin() const { return _members.begin(); }
  TypeArray::const_iterator end() const { return _members.end(); }

  /** Return the length of the key. */
  size_t size() const { return _members.size(); }

  /** Read-only random access. */
  Type* operator[](int index) const {
    return _members[index];
  }

private:
  TypeArray _members;
};

}}

namespace std {
/** Compute a hash for a TypeKey. */
template<>
struct hash<spark::semgraph::TypeKey> {
  inline std::size_t operator()(const spark::semgraph::TypeKey& value) const {
    std::size_t seed = 0;
    for (spark::semgraph::Type* member : value) {
      hash_combine(seed, std::hash<spark::semgraph::Type*>()(member));
    }
    return seed;
  }
};

}

#endif
