// ============================================================================
// semgraph/primitivetype.h: built-in types.
// ============================================================================

#ifndef SPARK_SEMGRAPH_PRIMITIVETYPE_H
#define SPARK_SEMGRAPH_PRIMITIVETYPE_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_SEMGRAPH_TYPE_H
  #include "spark/semgraph/type.h"
#endif

#ifndef SPARK_SEMGRAPH_DEFN_H
  #include "spark/semgraph/defn.h"
#endif

namespace spark {
namespace semgraph {
using collections::ArrayRef;
using collections::StringRef;

/** Base class for primitive types. */
class PrimitiveType : public Type {
public:
  PrimitiveType(Kind kind, const StringRef& name)
    : Type(kind)
    , _defn(Defn::Kind::TYPE, source::Location(), name, nullptr)
  {
    _defn.setType(this);
  }

  /** Name of this primitive type. */
  StringRef name() const { return _defn.name(); }

  /** Type definition for this primitive type. */
  TypeDefn* defn() { return &_defn; }
  const TypeDefn* defn() const { return &_defn; }

  /** Members of this primitive type (like maxval). */
  scope::SymbolScope* memberScope() { return _defn.memberScope(); }

  /** Lazily-constructed scope for all primitive types. */
  static scope::SymbolScope* scope();

  /** Dynamic casting support. */
  static bool classof(const PrimitiveType* t) { return true; }
  static bool classof(const Type* t) {
    return t->kind() >= Kind::VOID && t->kind() <= Kind::FLOAT;
  }

private:
  TypeDefn _defn;

  static scope::SymbolScope* _scope;

//   # Used internally by compiler to store methods for builtin types
};

/** The 'void' type. */
class VoidType : public PrimitiveType {
public:
  VoidType()
    : PrimitiveType(Kind::VOID, "void")
  {}

  /** Dynamic casting support. */
  static bool classof(const VoidType* t) { return true; }
  static bool classof(const Type* t) { return t->kind() == Kind::VOID; }

  static VoidType VOID;
};

/** A boolean type. */
class BooleanType : public PrimitiveType {
public:
  BooleanType() : PrimitiveType(Kind::BOOLEAN, "bool") {}

  /** Dynamic casting support. */
  static bool classof(const BooleanType* t) { return true; }
  static bool classof(const Type* t) { return t->kind() == Kind::BOOLEAN; }

  static BooleanType BOOL;
};

/** An integer type. */
class IntegerType : public PrimitiveType {
public:
  IntegerType(StringRef name, int32_t bits, bool isUnsigned, bool isPositive)
    : PrimitiveType(Kind::INTEGER, name)
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

  /** Dynamic casting support. */
  static bool classof(const IntegerType* t) { return true; }
  static bool classof(const Type* t) { return t->kind() == Kind::INTEGER; }

  static IntegerType CHAR;
  static IntegerType I8;
  static IntegerType I16;
  static IntegerType I32;
  static IntegerType I64;
  static IntegerType U8;
  static IntegerType U16;
  static IntegerType U32;
  static IntegerType U64;
  static IntegerType P8;
  static IntegerType P16;
  static IntegerType P32;
  static IntegerType P64;

  void createConstant(support::Arena& arena, const StringRef& name, int32_t value);
  void createConstants(support::Arena& arena);

private:
  int32_t _bits;
  bool _unsigned;
  bool _positive;
};

/** A floating-point type. */
class FloatType : public PrimitiveType {
public:
  FloatType(StringRef name, int32_t bits) : PrimitiveType(Kind::FLOAT, name) {}

  /** Number of bits in this floating-point type. */
  int32_t bits() const { return _bits; }

  /** Dynamic casting support. */
  static bool classof(const FloatType* t) { return true; }
  static bool classof(const Type* t) { return t->kind() == Kind::FLOAT; }

  static FloatType F32;
  static FloatType F64;

private:
  int32_t _bits;
};

/** The null pointer type. */
class NullPtrType : public PrimitiveType {
public:
  NullPtrType() : PrimitiveType(Kind::NULLPTR, "nullptr") {}

  /** Dynamic casting support. */
  static bool classof(const NullPtrType* t) { return true; }
  static bool classof(const Type* t) { return t->kind() == Kind::NULLPTR; }

  static NullPtrType NULLPTR;
};

void createConstants(support::Arena& arena);

}}

#endif
