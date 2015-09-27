// ============================================================================
// ident.h: AST representing identifiers
// ============================================================================

#ifndef SPARK_AST_IDENT_H
#define SPARK_AST_IDENT_H 1

#ifndef SPARK_AST_NODE_H
#include <spark/ast/node.h>
#endif

#ifndef SPARK_COMMON_STRINGREF_H
#include <spark/common/stringref.h>
#endif

namespace spark {
namespace type {
class Type;
}
namespace ast {
using spark::common::StringRef;
  
/** Node type representing an identifier. */
class Ident : public Node {
public:
  
  /** Construct an Ident node. */
  Ident(Location& location, StringRef name)
    : Node(KIND_IDENT, location)
    , _name(name)
  {}

  /** The text of this identifier. */
  const StringRef name() const { return _name; }
  
private:
  const StringRef _name;
};

/** Node type representing a member reference. */
class Member : public Node {
public:
  
  /** Construct a Member node. */
  Member(Location& location, StringRef name, Node* base)
    : Node(KIND_MEMBER, location)
    , _name(name)
    , _base(base)
  {}

  /** The text of this identifier. */
  const StringRef name() const { return _name; }

  /** The container of the member. */
  const Node const* base() const { return _base; }
  
private:
  const StringRef _name;
  const Node const* _base;
};

/** Node type representing a keyword argument. */
class KeywordArg : public Node {
public:
  
  /** Construct a Member node. */
  KeywordArg(Location& location, StringRef name, Node* arg)
    : Node(KIND_KEYWORD_ARG, location)
    , _arg(arg)
  {}

  /** The text of this identifier. */
  const StringRef name() const { return _name; }

  /** The container of the member. */
  const Node const* arg() const { return _arg; }
  
private:
  const StringRef _name;
  const Node const* _arg;
};

/** Node type representing a built-in type. */
class BuiltInType : public Node {
public:
  
  /** Construct an Ident node. */
  BuiltInType(Location& location, const type::Type* type)
    : Node(KIND_BUILTIN_TYPE, location)
    , _type(type)
  {}

  /** The text of this identifier. */
  const type::Type* type() const { return _type; }
  
private:
  const type::Type* _type;
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
    : Node(KIND_BUILTIN_ATTR, location)
    , _attribute(attribute)
  {}

  /** Type of attribute. */
  const Attribute attribute() const { return _attribute; }
  
private:
  const Attribute _attribute;
};

#endif
