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
    : Node(KIND_IDENT, location)
    , _name(name)
  {}

  /** The text of this identifier. */
  const StringRef name() const { return _name; }

  static inline bool classof(const Ident*) { return true; }
  static inline bool classof(const Node* node) {
      return node->kind() == Node::KIND_IDENT;
  }
private:
  const StringRef _name;
};

/** Node type representing a member reference. */
class Member : public Node {
public:

  /** Construct a Member node. */
  Member(const Location& location, StringRef name, Node* base)
    : Node(KIND_MEMBER, location)
    , _name(name)
    , _base(base)
  {}

  /** The text of this identifier. */
  const StringRef name() const { return _name; }

  /** The container of the member. */
  const Node* base() const { return _base; }

  static inline bool classof(const Member*) { return true; }
  static inline bool classof(const Node* node) {
      return node->kind() == Node::KIND_MEMBER;
  }
private:
  const StringRef _name;
  const Node* _base;
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
  const Node* arg() const { return _arg; }

  static inline bool classof(const KeywordArg*) { return true; }
  static inline bool classof(const Node* node) {
      return node->kind() == Node::KIND_KEYWORD_ARG;
  }
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
    : Node(KIND_BUILTIN_TYPE, location)
    , _type(type)
  {}

  /** The actual type. */
  Type type() const { return _type; }

  static inline bool classof(const BuiltInType*) { return true; }
  static inline bool classof(const Node* node) {
      return node->kind() == Node::KIND_BUILTIN_TYPE;
  }
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
    : Node(KIND_BUILTIN_ATTRIBUTE, location)
    , _attribute(attribute)
  {}

  /** Type of attribute. */
  Attribute attribute() const { return _attribute; }

  static inline bool classof(const BuiltInAttribute*) { return true; }
  static inline bool classof(const Node* node) {
      return node->kind() == Node::KIND_BUILTIN_ATTRIBUTE;
  }
private:
  const Attribute _attribute;
};

}}

#endif
