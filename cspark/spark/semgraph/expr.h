// ============================================================================
// semgraph/expr.h: Semantic graph nodes for expressions.
// ============================================================================

#ifndef SPARK_SEMGRAPH_EXPR_H
#define SPARK_SEMGRAPH_EXPR_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_SOURCE_LOCATION_H
  #include "spark/source/location.h"
#endif

#ifndef SPARK_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

#ifndef SPARK_SUPPORT_ARRAYBUILDER_H
  #include "spark/support/arraybuilder.h"
#endif

namespace spark {
namespace error { class Reporter; }
namespace scope { class SymbolScope; }
namespace semgraph {
using collections::ArrayRef;
using collections::StringRef;
using source::Location;

class Type;
class Member;
class Defn;
class ValueDefn;

/** An expression. */
class Expr {
public:
  enum class Kind {
    INVALID,
    IGNORED,
    SELF,
    SUPER,
    NAMESPACE,      // Reference to a module or package name.
    BUILTIN_ATTR,

    // Literals
    NIL,            // null
    BOOL_LITERAL,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    DOUBLE_LITERAL,
    STRING_LITERAL,
    ARRAY_LITERAL,

    // Unary operators
    NEGATE,                     // Arithmetic negation
    COMPLEMENT,                 // Bitwise complement
    LOGICAL_NOT,                // Logical complement
    OPTIONAL,                   // Nullable and/or voidable type
// #  PRE_INC, POST_INC,            # Pre / post increment
// #  PRE_DEC, POST_DEC,            # Pre / post decrement
//
//   # Builtin comparison operators
// #  REF_EQ,
//   IS_SUBTYPE, IS_SUPERTYPE,
//
    // Assignment operator
    ASSIGN,
    UNPACK,
//
//   # Cast operators
//   UP_CAST,                      # Cast from subclass to base type.
//   DOWN_CAST,                    # Cast from base type to subclass (unconditional)
//   TRY_CAST,                     # Cast from base to subclass, throw if fail.
//   IFACE_CAST,                   # Cast from type to interface which it is known to support.
//   DYN_IFACE_CAST,               # Cast from type to interface, throw if fail.
//   UNION_CTOR_CAST,              # Cast to a union type
//   UNION_MEMBER_CAST,            # Cast from a union type
//   BOX_CAST,                     # Cast from value type to reference type.
//   UNBOX_CAST,                   # Cast from reference type to value type.
// #  DYNAMIC_CAST,                 # Cast from base/iface to subclass, null if fail
// #  EXPR_TYPE(QualCast)       // Cast that changes only qualifiers (no effect)
// #  EXPR_TYPE(UnionMemberCast)// Cast from a union type.
// #  EXPR_TYPE(CheckedUnionMemberCast)// Cast from a union type, with type check.
//   REP_CAST,                     # Cast between types that have the exact same machine representation (example: char and u32).
//   TRUNCATE,                     # Number truncation.
//   SIGN_EXTEND,                  # Signed integer extend.
//   ZERO_EXTEND,                  # Unsigned integer extend.
//   FP_EXTEND,                    # Floating-point extend.
//   FP_TRUNC,                     # Floating-point truncate.
//   INT_TO_FLOAT,                 # Convert integer to float.
//   INTERFACE_DATA,               # Extract interface payload.
// # EXPR_TYPE(BitCast)        // Reinterpret cast
//
    // Misc operators
    LOGICAL_AND,
    LOGICAL_OR,
    RANGE,
    PACK,
    TYPE_TEST,
    TYPE_CAST,
// #  IN,
// #  NOT_IN,
//   RETURNS,
    PROG,                         // Evaluate a list of expressions and return the last one.
//   ANON_FN,                      # Anonymous function
//   TUPLE_MEMBER,                 # Return the Nth member of a tuple.
//   # TPARAM_DEFAULT,

    // Evaluations and Invocations
    CALL,                         // Function call
    SPECIALIZE,                 // Specialize (this form uses expressions as arguments).

