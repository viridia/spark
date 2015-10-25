// ============================================================================
// semgraph/defn.h: Semantic graph nodes for definitions.
// ============================================================================

#ifndef SPARK_SEMGRAPH_DEFN_H
#define SPARK_SEMGRAPH_DEFN_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

#ifndef SPARK_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

#ifndef SPARK_SOURCE_LOCATION_H
  #include "spark/source/location.h"
#endif

#ifndef SPARK_SCOPE_STDSCOPE_H
  #include "spark/scope/stdscope.h"
#endif

#ifndef SPARK_SCOPE_INHERITEDSCOPE_H
  #include "spark/scope/inheritedscope.h"
#endif

#ifndef SPARK_SEMGRAPH_ENV_H
  #include "spark/semgraph/env.h"
#endif

#if SPARK_HAVE_MEMORY
  #include <memory>
#endif

namespace spark {
namespace ast {
class Node;
}
namespace semgraph {
using collections::ArrayRef;
using collections::StringRef;

class Expr;
class FunctionType;
class Type;
class TypeParameter;
class TypeVariable;

typedef ArrayRef<Type*> TypeArray;

enum Visibility {
  PUBLIC,
  PROTECTED,
  PRIVATE,
  INTERNAL,
};

/** Base class for all members within a scope. */
class Member {
public:
  enum class Kind {
    // Definitions
    TYPE = 1,
    LET,
    VAR,
    ENUM_VAL,
    PARAM,
    TYPE_PARAM,
    FUNCTION,
    PROPERTY,

    MODULE,
    PACKAGE,

    // Internal types
    SPECIALIZED,   // A combination of generic member and type arguments for it
    TUPLE_MEMBER,
  };

  Member(Kind kind, const StringRef& name, Member* definedIn = nullptr)
    : _kind(kind)
    , _name(name.begin(), name.end())
    , _definedIn(definedIn)
    , _ast(nullptr)
  {}

  virtual ~Member();

  /** What kind of member this is. */
  Kind kind() const { return _kind; }

  /** Abstract syntax tree for this member. */
  const ast::Node* ast() const { return _ast; }
  void setAst(const ast::Node* ast) { _ast = ast; }

  /** The name of this member. */
  StringRef name() const { return _name; }

  /** Return the definition enclosing this one. */
  Member* definedIn() const { return _definedIn; }

  /** Produce a string representation of this member (for unit tests). */
  virtual void format(std::ostream& out) const = 0;

  /** Return the complete qualified name of this member, not including type parameters. */
  std::string qualifiedName() const {
    std::string result;
    qualifiedName(result);
    return result;
  }
  void qualifiedName(std::string& result) const {
    if (_definedIn) {
      _definedIn->qualifiedName(result);
      result.push_back('.');
    }
    result.insert(result.end(), _name.begin(), _name.end());
  }

protected:


  const Kind _kind;
  const std::string _name;
  Member* _definedIn;
  const ast::Node* _ast;
};

/** List of members. */
typedef std::vector<Member*> MemberList;
typedef ArrayRef<Member*> MemberArray;

/** A definition, that is, any type or object that has a name. */
class Defn : public Member {
public:
  Defn(Kind kind, const source::Location& location, const StringRef& name, Member* definedIn = nullptr)
    : Member(kind, name, definedIn)
    , _location(location)
    , _visibility(PUBLIC)
    , _final(false)
    , _override(false)
    , _abstract(false)
    , _undef(false)
    , _static(false)
    , _overloadIndex(0)
  {}

  /** Source location where this was defined. */
  const source::Location& location() const { return _location; }

  /** Visibility of this symbol. */
  Visibility visibility() const { return _visibility; }
  void setVisibility(Visibility visibility) { _visibility = visibility; }

  /** Whether this definition has the 'static' modifier. */
  bool isStatic() const { return _static; }
  void setStatic(bool value) { _static = value; }

  /** Whether this definition has the 'override' modifier. */
  bool isOverride() const { return _override; }
  void setOverride(bool value) { _override = value; }

  /** Whether this definition has the 'undef' modifier. */
  bool isUndef() const { return _undef; }
  void setUndef(bool value) { _undef = value; }

  /** If true, this definition can't be overridden. */
  bool isFinal() const { return _final; }
  void setFinal(bool value) { _final = value; }

  /** If true, this definition can't be instantiated. */
  bool isAbstract() const { return _abstract; }
  void setAbstract(bool value) { _abstract = value; }

