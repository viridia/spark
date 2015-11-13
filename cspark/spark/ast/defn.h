// ============================================================================
// ast/defn.h: AST nodes representing definitions.
// ============================================================================

#ifndef SPARK_AST_DEFN_H
#define SPARK_AST_DEFN_H 1

#ifndef SPARK_AST_NODE_H
  #include "spark/ast/node.h"
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

namespace spark {
namespace common {
class DocComment;
}
namespace ast {
using spark::collections::StringRef;

/** Base for all definitions. */
class Defn : public Node {
public:
  Defn(Kind kind, const Location& location, const StringRef& name)
    : Node(kind, location)
    , _name(name)
    , _docComment(nullptr)
    , _private(false)
    , _protected(false)
    , _static(false)
    , _override(false)
    , _undef(false)
    , _final(false)
    , _abstract(false)
  {}

  /** The name of this definition. */
  StringRef name() const { return _name; }

  /** The list of members of this definition. */
  const NodeList& members() const { return _members; }
  void setMembers(const NodeList& members) { _members = members; }

  /** The list of attributes of this definition. */
  const NodeList& attributes() const { return _attributes; }
  void setAttributes(const NodeList& attributes) { _attributes = attributes; }

  /** The list of type parameters of this definition. */
  const NodeList& typeParams() const { return _typeParams; }
  void setTypeParams(const NodeList& typeParams) { _typeParams = typeParams; }

  /** The list of requirements of this definition. */
  const NodeList& requires() const { return _requires; }
  void setRequires(const NodeList& requires) { _requires = requires; }

  /** Doc comment associated with this definition. */
  const common::DocComment* docComment() const { return _docComment; }
  void setDocComment(common::DocComment* doc) { _docComment = doc; }

  /** Whether this definition has the 'private' modifier. */
  bool isPrivate() const { return _private; }
  void setPrivate(bool value) { _private = value; }

  /** Whether this definition has the 'protected' modifier. */
  bool isProtected() const { return _protected; }
  void setProtected(bool value) { _protected = value; }

  /** Whether this definition has the 'static' modifier. */
  bool isStatic() const { return _static; }
  void setStatic(bool value) { _static = value; }

  /** Whether this definition has the 'override' modifier. */
  bool isOverride() const { return _override; }
  void setOverride(bool value) { _override = value; }

  /** Whether this definition has the 'undef' modifier. */
  bool isUndef() const { return _undef; }
  void setUndef(bool value) { _undef = value; }

  /** Whether this definition has the 'final' modifier. */
  bool isFinal() const { return _final; }
  void setFinal(bool value) { _final = value; }

  /** Whether this definition has the 'abstract' modifier. */
  bool isAbstract() const { return _abstract; }
  void setAbstract(bool value) { _abstract = value; }

private:
  StringRef _name;
  NodeList _members;
  NodeList _attributes;
  NodeList _typeParams;
  NodeList _requires;
  common::DocComment* _docComment;

  bool _private;
  bool _protected;
  bool _static;
  bool _override;
  bool _undef;
  bool _final;
  bool _abstract;
};

class TypeDefn : public Defn {
public:
  TypeDefn(Kind kind, const Location& location, const StringRef& name)
    : Defn(kind, location, name)
  {}

  /** The list of base types for this type. */
  const NodeList& bases() const { return _bases; }
  void setBases(const NodeList& bases) { _bases = bases; }

  /** Friend declarations for this type. */
  const NodeList& friends() const { return _friends; }
  void setFriends(const NodeList& friends) { _friends = friends; }
private:
  NodeList _bases;
  NodeList _friends;
};

/** Base for all vars, lets, enum values and parameters. */
class ValueDefn : public Defn {
public:
  ValueDefn(Kind kind, const Location& location, const StringRef& name)
    : Defn(kind, location, name)
    , _type(nullptr)
    , _init(nullptr)
  {}

  const Node* type() const { return _type; }
  void setType(Node* type) { _type = type; }

  const Node* init() const { return _init; }
  void setInit(Node* init) { _init = init; }
private:
  const Node* _type;
  const Node* _init;
};

// class Var(ValueDefn): pass
// class Let(ValueDefn): pass

class EnumValue : public ValueDefn {
public:
  EnumValue(const Location& location, const StringRef& name)
    : ValueDefn(Kind::ENUM_VALUE, location, name)
  {}