    // Member references
    MEMBER_SET,                  // List of members - results of a lookup
//   DEFN_REF,                     # Reference to a definition (produced from MEMBER_LIST).
//   FLUENT_MEMBER,                # Sticky reference to member (a.{b;c;d})
    TEMP_VAR,                   // Reference to an anonymous, temporary variable.
//   EXPLICIT_SPECIALIZE,          # An explicit specialization that could not be resolved at name-lookup time.
//
//   # Statements
    BLOCK,                        // Block statement
    IF,
    WHILE,
    LOOP,
    FOR,                          // C-style for
    FOR_IN,                       // for x in y
    THROW,
    TRY,
    RETURN,
    // YIELD,
    BREAK,
    CONTINUE,
    LOCAL_DEFN,
    SWITCH,
    SWITCH_CASE,
    MATCH,
    MATCH_PATTERN,
//   INTRINSIC,                    # Built-in function
//   UNREACHABLE,                  # Indicates control cannot reach this point
//
    // Other syntax
    KEYWORD_ARG,
//
//   # Constants
//   CONST_OBJ,                    # A static, immutable object
//   CONST_ARRAY,                  # A static, immutable array

    // Type expressions
    //MODIFIED,
    CONST_TYPE,                     // 'const' type modifier.
    PROVISIONAL_CONST_TYPE,         // 'const?' type modifier.
    UNION_TYPE,                     // union type expression.
    FUNCTION_TYPE,                  // function type expression.

//   # Lowered expressions
//   CALL_STATIC,                  # Call directly
//   CALL_INDIRECT,                # Call an instance method by table index
//   CALL_REQUIRED,                # Call a required method by table index
//   CALL_CTOR,                    # Construct object and call constructor
//   CALL_EXPR,                    # Call where the callable is an expression of function type
//   CALL_INTRINSIC,               # Generate code from an intrinsic
//   FNREF_STATIC,                 # Reference to a static function
//   FNREF_INDIRECT,               # Reference to a function looked up by method index
//   FNREF_REQUIRED,               # Reference to a required method looked up by method index
//   ALLOC,                        # Allocate an uninitialized object
//   GC_ALLOC,                     # Allocate an object on the heap
//   TYPEDESC,                     # Direct reference to a type descriptor
//   PTR_DEREF,                    # Pointer dereference.
//   ADDRESS_OF,                   # Address of a value, guaranteed not to be on the heap.
//   OFFSET_OF,                    # Offset of a field within a class.
//   UNBOX,                        # Unboxing of a type.
  };

  void* operator new(size_t size, support::Arena& arena) {
    return arena.allocate(size);
  }

  Expr(Kind kind) : _kind(kind), _type(nullptr) {}
  Expr(Kind kind, const Location& loc) : _kind(kind), _location(loc), _type(nullptr) {}
  Expr() = delete;

  /** What kind of expression this is. */
  Kind kind() const { return _kind; }

  const Location& location() const { return _location; }
  void setLocation(const Location& l) { _location = l; }

  /** Data type of this expression. */
  Type* type() const { return _type; }
  void setType(Type* t) { _type = t; }

  Expr& operator=(const Expr&) = delete;

  /** Test whether an expression result is an error sentinel. */
  static bool isError(Expr* e) {
    return e == nullptr || e->_kind == Kind::INVALID;
  }

  static Expr ERROR;
  static Expr IGNORED;

private:
  Kind _kind;
  Location _location;
  Type* _type;
};

/** A boolean constant. */
class BooleanLiteral : public Expr {
public:
  BooleanLiteral(const Location& location, bool value)
    : Expr(Kind::BOOL_LITERAL, location)
    , _value(value)
  {}

  /** The literal value. */
  bool value() const { return _value; }

private:
  bool _value;
};

/** An integer constant. */
class IntegerLiteral : public Expr {
public:
  IntegerLiteral(const Location& location, int64_t value, bool isUnsigned)
    : Expr(Kind::INTEGER_LITERAL, location)
    , _value(value)
  {}

  /** The literal value. */
  int64_t value() const { return _value; }

  /** The literal value. */
  bool isUnsigned() const { return _unsigned; }

private:
  int64_t _value;
  bool _unsigned;
};

/** A floating-point constant. */
class FloatLiteral : public Expr {
public:
  FloatLiteral(const Location& location, double value)
    : Expr(Kind::FLOAT_LITERAL, location)
    , _value(value)
  {}