  // If non-zero, means this is the Nth member with the same name.
  int32_t overloadIndex() const { return _overloadIndex; }
  void setOverloadIndex(int32_t index) { _overloadIndex = index; }

protected:
  void formatModifiers(std::ostream& out) const;

protected:
  source::Location _location;
  Visibility _visibility;
  bool _final;
  bool _override;               // Overridden method
  bool _abstract;
  bool _undef;                  // Undefined method
  bool _static;                 // Was declared static

//   ArrayRef<Member*> _members;
  ArrayRef<Expr*> _attributes;

  int32_t _overloadIndex;
//  docComment: DocComment = 7;   # Doc comments

//  requirements: list[Requirement] = 10;   # List of 'where' constraints
//  fulfillments: list[Function] = 11;      # Fulfillment of required methods
};

/** A type definition. */
class TypeDefn : public Defn {
public:
  TypeDefn(Kind kind, const source::Location& location, const StringRef& name, Member* definedIn)
    : Defn(kind, location, name, definedIn)
    , _memberScope(new scope::StandardScope(scope::SymbolScope::INSTANCE, this))
    , _inheritedMemberScope(new scope::InheritedScope(_memberScope.get(), this))
    , _typeParamScope(new scope::StandardScope(scope::SymbolScope::DEFAULT, this))
  {
  }

  ~TypeDefn();

  /** The type defined by this type definition. */
  Type* type() const { return _type; }
  void setType(Type* type) { _type = type; }

  /** List of all members of this type. */
  std::vector<Member*>& members() { return _members; }
  const std::vector<Member*>& members() const { return _members; }

  /** List of function parameters. */
  std::vector<TypeParameter*>& typeParams() { return _typeParams; }
  const std::vector<TypeParameter*>& typeParams() const { return _typeParams; }

  /** Scope containing all of the members of this type. */
  scope::StandardScope* memberScope() const { return _memberScope.get(); }

  /** Scope containing all of the members of this type, including inherited members (but
      only non-shadowed inherited members. */
  scope::InheritedScope* inheritedMemberScope() const { return _inheritedMemberScope.get(); }

  /** Scope containing all of the type parameters of this type. */
  scope::StandardScope* typeParamScope() const { return _typeParamScope.get(); }

  void format(std::ostream& out) const;

private:
  Type* _type;
  std::vector<Member*> _members;
  std::vector<TypeParameter*> _typeParams;
  std::auto_ptr<scope::StandardScope> _memberScope;
  std::auto_ptr<scope::InheritedScope> _inheritedMemberScope;
  std::auto_ptr<scope::StandardScope> _typeParamScope;

//   # List of friend declarations for thibs class
//   friends: list[Member] = 3;
//
//   # Set if this is an intrinsic type
//   intrinsic: expr.Intrinsic = 14;
//
//   # If this is an attribute type, some attribute properties.
//   attribute: AttributeInfo = 15;
};

/** A type parameter of a function or composite type. */
class TypeParameter : public Defn {
public:
  TypeParameter(const source::Location& location, const StringRef& name, Member* definedIn = nullptr)
    : Defn(Kind::TYPE_PARAM, location, name, definedIn)
    , _valueType(nullptr)
    , _typeVar(nullptr)
    , _defaultType(nullptr)
    , _variadic(false)
  {}

  /** If this type parameter represents a constant value rather than a type, then this is
      the type of that constant value. Otherwise, this is nullptr. */
  Type* valueType() const { return _valueType; }
  void setValueType(Type* type) { _valueType = type; }

  /** The type variable for this parameter. This is used whenever there is a need to
      refer to this parameter within a type expression. */
  TypeVariable* typeVar() const { return _typeVar; }
  void setTypeVar(TypeVariable* type) { _typeVar = type; }

  /** If this type pararameter holds a type expression, this is the default value for
      the type parameter. nullptr if no default has been specified. */
  Type* defaultType() const { return _defaultType; }
  void setDefaultType(Type* type) { _defaultType = type; }

  /** Indicates this type parameter accepts a list of types. */
  bool isVariadic() const { return _variadic; }
  void setVariadic(bool variadic) { _variadic = variadic; }

  /** List of subtype constraints on this type parameter. The type bound to this parameter
      must be a subtype of every type listed. */
  const TypeArray& subtypeConstraints() const { return _subtypeConstraints; }
  void setSubtypeConstraints(const TypeArray& types) { _subtypeConstraints = types; }

