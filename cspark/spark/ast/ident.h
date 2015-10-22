// ============================================================================
// ident.h: AST representing identifiers
// ============================================================================

#ifndef SPARK_AST_IDENT_H
#define SPARK_AST_IDENT_H 1

#ifndef SPARK_AST_NODE_H
#include <spark/ast/node.h>
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
#include <spark/collections/stringref.h>
#endif

namespace spark {
namespace ast {
using spark::collections::StringRef;

/** Node type representing an identifier. */
class Ident : public Node {
public:

  /** Construct an Ident node. */
  Ident(const Location& location, StringRef name)
    : Node(Kind::IDENT, location)
    , _name(name)
  {}

  /** The text of this identifier. */
  const StringRef name() const { return _name; }

private:
  const StringRef _name;
};

/** Node type representing a member reference. */
class MemberRef : public Node {
public:

  /** Construct a Member node. */
  MemberRef(const Location& location, StringRef name, Node* base)
    : Node(Kind::MEMBER, location)
    , _name(name)
    , _base(base)
  {}

  /** The text of this identifier. */
  const StringRef name() const { return _name; }

  /** The container of the member. */
  const Node* base() const { return _base; }

private:
  const StringRef _name;
  const Node* _base;
};

/** Node type representing a keyword argument. */
class KeywordArg : public Node {
public:

  /** Construct a Member node. */
  KeywordArg(Location& location, StringRef name, Node* arg)
    : Node(Kind::KEYWORD_ARG, location)
    , _arg(arg)
  {}

  /** The text of this identifier. */
  const StringRef name() const { return _name; }

  /** The container of the member. */
  const Node* arg() const { return _arg; }

private:
  const StringRef _name;
  const Node* _arg;
};

/** Node type representing a built-in type. */
class BuiltInType : public Node {
public:
  enum Type {
    VOID,
    BOOL,
    CHAR,
    I8,
    I16,
    I32,
    I64,
    INT,
    U8,
    U16,
    U32,
    U64,
    UINT,
    F32,
    F64,
  };

  /** Construct an Ident node. */
  BuiltInType(const Location& location, Type type)
    : Node(Kind::BUILTIN_TYPE, location)
    , _type(type)
  {}

  /** The actual type. */
  Type type() const { return _type; }

private:
  Type _type;
};

/** Node type representing a built-in attribute. */
class BuiltInAttribute : public Node {
public:
  enum Attribute {
    INTRINSIC = 0,
    TRACEMETHOD,
    UNSAFE
  };

  /** Construct an Ident node. */
  BuiltInAttribute(Location& location, Attribute attribute)
    : Node(Kind::BUILTIN_ATTRIBUTE, location)
    , _attribute(attribute)
  {}

  /** Type of attribute. */
  Attribute attribute() const { return _attribute; }

private:
  const Attribute _attribute;
};

}}

#endif