  /** The literal value. */
  double value() const { return _value; }

private:
  double _value;
};

/** A string constant. */
class StringLiteral : public Expr {
public:
  StringLiteral(const Location& location, const StringRef& value)
    : Expr(Kind::STRING_LITERAL, location)
    , _value(value)
  {}

  /** The literal value. */
  const StringRef& value() const { return _value; }

private:
  const StringRef& _value;
};

/** An expression in which an operator is applied to a single. */
class UnaryOp : public Expr {
public:
  UnaryOp(Kind kind, const Location& location, Expr* arg = nullptr)
    : Expr(kind, location)
    , _arg(arg)
  {}

  /** The argument expression. */
  Expr* arg() const { return _arg; }
  void setArg(Expr* e) { _arg = e; }

private:
  Expr* _arg;
};

/** An expression in which an operator is applied to two arguments. */
class BinaryOp : public Expr {
public:
  BinaryOp(Kind kind, const Location& location, Expr* l = nullptr, Expr* r = nullptr)
    : Expr(kind, location)
    , _left(l)
    , _right(r)
  {}

  /** The left-hand expression. */
  Expr* left() const { return _left; }
  void setLeft(Expr* e) { _left = e; }

  /** The right-hand expression. */
  Expr* right() const { return _right; }
  void setRight(Expr* e) { _right = e; }

private:
  Expr* _left;
  Expr* _right;
};

/** An expression in which an operator is applied to an arbitrary number of arguments. */
class MultiArgOp : public Expr {
public:
  MultiArgOp(
      Kind kind,
      const Location& location,
      const ArrayRef<Expr*>& arguments = ArrayRef<Expr*>())
    : Expr(kind, location)
    , _arguments(arguments)
  {}

  /** The list of arguments. */
  const ArrayRef<Expr*>& arguments() const { return _arguments; }
  void setArguments(const ArrayRef<Expr*>& arguments) { _arguments = arguments; }

private:
  ArrayRef<Expr*> _arguments;
};

/** An assignment from a single tuple value to multiple lvalues. */
class Unpack : public Expr {
public:
  Unpack(
      const Location& location,
      Expr* rvalue = nullptr,
      const ArrayRef<Expr*>& lvalues = ArrayRef<Expr*>())
    : Expr(Kind::UNPACK, location)
    , _rvalue(rvalue)
    , _lvalues(lvalues)
  {}

  /** The tuple source expression. */
  Expr* rvalue() const { return _rvalue; }
  void setRvalue(Expr* e) { _rvalue = e; }

  /** The list of lvalues to assign to. */
  const ArrayRef<Expr*>& lvalues() const { return _lvalues; }
  void setLvalues(const ArrayRef<Expr*>& lvalues) { _lvalues = lvalues; }

private:
  Expr* _rvalue;
  ArrayRef<Expr*> _lvalues;
};

/** An expression in which an operator represented as an expression is applied to an array
    of arguments. */
class Call : public Expr {
public:
  Call(Kind kind, const Location& location)
    : Expr(kind, location)
    , _callable(nullptr)
  {}

  /** The callable operator. */
  Expr* callable() const { return _callable; }
  void setCallable(Expr* callable) { _callable = callable; }

  /** The list of arguments. */
  const ArrayRef<Expr*>& arguments() const { return _arguments; }
  void setArguments(const ArrayRef<Expr*>& arguments) { _arguments = arguments; }

private:
  Expr* _callable;
  ArrayRef<Expr*> _arguments;
};

/** Member List: contains a set of overloaded members derived from a name lookup. One member
    will eventually be chosen during type inference. */
class MemberSet : public Expr {
public:
  /** Another way of categorizing members, useful in determining whether a symbol is
      actually ambiguous or merely overloaded. */
  enum class Genus {
    INCOMPLETE,     // We don't actually know yet
    INCONSISTENT,   // Set contains more than one genus, which is an error.
    NAMESPACE,      // Package or module
    FUNCTION,       // List of functions, possibly overloaded
    TYPE,           // List of types, possibly overloaded
    VARIABLE,       // A reference to a variable, parameter, or property
  };