  void format(std::ostream& out) const;

private:
  Type* _valueType;
  TypeVariable* _typeVar;
  Type* _defaultType;
  bool _variadic;
  TypeArray _subtypeConstraints;
};

class ValueDefn : public Defn {
public:
  ValueDefn(
      Kind kind, const source::Location& location, const StringRef& name, Member* definedIn = nullptr)
    : Defn(kind, location, name, definedIn)
    , _type(nullptr)
    , _init(nullptr)
    , _fieldIndex(0)
  {}

  /** Type of this value. */
  Type* type() const { return _type; }
  void setType(Type* type) { _type = type; }

  /** Initialization expression or default value. */
  Expr* init() const { return _init; }
  void setInit(Expr* init) { _init = init; }

  /** Field index for fields within an object. */
  int32_t fieldIndex() const { return _fieldIndex; }
  void setFieldIndex(int32_t index) { _fieldIndex = index; }

  void format(std::ostream& out) const;

private:
  Type* _type;
  Expr* _init;
  int32_t _fieldIndex;
};

// struct Var(ValueDefn) = MemberKind.VAR {}
// struct Let(ValueDefn) = MemberKind.LET {}
// struct EnumValue(ValueDefn) = MemberKind.ENUM_VAL {
//   ordinal:i32 = 1;
// }
// struct TupleMember(ValueDefn) = MemberKind.TUPLE_MEMBER {}

class Parameter : public ValueDefn {
public:
  Parameter(const source::Location& location, const StringRef& name, Member* definedIn = nullptr)
    : ValueDefn(Kind::PARAM, location, name, definedIn)
    , _internalType(nullptr)
    , _keywordOnly(false)
    , _selfParam(false)
    , _classParam(false)
    , _variadic(false)
    , _expansion(false)
  {}

  /** Type of this param within the body of the function, which may be different than the
      declared type of the parameter. Example: variadic parameters become arrays. */
  Type* internalType() const { return _internalType; }
  void setInternalType(Type* type) { _internalType = type; }

  /** Indicates a keyword-only parameter. */
  bool isKeywordOnly() const { return _keywordOnly; }
  void setKeywordOnly(bool keywordOnly) { _keywordOnly = keywordOnly; }

  /** Indicates this parameter accepts a list of values. */
  bool isVariadic() const { return _variadic; }
  void setVariadic(bool variadic) { _variadic = variadic; }

  /** Indicates this parameter's type is actually an expansion of a variadic template param. */
  bool isExpansion() const { return _expansion; }
  void setExpansion(bool expansion) { _expansion = expansion; }

  /** Indicates that this is the special 'self' parameter. */
  bool isSelfParam() const { return _selfParam; }
  void setSelfParam(bool selfParam) { _selfParam = selfParam; }

  /** Indicates that this is the special 'class' parameter. */
  bool isClassParam() const { return _classParam; }
  void setClassParam(bool classParam) { _classParam = classParam; }

  void format(std::ostream& out) const;

private:
  Type* _internalType;
  bool _keywordOnly;
  bool _selfParam;
  bool _classParam;
//   bool _mutable;
  bool _variadic;
  bool _expansion;

//  explicit: bool = 3;           # No type conversion - type must be exact
};

/** A method or global function. */
class Function : public Defn {
public:
  Function(const source::Location& location, const StringRef& name, Member* definedIn = nullptr)
    : Defn(Kind::FUNCTION, location, name, definedIn)
    , _type(nullptr)
    , _paramScope(new scope::StandardScope(scope::SymbolScope::DEFAULT, this))
    , _typeParamScope(new scope::StandardScope(scope::SymbolScope::DEFAULT, this))
    , _body(nullptr)
    , _constructor(false)
    , _requirement(false)
    , _native(false)
    , _methodIndex(0)
  {}

  ~Function();

  /** Type of this function. */
  FunctionType* type() const { return _type; }
  void setType(FunctionType* type) { _type = type; }

  /** List of function parameters. */
  std::vector<Parameter*>& params() { return _params; }
  const std::vector<Parameter*>& params() const { return _params; }

  /** Scope containing all of the parameters of this function. */
  scope::StandardScope* paramScope() const { return _paramScope.get(); }

  /** List of function type parameters. */
  std::vector<TypeParameter*>& typeParams() { return _typeParams; }
  const std::vector<TypeParameter*>& typeParams() const { return _typeParams; }

  /** Scope containing all of the type parameters of this function. */
  scope::StandardScope* typeParamScope() const { return _typeParamScope.get(); }

