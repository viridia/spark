// ============================================================================
// literal.h: AST representing literals
// ============================================================================

#ifndef SPARK_AST_LITERAL_H
#define SPARK_AST_LITERAL_H 1

#ifndef SPARK_AST_NODE_H
  #include "spark/ast/node.h"
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

namespace spark {
namespace ast {
using spark::collections::StringRef;

/** Character or String literal. */
class TextLiteral : public Node {
public:
  TextLiteral(Kind kind, const Location& location, StringRef value)
    : Node(kind, location)
    , _value(value)
  {}

  /** The text of character literal. */
  const StringRef value() const { return _value; }

private:
  const StringRef _value;
};

/** Integer literal. */
class IntegerLiteral : public Node {
public:
  IntegerLiteral(const Location& location, int64_t value, bool uns)
    : Node(Kind::INTEGER_LITERAL, location)
    , _value(value)
    , _unsigned(uns)
  {}

  /** The integer value. */
  // TODO: use multi-precision integers
  int64_t value() const { return _value; }

  /** Whether the integer is explicitly unsigned. */
  bool isUnsigned() const { return _unsigned; }

private:
  const int64_t _value;
  const bool _unsigned;
};

/** Float literal. */
class FloatLiteral : public Node {
public:
  FloatLiteral(const Location& location, double value)
    : Node(Kind::FLOAT_LITERAL, location)
    , _value(value)
  {}

  /** The float value. */
  double value() const { return _value; }

private:
  const double _value;
};

}}

#endif