  MemberSet(const StringRef& name)
    : Expr(Kind::MEMBER_SET)
    , _name(name)
    , _stem(nullptr)
    , _genus(Genus::INCOMPLETE)
  {}

  MemberSet(const StringRef& name, const Location& loc)
    : Expr(Kind::MEMBER_SET, loc)
    , _name(name)
    , _stem(nullptr)
    , _genus(Genus::INCOMPLETE)
  {}

  /** The name shared by all of the members in the list. */
  const StringRef& name() const { return _name; }

  /** The expression on the left-hand side of the dereference, e.g. for an expression like
      self.fieldName, the stem would be 'self. */
  Expr* stem() const { return _stem; }
  void setStem(Expr* stem) { _stem = stem; }

  /** The list of members. */
  const ArrayRef<Member*>& members() const { return _members; }
  void setMembers(const ArrayRef<Member*>& members) { _members = members; }

  /** A categorization of the members of the list (type, namespace, variable, etc.) All members
      must be of the same genus. */
  Genus genus() const { return _genus; }
  void setGenus(Genus g) { _genus = g; }

private:
  StringRef _name;
  Expr* _stem;
  ArrayRef<Member*> _members;
  Genus _genus;
};

/** An operation which involves a value expression and a type. */
class TypeOp : public UnaryOp {
public:
  TypeOp(Kind kind, const Location& location, Expr* arg, Type* type)
    : UnaryOp(kind, location, arg)
    , _type(type)
  {}

  /** The type we're testing against. */
  Type* type() const { return _type; }
  void setType(Type* t) { _type = t; }

private:
  Type* _type;
};

/** A statement block. */
class Block : public Expr {
public:
  Block(
      const Location& location,
      const ArrayRef<Expr*>& stmts = ArrayRef<Expr*>())
    : Expr(Kind::BLOCK, location)
    , _stmts(stmts)
  {}

  /** The list of stmts to assign to. */
  const ArrayRef<Expr*>& stmts() const { return _stmts; }
  void setLvalues(const ArrayRef<Expr*>& stmts) { _stmts = stmts; }

private:
  ArrayRef<Expr*> _stmts;
};

/** A local definition statement. */
class LocalDefn : public Expr {
public:
  LocalDefn(const Location& location, Defn* defn)
    : Expr(Kind::LOCAL_DEFN, location)
    , _defn(defn)
  {}

  /** The definition being defined. */
  Defn* defn() const { return _defn; }

private:
  Defn* _defn;
};

/** A reference to a temporary variable. Used to hold results of expressions that are consumed
    multiple times within a function. */
class TempVarRef : public Expr {
public:
  TempVarRef(const Location& location, Expr* init, int32_t index)
    : Expr(Kind::TEMP_VAR, location)
    , _init(init)
    , _index(index)
  {}

  /** Value that the temp var is initialized to. */
  Expr* init() const { return _init; }

  /** Index of the variable. Positive if it's local to a function, negative if it's local
      to a module. */
  int32_t index() const { return _index; }

private:
  Expr* _init;          // Initialization of this value.
  int32_t _index;       // Unique ID of this variable within the function.
};

/** An if-statement. */
class IfStmt : public Expr {
public:
  IfStmt(const Location& location, Expr* test, Expr* thenBlock, Expr* elseBlock)
    : Expr(Kind::IF, location)
    , _test(test)
    , _thenBlock(thenBlock)
    , _elseBlock(elseBlock)
  {}

  /** The test expression. */
  Expr* test() const { return _test; }

  /** The 'then' block. */
  Expr* thenBlock() const { return _thenBlock; }

  /** The 'else' block. */
  Expr* elseBlock() const { return _elseBlock; }

private:
  Expr* _test;
  Expr* _thenBlock;
  Expr* _elseBlock;
};

/** A while-statement. */
class WhileStmt : public Expr {
public:
  WhileStmt(const Location& location, Expr* test, Expr* body)
    : Expr(Kind::WHILE, location)
    , _test(test)
    , _body(body)
  {}

  /** The test expression. */
  Expr* test() const { return _test; }