  /** List of all local variables and definitions. */
  std::vector<Defn*>& localDefns() { return _localDefns; }
  const std::vector<Defn*>& localDefns() const { return _localDefns; }

  /** The function body. nullptr if no body has been declared. */
  Expr* body() const { return _body; }
  void setBody(Expr* body) { _body = body; }

  /** True if this function is a constructor. */
  bool isConstructor() const { return _constructor; }
  void setConstructor(bool ctor) { _constructor = ctor; }

  /** True if this function is a requirement. */
  bool isRequirement() const { return _requirement; }
  void setRequirement(bool req) { _requirement = req; }

  /** True if this is a native function. */
  bool isNative() const { return _native; }
  void setNative(bool native) { _native = native; }

  /** Method index for this function in a dispatch table. */
  int32_t methodIndex() const { return _methodIndex; }
  void setNative(int32_t index) { _methodIndex = index; }

  void format(std::ostream& out) const;

private:
  FunctionType* _type;
  std::vector<Parameter*> _params;
  std::vector<TypeParameter*> _typeParams;
  std::auto_ptr<scope::StandardScope> _paramScope;
  std::auto_ptr<scope::StandardScope> _typeParamScope;
  std::vector<Defn*> _localDefns;
  Expr* _body;
  bool _constructor;
  bool _requirement;
  bool _native;
  int32_t _methodIndex;
  //tempVarCount: i32 = 8;        # Number of temporary variables used in this function.
  //linkageName: string = 10;     # If present, indicates the symbolic linkage name of this function
  //selfType : type.Type = 11 [nullable]; # Type of the 'self' argument (not part of the function type).
  //evaluable : bool = 12;        # If true, function can be evaluated at compile time.
};

/** A property definition. */
class Property : public Defn {
public:
  Property(const source::Location& location, const StringRef& name, Member* definedIn = nullptr)
    : Defn(Kind::FUNCTION, location, name, definedIn)
    , _type(nullptr)
    , _paramScope(new scope::StandardScope(scope::SymbolScope::DEFAULT, this))
    , _typeParamScope(new scope::StandardScope(scope::SymbolScope::DEFAULT, this))
    , _getter(nullptr)
    , _setter(nullptr)
  {}

  ~Property();

  /** The type of the property value. */
  Type* type() const { return _type; }
  void setType(Type* type) { _type = type; }

  /** List of function parameters. */
  std::vector<Parameter*>& params() { return _params; }
  const std::vector<Parameter*>& params() const { return _params; }

  /** Scope containing all of the parameters of this function. */
  scope::StandardScope* paramScope() const { return _paramScope.get(); }

  /** List of function type parameters. */
  std::vector<TypeParameter*>& typeParams() { return _typeParams; }
  const std::vector<TypeParameter*>& typeParams() const { return _typeParams; }

  /** Scope containing all of the type parameters of this function. */
  scope::StandardScope* typeParamScope() const { return _typeParamScope.get(); }

  /** The 'get' accessor function for this property. */
  Function* getter() const { return _getter; }
  void setGetter(Function* fn) { _getter = fn; }

  /** The 'set' accessor function for this property. */
  Function* setter() const { return _setter; }
  void setSetter(Function* fn) { _setter = fn; }

  void format(std::ostream& out) const;

private:
  Type* _type;
  std::vector<Parameter*> _params;
  std::vector<TypeParameter*> _typeParams;
  std::auto_ptr<scope::StandardScope> _paramScope;
  std::auto_ptr<scope::StandardScope> _typeParamScope;
  Function* _getter;
  Function* _setter;
//  selfType : type.Type = 7 [nullable]; # Type of the 'self' argument (not part of the function type).
};

class SpecializedMember : public Member {
public:
  SpecializedMember(Member* generic, const Env& env)
    : Member(Kind::SPECIALIZED, generic->name(), generic->definedIn())
    , _generic(generic)
    , _env(env)
  {}

  /** The generic version of this specialized member. */
  Member* generic() const { return _generic; }
  void setGeneric(Member* t) { _generic = t; }

  /** The set of variable bindings for the generic type. */
  Env& env() { return _env; }
  const Env& env() const { return _env; }

private:
  Member* _generic;
  Env _env;
};

// struct InstantiatedMember(Member) = MemberKind.INSTANTIATED {
//   # Generic version of the member
//   base: defn.Member = 1;
//
//   # Environment of template variable bindings
//   env: type.Env = 2;
// }


}}

#endif