  int32_t ordinal() const { return _ordinal; }
  void setOrdinal(int32_t ordinal) { _ordinal = ordinal; }
private:
  int32_t _ordinal;
};

class Parameter : public ValueDefn {
public:
  Parameter(const Location& location, const StringRef& name)
    : ValueDefn(Kind::PARAMETER, location, name)
  {}

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
private:
  bool _keywordOnly;
  bool _selfParam;
  bool _classParam;
//   bool _mutable;
  bool _variadic;
  bool _expansion;
};

// class Parameter(ValueDefn):
//   explicit = defscalar(bool)      # No type conversion - type must be exact
//   multi = defscalar(bool)         # Represents multiple params expanded from a tuple type

class TypeParameter : public Defn {
public:
  TypeParameter(const Location& location, const StringRef& name)
    : Defn(Kind::TYPE_PARAMETER, location, name)
    , _type(nullptr)
    , _init(nullptr)
    , _variadic(false)
  {}

  /** Type of this defn. */
  const Node* type() const { return _type; }
  void setType(Node* type) { _type = type; }

  const Node* init() const { return _init; }
  void setInit(Node* init) { _init = init; }

  /** Whether this type parameter is variadic. */
  bool isVariadic() const { return _variadic; }
  void setVariadic(bool value) { _variadic = value; }

  /** The list of subtype constraints for this type parameter. */
  const NodeList& subtypeConstraints() const { return _subtypeConstraints; }
  void setSubtypeConstraints(const NodeList& constraints) { _subtypeConstraints = constraints; }
private:
  const Node* _type;
  const Node* _init;
  bool _variadic;
  NodeList _subtypeConstraints;
};

class Function : public Defn {
public:
  Function(const Location& location, const StringRef& name)
    : Defn(Kind::FUNCTION, location, name)
    , _returnType(nullptr)
    , _body(nullptr)
    , _constructor(false)
    , _requirement(false)
    , _native(false)
    , _selfType(nullptr)
  {}

  /** Type of this defn. */
  const Node* returnType() const { return _returnType; }
  void setReturnType(Node* type) { _returnType = type; }

  /** The list of parametgers for this property. */
  const NodeList& params() const { return _params; }
  void setParams(const NodeList& params) { _params = params; }

  /** Function body. */
  const Node* body() const { return _body; }
  void setBody(Node* body) { _body = body; }

  /** Whether this function is a constructor. */
  bool isConstructor() const { return _constructor; }
  void setConstructor(bool constructor) { _constructor = constructor; }

  /** Whether this function is a requirement. */
  bool isRequirement() const { return _requirement; }
  void setRequirement(bool requirement) { _requirement = requirement; }

  /** Whether this function is native. */
  bool isNative() const { return _native; }
  void setNative(bool native) { _native = native; }

  /** Type of implicit 'self' param. */
  const Node* selfType() const { return _selfType; }
  void setSelfType(Node* selfType) { _selfType = selfType; }
private:
  const Node* _returnType;
  NodeList _params;
  const Node* _body;
  bool _constructor;
  bool _requirement;
  bool _native;
  const Node* _selfType;
};

class Property : public Defn {
public:
  Property(const Location& location, const StringRef& name)
    : Defn(Kind::PROPERTY, location, name)
    , _type(nullptr)
    , _getter(nullptr)
    , _setter(nullptr)
    , _selfType(nullptr)
  {}

  /** Type of this defn. */
  const Node* type() const { return _type; }
  void setType(Node* type) { _type = type; }

  /** The list of parametgers for this property. */
  const NodeList& params() const { return _params; }
  void setParams(const NodeList& params) { _params = params; }

  /** Getter function. */
  const Function* getter() const { return _getter; }
  void setGetter(Function* getter) { _getter = getter; }

  /** Setter function. */
  const Function* setter() const { return _setter; }
  void setSetter(Function* setter) { _setter = setter; }

  /** Type of implicit 'self' param. */
  const Node* selfType() const { return _selfType; }
  void setSelfType(Node* selfType) { _selfType = selfType; }
private:
  const Node* _type;
  NodeList _params;
  const Function* _getter;
  const Function* _setter;
  const Node* _selfType;
};

}}

#endif