  /** The 'then' block. */
  Expr* body() const { return _body; }

private:
  Expr* _test;
  Expr* _body;
};

#if 0
struct Loop(Expr) = ExprType.LOOP {
  body: Expr = 2;
}
#endif

/** A for-statement. */
class ForStmt : public Expr {
public:
  ForStmt(const Location& location)
    : Expr(Kind::FOR, location)
    , _init(nullptr)
    , _test(nullptr)
    , _step(nullptr)
    , _body(nullptr)
  {}

  /** The list of defined variables. */
  const ArrayRef<ValueDefn*>& vars() const { return _vars; }
  void setVars(const ArrayRef<ValueDefn*>& vars) { _vars = vars; }

  /** The init expression. */
  Expr* init() const { return _init; }
  void setInit(Expr* e) { _init = e; }

  /** The test expression. */
  Expr* test() const { return _test; }
  void setTest(Expr* e) { _test = e; }

  /** The step expression. */
  Expr* step() const { return _step; }
  void setStep(Expr* e) { _step = e; }

  /** The loop body. */
  Expr* body() const { return _body; }
  void setBody(Expr* e) { _body = e; }

private:
  ArrayRef<ValueDefn*> _vars;
  Expr* _init;
  Expr* _test;
  Expr* _step;
  Expr* _body;
};

/** A for-in-statement. */
class ForInStmt : public Expr {
public:
  ForInStmt(const Location& location)
    : Expr(Kind::FOR, location)
    , _iter(nullptr)
    , _body(nullptr)
    , _iterVar(nullptr)
    , _nextVar(nullptr)
  {}

  /** The list of defined variables. */
  const ArrayRef<ValueDefn*>& vars() const { return _vars; }
  void setVars(const ArrayRef<ValueDefn*>& vars) { _vars = vars; }

  /** The init expression. */
  Expr* iter() const { return _iter; }
  void setIter(Expr* e) { _iter = e; }

  /** The loop body. */
  Expr* body() const { return _body; }
  void setBody(Expr* e) { _body = e; }

  /** The variable that contains the iteration expression. */
  ValueDefn* iterVar() const { return _iterVar; }
  void setIterVar(ValueDefn* vd) { _iterVar = vd; }

  /** The variable that contains the value of iterator.next(). This is used to determine
      if we've reached the end of the sequence before assigning the values to the individual
      loop vars. */
  ValueDefn* nextVar() const { return _nextVar; }
  void setNextVar(ValueDefn* vd) { _nextVar = vd; }

private:
  ArrayRef<ValueDefn*> _vars;
  Expr* _iter;
  Expr* _body;
  ValueDefn* _iterVar;
  ValueDefn* _nextVar;
};

/** A switch case statement. */
class CaseStmt : public Expr {
public:
  CaseStmt(const Location& location)
    : Expr(Kind::SWITCH_CASE, location)
    , _body(nullptr)
  {}

  /** The set of constant case values to compare to the test expression. */
  const ArrayRef<Expr*>& values() const { return _values; }
  void setValues(const ArrayRef<Expr*>& values) { _values = values; }

  /** The set of constant case ranges to compare to the test expression. */
  const ArrayRef<Expr*>& rangeValues() const { return _rangeValues; }
  void setRangeValues(const ArrayRef<Expr*>& rangeValues) { _rangeValues = rangeValues; }

  /** The body of the case statement. */
  Expr* body() const { return _body; }
  void setBody(Expr* body) { _body = body; }

private:
  ArrayRef<Expr*> _values;
  ArrayRef<Expr*> _rangeValues;
  Expr* _body;
};

/** A switch statement. */
class SwitchStmt : public Expr {
public:
  SwitchStmt(const Location& location)
    : Expr(Kind::SWITCH, location)
    , _testExpr(nullptr)
    , _elseBody(nullptr)
  {}

  /** The test expression. */
  Expr* testExpr() const { return _testExpr; }
  void setTestExpr(Expr* e) { _testExpr = e; }

  /** The list of cases. */
  const ArrayRef<CaseStmt*>& cases() const { return _cases; }
  void setCases(const ArrayRef<CaseStmt*>& cases) { _cases = cases; }

