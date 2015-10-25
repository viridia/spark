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
namespace semgraph {
using collections::ArrayRef;
using collections::StringRef;
using source::Location;

class Type;
class Member;
class Defn;

/** An expression. */
class Expr {
public:
  enum class Kind {
    INVALID,
    SELF,
    SUPER,
    NAMESPACE,      // Reference to a module or package name.
    BUILTIN_ATTR,

    // Literals
    TRUE,
    FALSE,
    NIL,            // null
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    DOUBLE_LITERAL,
    STRING_LITERAL,
    ARRAY_LITERAL,

//     // etc.
//   # Unary operators
// #  NEGATE,                       # Arithmetic negation
// #  COMPLEMENT,                   # Bitwise complement
//   LOGICAL_NOT,                  # Logical complement
// #  PRE_INC, POST_INC,            # Pre / post increment
// #  PRE_DEC, POST_DEC,            # Pre / post decrement
//
//   # Builtin comparison operators
// #  REF_EQ,
//   IS_SUBTYPE, IS_SUPERTYPE,
//
//   # Assignment operator
//   ASSIGN,
//   UNPACK,
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
//   # Misc operators
//   LOGICAL_AND, LOGICAL_OR,
//   RANGE,
//   PACK,
//   AS_TYPE,
//   IS_TYPE,
// #  IN,
// #  NOT_IN,
//   RETURNS,
//   PROG,                         # Evaluate a list of expressions and return the last one.
//   ANON_FN,                      # Anonymous function
//   TUPLE_MEMBER,                 # Return the Nth member of a tuple.
//   # TPARAM_DEFAULT,

    // Evaluations and Invocations
    CALL,                         // Function call
    SPECIALIZE,                 // Specialize (this form uses expressions as arguments).

    // Member references
    MEMBER_SET,                  // List of members - results of a lookup
//   DEFN_REF,                     # Reference to a definition (produced from MEMBER_LIST).
//   PRIVATE_NAME,                 # Reference to private member (.a)
//   FLUENT_MEMBER,                # Sticky reference to member (a.{b;c;d})
//   TEMP_VAR,                     # Reference to an anonymous, temporary variable.
//   EXPLICIT_SPECIALIZE,          # An explicit specialization that could not be resolved at name-lookup time.
//
//   # Statements
//   BLOCK,                        # Block statement
//   IF,
//   WHILE,
//   LOOP,
//   FOR,                          # C-style for
//   FOR_IN,                       # for x in y
//   THROW,
//   TRY,
//   RETURN,
//   # YIELD,
//   BREAK,
//   CONTINUE,
//   LOCAL_DECL,
//   IMPORT,
//   SWITCH, SWITCH_CASE,
//   MATCH, MATCH_PATTERN,
//   INTRINSIC,                    # Built-in function
//   UNREACHABLE,                  # Indicates control cannot reach this point
//
//   # Other syntax
//   KEYWORD_ARG,
//
//   # Constants
//   CONST_OBJ,                    # A static, immutable object
//   CONST_ARRAY,                  # A static, immutable array
//
//   # Type expressions
//   MODIFIED,
//
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

private:
  Kind _kind;
  Location _location;
  Type* _type;
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
    INCOMPLETE, // We don't actually know yet
    NAMESPACE,  // Package or module
    FUNCTION,   // List of functions, possibly overloaded
    TYPE,       // List of types, possibly overloaded
    VARIABLE,   // A reference to a variable, parameter, or property
  };

  MemberSet(const StringRef& name)
    : Expr(Kind::MEMBER_SET)
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
