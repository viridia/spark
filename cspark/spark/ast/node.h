// ============================================================================
// node.h: AST nodes
// ============================================================================

#ifndef SPARK_AST_NODE_H
#define SPARK_AST_NODE_H 1

#ifndef SPARK_SOURCE_LOCATION_H
  #include "spark/source/location.h"
#endif

#ifndef SPARK_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

#if SPARK_HAVE_OSTREAM
  #include <ostream>
#endif

namespace spark {
namespace support {
class Arena;
}
namespace ast {
using spark::source::Location;

enum class Kind {
  /* Sentinel values */
  ERROR,
  ABSENT,

  /* Built-in Values */
  NULL_LITERAL,
  SELF,
  SUPER,

  /* Identifiers */
  IDENT,
  MEMBER,
  SELF_NAME_REF,
  BUILTIN_ATTRIBUTE,
  BUILTIN_TYPE,
  KEYWORD_ARG,

  /* Literals */
  BOOLEAN_TRUE,
  BOOLEAN_FALSE,
  CHAR_LITERAL,
  INTEGER_LITERAL,
  FLOAT_LITERAL,
  STRING_LITERAL,

  /* Unary operators */
  NEGATE,
  COMPLEMENT,
  LOGICAL_NOT,
  PRE_INC,
  POST_INC,
  PRE_DEC,
  POST_DEC,
  STATIC,
  CONST,
  PROVISIONAL_CONST,
  OPTIONAL,

  /* Binary operators */
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
  BIT_AND,
  BIT_OR,
  BIT_XOR,
  RSHIFT,
  LSHIFT,
  EQUAL,
  REF_EQUAL,
  NOT_EQUAL,
  LESS_THAN,
  GREATER_THAN,
  LESS_THAN_OR_EQUAL,
  GREATER_THAN_OR_EQUAL,
  IS_SUB_TYPE,
  IS_SUPER_TYPE,
  ASSIGN,
  ASSIGN_ADD,
  ASSIGN_SUB,
  ASSIGN_MUL,
  ASSIGN_DIV,
  ASSIGN_MOD,
  ASSIGN_BIT_AND,
  ASSIGN_BIT_OR,
  ASSIGN_BIT_XOR,
  ASSIGN_RSHIFT,
  ASSIGN_LSHIFT,
  LOGICAL_AND,
  LOGICAL_OR,
  RANGE,
  AS_TYPE,
  IS,
  IS_NOT,
  IN,
  NOT_IN,
  RETURNS,
  LAMBDA,
  EXPR_TYPE,
  RETURN,
  THROW,

  /* N-ary operators */
  TUPLE,
  UNION,
  SPECIALIZE,
  CALL,
  FLUENT_MEMBER,
  ARRAY_LITERAL,
  LIST_LITERAL,
  SET_LITERAL,
  CALL_REQUIRED,
  CALL_REQUIRED_STATIC,
  LIST,       // List of opions for switch cases, catch blocks, etc.

  /* Misc statements */
  BLOCK,      // A statement block
  VAR_DEFN,   // A single variable definition (ident, type, init)
  ELSE,       // default for match or switch
  FINALLY,    // finally block for try

  IF,         // if-statement (test, thenBlock, elseBlock)
  WHILE,      // while-statement (test, body)
  LOOP,       // loop (body)
  FOR,        // for (vars, init, test, step, body)
  FOR_IN,     // for in (vars, iter, body)
  TRY,        // try (test, body, cases...)
  CATCH,      // catch (except-list, body)
  SWITCH,     // switch (test, cases...)
  CASE,       // switch case (values | [values...]), body
  MATCH,      // match (test, cases...)
  PATTERN,    // match pattern (pattern, body)

  /* Type operators */
  MODIFIED,
  FUNCTION_TYPE,

  /* Other statements */
  BREAK,
  CONTINUE,

  /* Definitions */
  /* TODO: Move this outside */
  VISIBILITY,

  DEFN,
  TYPE_DEFN,
  CLASS_DEFN,
  STRUCT_DEFN,
  INTERFACE_DEFN,
  EXTEND_DEFN,
  OBJECT_DEFN,
  ENUM_DEFN,
  VAR,
  LET,
  VAR_LIST,   // A list of variable definitions
  ENUM_VALUE,
  PARAMETER,
  TYPE_PARAMETER,
  FUNCTION,
  PROPERTY,
  DEFN_END,

  MODULE,
  IMPORT,
};

class Node {
public:
  /** Construct an AST node. */
  Node(Kind kind, const Location& location) : _kind(kind), _location(location) {}

  /** What kind of node this is. */
  Kind kind() const { return _kind; }

  /** The source location where this node was parsed. */
  const source::Location& location() const { return _location; }

  static inline bool isError(const Node* node) {
    return node == nullptr || node->kind() == Kind::ERROR;
  }

  static inline bool isPresent(const Node* node) {
    return node != nullptr && node->kind() != Kind::ABSENT;
  }

  static Node ERROR; // Sentinel node that indicate errors.
  static Node ABSENT; // Sentinel node that indicate lack of a node.

  // Return the name of the specified kind.
  static const char* KindName(Kind kind);
private:
  const Kind _kind;
  const source::Location _location;
};

/** Typedef for a list of nodes. */
typedef collections::ArrayRef<const Node*> NodeList;

// How to print a node kind.
inline ::std::ostream& operator<<(::std::ostream& os, Kind kind) {
  return os << Node::KindName(kind);
}

}}

#endif