  /** The loop body. */
  Expr* elseBody() const { return _elseBody; }
  void setElseBody(Expr* e) { _elseBody = e; }

private:
  Expr* _testExpr;
  ArrayRef<CaseStmt*> _cases;
  Expr* _elseBody;
};

/** A match pattern. */
class Pattern : public Expr {
public:
  Pattern(const Location& location)
    : Expr(Kind::MATCH_PATTERN, location)
    , _var(nullptr)
    , _body(nullptr)
  {}

  /** The variable that the expression is assigned to. */
  ValueDefn* var() const { return _var; }
  void setVar(ValueDefn* var) { _var = var; }

  /** The body of the case statement. */
  Expr* body() const { return _body; }
  void setBody(Expr* body) { _body = body; }

private:
  ValueDefn* _var;
  Expr* _body;
};

/** A match statement. */
class MatchStmt : public Expr {
public:
  MatchStmt(const Location& location)
    : Expr(Kind::SWITCH, location)
    , _testExpr(nullptr)
    , _elseBody(nullptr)
  {}

  /** The test expression. */
  Expr* testExpr() const { return _testExpr; }
  void setTestExpr(Expr* e) { _testExpr = e; }

  /** The list of cases. */
  const ArrayRef<Pattern*>& patterns() const { return _patterns; }
  void setPatterns(const ArrayRef<Pattern*>& patterns) { _patterns = patterns; }

  /** The loop body. */
  Expr* elseBody() const { return _elseBody; }
  void setElseBody(Expr* e) { _elseBody = e; }

private:
  Expr* _testExpr;
//   Type* _testType;
  ArrayRef<Pattern*> _patterns;
  Expr* _elseBody;
};

#if 0

struct Try(Expr) = ExprType.TRY {
  struct Catch {
    location: spark.location.SourceLocation = 1;
    exceptDefn: defn.Var = 2;
    body: Expr = 3;
  }
  body: Expr = 1;
  catchList: list[Catch] = 2;
  else: Expr = 3;
  finally: Expr = 4;
}

struct Return(UnaryOp) = ExprType.RETURN {}
struct Throw(UnaryOp) = ExprType.THROW {}
struct Break(Oper) = ExprType.BREAK {}
struct Continue(Oper) = ExprType.CONTINUE {}
struct Unreachable(Oper) = ExprType.UNREACHABLE {}

struct LocalDefn(Expr) = ExprType.LOCAL_DECL {
  defn: defn.Defn = 1;
}
struct Intrinsic(Expr) = ExprType.INTRINSIC {}
#endif

/** A keyword argument to a method call. */
class KeywordArg : public Expr {
public:
  KeywordArg(const Location& location, const StringRef& keyword, Expr* arg)
    : Expr(Expr::Kind::KEYWORD_ARG, location)
    , _keyword(keyword)
    , _arg(arg)
  {}

  /** The keyword. */
  const StringRef& keyword() const { return _keyword; }
  void setKeyword(const StringRef& keyword) { _keyword = keyword; }

  /** The argument expression. */
  Expr* arg() const { return _arg; }
  void setArg(Expr* e) { _arg = e; }

private:
  StringRef _keyword;
  Expr* _arg;
};

/** Array builder for expression lists. */
class ExprArrayBuilder : public support::ArrayBuilder<Expr*> {
public:
  ExprArrayBuilder(support::Arena& arena)
    : support::ArrayBuilder<Expr*>(arena) {}
  ExprArrayBuilder(support::Arena& arena, const collections::ArrayRef<Expr*>& init)
    : support::ArrayBuilder<Expr*>(arena, init) {}
  ExprArrayBuilder(support::Arena& arena, const std::vector<Expr*>& init)
    : support::ArrayBuilder<Expr*>(arena, init) {}
  ExprArrayBuilder(support::Arena& arena, const std::vector<Expr*>&& init)
    : support::ArrayBuilder<Expr*>(arena, init) {}

  /** Return a location which spans all of the expressions. */
  Location location() const {
    Location loc;
    for (Expr* e : *this) {
      loc |= e->location();
    }
    return loc;
  }
};

}}

#endif
