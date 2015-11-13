// ============================================================================
// Parser implementation.
// ============================================================================

#include "spark/config.h"
#include "spark/ast/builder.h"
#include "spark/ast/defn.h"
#include "spark/ast/node.h"
#include "spark/ast/ident.h"
#include "spark/ast/literal.h"
#include "spark/ast/module.h"
#include "spark/ast/oper.h"
#include "spark/error/reporter.h"
#include "spark/parse/parser.h"

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace parse {
using spark::ast::Defn;
using spark::ast::Node;
using spark::ast::Kind;
using spark::ast::Module;
using spark::collections::StringRef;

enum precedence {
  PREC_COMMA = 0,
  PREC_FAT_ARROW,
  PREC_RETURNS,
  PREC_ASSIGN,
  PREC_RANGE,   // And colon
  PREC_LOGICAL_AND, // 'and' and 'or'
  PREC_LOGICAL_OR, // 'and' and 'or'
  PREC_NOT,     // 'not'
  PREC_IN,      // 'in'
  PREC_IS_AS,   // 'is' and 'as'
  PREC_RELATIONAL,  // comparison
  PREC_BIT_OR,  // bitwise-or
  PREC_BIT_XOR, // bitwise-xor
  PREC_BIT_AND, // bitwise-and
  PREC_SHIFT,   // shift operators
  PREC_ADD_SUB, // binary plus and minus
  PREC_MUL_DIV, // multiply and divide
};

Parser::Parser(Reporter& reporter, ProgramSource* source, support::Arena& arena)
  : _reporter(reporter)
  , _source(source)
  , _arena(arena)
  , _lexer(source)
  , _recovering(false)
{
  _token = _lexer.next();
}

void Parser::next() {
  _token = _lexer.next();
}

bool Parser::match(TokenType tok) {
  if (_token == tok) {
    next();
    return true;
  }
  return false;
}

// Module

ast::Module* Parser::module() {
  ast::NodeListBuilder imports(_arena);
  ast::NodeListBuilder members(_arena);

  Module* mod = new (_arena) Module(location());
  while (match(TOKEN_IMPORT)) {
    if (_token != TOKEN_ID) {
      _reporter.error(location()) << "Module name expected.";
      return nullptr;
    }
    Node* path = dottedIdent();
    assert(path != nullptr);
    StringRef alias;
    if (match(TOKEN_AS)) {
      if (_token != TOKEN_ID) {
        _reporter.error(location()) << "Identifier expected.";
        return nullptr;
      }
      alias = copyOf(tokenValue());
    }

    if (!match(TOKEN_SEMI)) {
      _reporter.error(location()) << "Semicolon expected.";
    }

    imports.append(new (_arena) ast::Import(path->location(), path, alias));
  }

  while (_token != TOKEN_END) {
    if (!declaration(members)) {
      return nullptr;
    }
  }

  mod->setImports(imports.build());
  mod->setMembers(members.build());
  return mod;
}

// Declaration

bool Parser::declaration(ast::NodeListBuilder& decls, bool isProtected, bool isPrivate) {
  ast::NodeListBuilder attributes(_arena);
  while (Node* attr = attribute()) {
    attributes.append(attr);
  }

  bool explicitVisibility = false;
  if (match(TOKEN_PRIVATE)) {
    if (isPrivate || isProtected) {
      _reporter.error(location()) << "Visibility already specified.";
    }
    isPrivate = explicitVisibility = true;
  } else if (match(TOKEN_PROTECTED)) {
    if (isPrivate || isProtected) {
      _reporter.error(location()) << "Visibility already specified.";
    }
    isProtected = explicitVisibility = true;
  }

  if (explicitVisibility) {
    Location loc = location();
    if (match(TOKEN_LBRACE)) {
      while (!match(TOKEN_RBRACE)) {
        if (_token == TOKEN_END) {
          _reporter.error(loc) << "Visibility block not closed.";
          return false;
        }
        if (!declaration(decls, isProtected, isPrivate)) {
          return false;
        }
      }
      return true;
    }
  }

  bool isAbstract = false;
  bool isFinal = false;
  bool isStatic = false;
  for (;;) {
    if (match(TOKEN_ABSTRACT)) {
      if (isAbstract) {
        _reporter.error(location()) << "'abstract' already specified.";
      }
      isAbstract = true;
    } else if (match(TOKEN_FINAL)) {
      if (isFinal) {
        _reporter.error(location()) << "'final' already specified.";
      }
      isFinal = true;
    } else if (match(TOKEN_STATIC)) {
      if (isStatic) {
        _reporter.error(location()) << "'static' already specified.";
      }
      isStatic = true;
    } else {
      break;
    }
  }

  Defn* d = memberDef();
  if (d != nullptr) {
    d->setPrivate(isPrivate);
    d->setProtected(isProtected);
    d->setAbstract(isAbstract);
    d->setFinal(isFinal);
    d->setStatic(isStatic);
    decls.append(d);
    return true;
  }

  _reporter.error(location()) << "declaration expected.";
  return false;
}

Defn* Parser::memberDef() {
  switch (_token) {
    case TOKEN_VAR:
    case TOKEN_LET:
      return varOrLetDefn();
    case TOKEN_CLASS:
    case TOKEN_STRUCT:
    case TOKEN_INTERFACE:
    case TOKEN_EXTEND:
    case TOKEN_OBJECT:
      return compositeTypeDef();
    case TOKEN_ENUM:
      return enumTypeDef();
    case TOKEN_DEF:
    case TOKEN_UNDEF:
    case TOKEN_OVERRIDE:
      return methodDef();
      break;
    default:
      _reporter.error(location()) << "Declaration expected.";
      break;
  }
  assert(false);
}

#if 0
  def p_local_def(self, p):
    '''local_def : composite_type_def
                 | enum_type_def
                 | method_def
                 | prop_def
                 | var_def'''
    p[0] = p[1]
#endif

// Composite types

Defn* Parser::compositeTypeDef() {
  Kind kind;
  switch (_token) {
    case TOKEN_CLASS:       kind = Kind::CLASS_DEFN; break;
    case TOKEN_STRUCT:      kind = Kind::STRUCT_DEFN; break;
    case TOKEN_INTERFACE:   kind = Kind::INTERFACE_DEFN; break;
    case TOKEN_EXTEND:      kind = Kind::EXTEND_DEFN; break;
    case TOKEN_OBJECT:      kind = Kind::OBJECT_DEFN; break;
    default:
      assert(false);
      break;
  }
  next();

  if (_token != TOKEN_ID) {
    _reporter.error(location()) << "Type name expected.";
    _recovering = true;
  }
  StringRef name = copyOf(tokenValue());
  Location loc = location();
  next();

  ast::TypeDefn* d = new (_arena) ast::TypeDefn(kind, loc, name);

  // Template parameters
  ast::NodeListBuilder templateParams(_arena);
  templateParamList(templateParams);
  d->setTypeParams(templateParams.build());

  // Supertype list
  if (match(TOKEN_COLON)) {
    ast::NodeListBuilder baseTypes(_arena);
    for (;;) {
      Node* base = typeExpression();
      if (base == nullptr) {
        skipUntil({TOKEN_LBRACE});
        return d;
      }
      baseTypes.append(base);
      if (!match(TOKEN_COMMA)) {
        break;
      }
    }
    d->setBases(baseTypes.build());
  }

  // Type constraints
  ast::NodeListBuilder requires(_arena);
  requirements(requires);
  d->setRequires(requires.build());

  // Body
  if (!match(TOKEN_LBRACE)) {
    expected("{");
    skipOverDefn();
  } else {
    if (!classBody(d)) {
      return nullptr;
    }
  }

  return d;
}

bool Parser::classBody(ast::TypeDefn* d) {
  ast::NodeListBuilder members(_arena);
  ast::NodeListBuilder friends(_arena);
  while (_token != TOKEN_END && !match(TOKEN_RBRACE)) {
    if (!classMember(members, friends)) {
      return false;
    }
  }
  d->setMembers(members.build());
  d->setFriends(friends.build());
  return true;
}

bool Parser::classMember(ast::NodeListBuilder &members, ast::NodeListBuilder &friends) {
  if (match(TOKEN_FRIEND)) {
    Node* friendDecl = dottedIdent();
    if (friendDecl == nullptr) {
      expected("class or function name");
      skipUntil({TOKEN_SEMI, TOKEN_RBRACE});
    }
    if (!match(TOKEN_SEMI)) {
      expected("';'");
    }
    friends.append(friendDecl);
  } else if (!declaration(members)) {
    return false;
  }
  return true;
}

// Enumeration types

// Definitions allowed inside of an enumeration.
static const std::unordered_set<TokenType> ENUM_MEMBERS = {
  TOKEN_DEF, TOKEN_OVERRIDE, TOKEN_UNDEF, TOKEN_VAR, TOKEN_LET,
  TOKEN_CLASS, TOKEN_STRUCT, TOKEN_INTERFACE, TOKEN_EXTEND, TOKEN_ENUM
};

Defn* Parser::enumTypeDef() {
  next();
  if (_token != TOKEN_ID) {
    _reporter.error(location()) << "Type name expected.";
    _recovering = true;
  }
  StringRef name = copyOf(tokenValue());
  Location loc = location();
  next();

  ast::TypeDefn* d = new (_arena) ast::TypeDefn(Kind::ENUM_DEFN, loc, name);

  // Supertype list
  if (match(TOKEN_COLON)) {
    ast::NodeListBuilder baseTypes(_arena);
    for (;;) {
      Node* base = typeExpression();
      if (base == nullptr) {
        skipUntil({TOKEN_LBRACE});
        return d;
      }
      baseTypes.append(base);
      if (!match(TOKEN_COMMA)) {
        break;
      }
    }
    d->setBases(baseTypes.build());
  }

  // Body
  if (!match(TOKEN_LBRACE)) {
    expected("{");
    skipOverDefn();
  } else {
    ast::NodeListBuilder members(_arena);
    ast::NodeListBuilder friends(_arena);
    bool defnMode = false;
    while (!match(TOKEN_RBRACE)) {
      if (_token == TOKEN_END) {
        _reporter.error(loc) << "Incomplete enum.";
        return nullptr;
      }
      if (defnMode || ENUM_MEMBERS.find(_token) != ENUM_MEMBERS.end()) {
        defnMode = true;
        if (!classMember(members, friends)) {
          return nullptr;
        }
      } else {
        if (!enumMember(members)) {
          return nullptr;
        }
      }

      if (match(TOKEN_COMMA)) {
        continue;
      } else if (match(TOKEN_SEMI)) {
        defnMode = true;
      }
    }

    d->setMembers(members.build());
  }

  return d;
}

bool Parser::enumMember(ast::NodeListBuilder &members) {
  if (_token != TOKEN_ID) {
    _reporter.error(location()) << "Identifier expected.";
    _recovering = true;
  }

  StringRef name = copyOf(tokenValue());
  Location loc = location();
  next();

  ast::EnumValue* ev = new (_arena) ast::EnumValue(loc, name);

  // Initializer
  if (match(TOKEN_ASSIGN)) {
    Node* init = expression();
    if (init != nullptr) {
      ev->setInit(init);
    }
  } else if (match(TOKEN_LPAREN)) {
    ast::NodeListBuilder args(_arena);
    Location callLoc = loc;
    if (!callingArgs(args, callLoc)) {
      return false;
    }
    ev->setInit(new (_arena) ast::Oper(Kind::CALL, callLoc, args.build()));
  }
  members.append(ev);
  return true;
}

// Method

Defn* Parser::methodDef() {
  bool isUndef = false;
  bool isOverride = false;
  if (match(TOKEN_UNDEF)) {
    isUndef = true;
  } else if (match(TOKEN_OVERRIDE)) {
    isOverride = true;
  } else if (!match(TOKEN_DEF)) {
    assert(false && "Invalid def token.");
  }

  // Method name (may be empty).
  Location loc = location();
  StringRef name = methodName();

  // Template parameters
  ast::NodeListBuilder templateParams(_arena);
  templateParamList(templateParams);

  ast::NodeListBuilder params(_arena);
  if (_token == TOKEN_LPAREN) {
    if (!paramList(params)) {
      skipOverDefn();
      return nullptr;
    }
  } else if (name.empty()) {
    expected("function parameter list");
    skipOverDefn();
    return nullptr;
  }

  if (name.empty()) {
    name = "()";
  }

  Node* returnType = nullptr;
  if (match(TOKEN_RETURNS)) {
    returnType = typeExpression();
    if (returnType == nullptr) {
      skipOverDefn();
      return nullptr;
    }
  }

  if (returnType != nullptr ||
      _token == TOKEN_WHERE ||
      _token == TOKEN_LBRACE ||
      _token == TOKEN_FAT_ARROW ||
      _token == TOKEN_SEMI) {
    // Method
    ast::Function* fn = new (_arena) ast::Function(loc, name);
    fn->setTypeParams(templateParams.build());
    fn->setParams(params.build());
    fn->setOverride(isOverride);
    fn->setUndef(isUndef);
    fn->setReturnType(returnType);

    // Type constraints
    ast::NodeListBuilder requires(_arena);
    requirements(requires);
    fn->setRequires(requires.build());

    Node* body = methodBody();
    if (body == &Node::ERROR) {
      skipOverDefn();
      return nullptr;
    } else if (body != nullptr) {
      fn->setBody(body);
    }
    return fn;
  } else if (match(TOKEN_COLON)) {
    // Property
    Node* propType = typeExpression();
    if (propType == nullptr) {
      skipOverDefn();
      return nullptr;
    }

    ast::Property* prop = new (_arena) ast::Property(loc, name);
    prop->setType(propType);
    prop->setTypeParams(templateParams.build());
    prop->setParams(params.build());
    prop->setOverride(isOverride);
    prop->setUndef(isUndef);

    // Type constraints
    ast::NodeListBuilder requires(_arena);
    requirements(requires);
    prop->setRequires(requires.build());

    // Accessors
    if (match(TOKEN_SEMI)) {
      return prop;
    }
    if (!match(TOKEN_LBRACE)) {
      expected("{");
      skipOverDefn();
      return nullptr;
    }

    for (;;) {
      if (match(TOKEN_RBRACE)) {
        break;
      }

      if (_token == TOKEN_ID) {
        loc = location();
        name = copyOf(tokenValue());
        next();

        ast::NodeListBuilder accessorParams(_arena);
        if (match(TOKEN_LPAREN)) {
          if (!paramList(accessorParams)) {
            skipOverDefn();
            continue;
          }
        }

        ast::Function* accessor = new (_arena) ast::Function(loc, name);
        accessor->setParams(accessorParams.build());

        if (name == "get") {
          if (prop->getter() != nullptr) {
            _reporter.error(loc) << "Property 'get' method already defined.";
          }
          prop->setGetter(accessor);
        } else if (name == "set") {
          if (prop->setter() != nullptr) {
            _reporter.error(loc) << "Property 'get' method already defined.";
          }
          prop->setSetter(accessor);
        } else {
          _reporter.error(loc) << "Invalid accessor name: " << name;
        }

        Node* body = methodBody();
        if (body == &Node::ERROR) {
          skipOverDefn();
          continue;
        } else if (body != nullptr) {
          accessor->setBody(body);
        }
      }
    }

    return prop;
  } else {
    _reporter.error(location()) << "return type or function body expected.";
    skipOverDefn();
    return nullptr;
  }
}

StringRef Parser::methodName() {
  StringRef methodName;
  switch (_token) {
    case TOKEN_ID:      methodName = copyOf(tokenValue()); break;
    case TOKEN_VBAR:    methodName = "|"; break;
    case TOKEN_CARET:   methodName = "^"; break;
    case TOKEN_AMP:     methodName = "&"; break;
    case TOKEN_PLUS:    methodName = "+"; break;
    case TOKEN_MINUS:   methodName = "-"; break;
    case TOKEN_MUL:     methodName = "*"; break;
    case TOKEN_DIV:     methodName = "/"; break;
    case TOKEN_MOD:     methodName = "%"; break;
    case TOKEN_LSHIFT:  methodName = "<<"; break;
    case TOKEN_RSHIFT:  methodName = ">>"; break;
    case TOKEN_IN:      methodName = "in"; break;
    case TOKEN_EQ:      methodName = "=="; break;
    case TOKEN_NE:      methodName = "!="; break;
    case TOKEN_LT:      methodName = "<"; break;
    case TOKEN_GT:      methodName = ">"; break;
    case TOKEN_LE:      methodName = ">="; break;
    case TOKEN_GE:      methodName = "<"; break;
    default:
      return StringRef();
  }
  next();
  return methodName;
}

Node* Parser::methodBody() {
  if (match(TOKEN_SEMI)) {
    return &Node::ABSENT;
  } else if (_token == TOKEN_LBRACE) {
    Node* body = block();
    if (body != nullptr) {
      return body;
    }
  } else if (match(TOKEN_FAT_ARROW)) {
    Node* body = exprList();
    if (body != nullptr) {
      if (!match(TOKEN_SEMI)) {
        expected("';'");
      }
      return body;
    }
  } else {
    _reporter.error(location()) << "Method body expected.";
  }
  return &Node::ERROR;
}

// Requirements

bool Parser::requirements(ast::NodeListBuilder& out) {
  if (match(TOKEN_WHERE)) {
    for (;;) {
      Node* req = requirement();
      if (req == nullptr) {
        skipUntil({TOKEN_LBRACE});
        return false;
      }
      out.append(req);
      if (!match(TOKEN_AND)) {
        break;
      }
    }
  }
  return true;
}

Node* Parser::requirement() {
  Location loc = location();
  if (match(TOKEN_INC)) {
    Node* operand = typeExpression();
    if (operand == nullptr) {
      return nullptr;
    }
    return new (_arena) ast::UnaryOp(Kind::PRE_INC, loc, operand);
  } else if (match(TOKEN_INC)) {
    Node* operand = typeExpression();
    if (operand == nullptr) {
      return nullptr;
    }
    return new (_arena) ast::UnaryOp(Kind::PRE_DEC, loc, operand);
  } else if (match(TOKEN_MINUS)) {
    Node* operand = typeExpression();
    if (operand == nullptr) {
      return nullptr;
    }
    return new (_arena) ast::UnaryOp(Kind::NEGATE, loc, operand);
  } else if (match(TOKEN_STATIC)) {
    Node* fn = typeExpression();
    if (fn == nullptr) {
      return nullptr;
    }
    return requireCall(Kind::CALL_REQUIRED_STATIC, fn);
  }

  Node* rqTerm = typeExpression();
  if (rqTerm == nullptr) {
    return nullptr;
  }
  switch (_token) {
    case TOKEN_PLUS:    return requireBinaryOp(Kind::ADD, rqTerm);
    case TOKEN_MINUS:   return requireBinaryOp(Kind::SUB, rqTerm);
    case TOKEN_MUL:     return requireBinaryOp(Kind::MUL, rqTerm);
    case TOKEN_DIV:     return requireBinaryOp(Kind::DIV, rqTerm);
    case TOKEN_MOD:     return requireBinaryOp(Kind::MOD, rqTerm);
    case TOKEN_VBAR:    return requireBinaryOp(Kind::BIT_OR, rqTerm);
    case TOKEN_CARET:   return requireBinaryOp(Kind::BIT_XOR, rqTerm);
    case TOKEN_AMP:     return requireBinaryOp(Kind::BIT_AND, rqTerm);
    case TOKEN_RSHIFT:  return requireBinaryOp(Kind::RSHIFT, rqTerm);
    case TOKEN_LSHIFT:  return requireBinaryOp(Kind::LSHIFT, rqTerm);
    case TOKEN_EQ:      return requireBinaryOp(Kind::EQUAL, rqTerm);
    case TOKEN_NE:      return requireBinaryOp(Kind::NOT_EQUAL, rqTerm);
    case TOKEN_LT:      return requireBinaryOp(Kind::LESS_THAN, rqTerm);
    case TOKEN_GT:      return requireBinaryOp(Kind::GREATER_THAN, rqTerm);
    case TOKEN_LE:      return requireBinaryOp(Kind::LESS_THAN_OR_EQUAL, rqTerm);
    case TOKEN_GE:      return requireBinaryOp(Kind::GREATER_THAN_OR_EQUAL, rqTerm);
    case TOKEN_TYPE_LE: return requireBinaryOp(Kind::IS_SUB_TYPE, rqTerm);
    case TOKEN_TYPE_GE: return requireBinaryOp(Kind::IS_SUPER_TYPE, rqTerm);
    case TOKEN_IN:      return requireBinaryOp(Kind::IN, rqTerm);
    case TOKEN_RANGE:   return requireBinaryOp(Kind::RANGE, rqTerm);

    case TOKEN_INC:
      next();
      return new (_arena) ast::UnaryOp(Kind::POST_INC, rqTerm->location(), rqTerm);
    case TOKEN_DEC:
      next();
      return new (_arena) ast::UnaryOp(Kind::POST_DEC, rqTerm->location(), rqTerm);

    case TOKEN_NOT: {
      next();
      if (!match(TOKEN_IN)) {
        _reporter.error(location()) <<
            "'not' must be followed by 'in' when used as a binary operator.";
        return nullptr;
      }
      return requireBinaryOp(Kind::NOT_IN, rqTerm);
    }

    case TOKEN_LPAREN: {
      return requireCall(Kind::CALL_REQUIRED, rqTerm);
    }

    default:
      return rqTerm;
  }
  assert(false);
}

Node* Parser::requireBinaryOp(Kind kind, Node* left) {
  next();
  Node* right = typeExpression();
  if (right == nullptr) {
    return nullptr;
  }
  ast::NodeListBuilder builder(_arena);
  builder.append(left);
  builder.append(right);
  return new (_arena) ast::Oper(
    kind, left->location() | right ->location(), builder.build());
}

Node* Parser::requireCall(Kind kind, Node* fn) {
  Node* fnType = functionType();
  if (fnType == nullptr) {
    return nullptr;
  }
  ast::NodeListBuilder signature(_arena);
  signature.append(fnType);
  ast::Oper *call = new (_arena) ast::Oper(kind, fn->location(), signature.build());
  call->setOp(fn);
  return call;
}

// Method and Property parameter lists

bool Parser::paramList(ast::NodeListBuilder& builder) {
  bool keywordOnly = false;
  if (match(TOKEN_LPAREN)) {
    if (match(TOKEN_RPAREN)) {
      return true;
    }

    if (match(TOKEN_SEMI)) {
      keywordOnly = true;
    }

    for (;;) {
      // Parameter name
      ast::Parameter* param = nullptr;
      if (_token == TOKEN_ID) {
        param = new (_arena) ast::Parameter(location(), copyOf(tokenValue()));
        next();
      } else if (match(TOKEN_SELF)) {
        param = new (_arena) ast::Parameter(location(), copyOf(tokenValue()));
        param->setSelfParam(true);
      } else if (match(TOKEN_CLASS)) {
        param = new (_arena) ast::Parameter(location(), copyOf(tokenValue()));
        param->setClassParam(true);
      } else {
        expected("parameter name");
        skipUntil({TOKEN_COMMA, TOKEN_RBRACE});
        if (match(TOKEN_COMMA)) {
          continue;
        } else if (match(TOKEN_RBRACE)) {
          break;
        } else {
          return false;
        }
      }

      if (param) {
        // Parameter type
        param->setKeywordOnly(keywordOnly);
        if (match(TOKEN_COLON)) {
          if (match(TOKEN_REF)) {
            _reporter.debug(location()) << "here";
            assert(false && "Implement ref param");
          }
          if (match(TOKEN_ELLIPSIS)) {
            param->setExpansion(true);
          }
          Node* paramType;
          if (param->isSelfParam() || param->isClassParam()) {
            paramType = typeTerm(true);
          } else {
            paramType = typeExpression();
          }
          if (paramType == nullptr) {
            skipUntil({TOKEN_COMMA, TOKEN_RPAREN});
          } else {
            param->setType(paramType);
          }

          if (match(TOKEN_ELLIPSIS)) {
            param->setVariadic(true);
          }
        }

        // Parameter initializer
        if (match(TOKEN_ASSIGN)) {
          Node* init = expression();
          if (init != nullptr) {
            param->setInit(init);
          }
        }

        builder.append(param);

        if (match(TOKEN_RPAREN)) {
          break;
        }

        if (_token != TOKEN_SEMI && _token != TOKEN_COMMA) {
          expected("',' or '}'");
          skipUntil({TOKEN_COMMA, TOKEN_RPAREN});
        }

        if (match(TOKEN_SEMI)) {
          keywordOnly = true;
        } else {
          next();
        }
      }
    }
  }
  return true;
}

// Variable

Defn* Parser::varOrLetDefn() {
  Kind kind;
  if (match(TOKEN_VAR)) {
    kind = Kind::VAR;
  } else if (match(TOKEN_LET)) {
    kind = Kind::LET;
  } else {
    assert(false);
  }

  ast::ValueDefn* var = varDeclList(kind);

  // Initializer
  if (match(TOKEN_ASSIGN)) {
    Node* init = exprList();
    if (init != nullptr) {
      var->setInit(init);
    }
  }

  if (!match(TOKEN_SEMI)) {
    _reporter.error(location()) << "Semicolon expected.";
    skipUntil({TOKEN_SEMI});
    next();
  }

  return var;
};

ast::ValueDefn* Parser::varDeclList(Kind kind) {
  Location loc = location();
  ast::ValueDefn* var = varDecl(kind);
  if (var == nullptr) {
    return nullptr;
  }

  // Handle multiple variables, i.e. var x, y = ...
  if (match(TOKEN_COMMA)) {
    ast::NodeListBuilder varList(_arena);
    varList.append(var);

    for (;;) {
      ast::ValueDefn* nextVar = varDecl(kind);
      if (nextVar == nullptr) {
        return nullptr;
      }
      varList.append(nextVar);
      if (!match(TOKEN_COMMA)) {
        break;
      }
    }

    var = new (_arena) ast::ValueDefn(Kind::VAR_LIST, loc, "");
    var->setMembers(varList.build());
  }

  return var;
}

ast::ValueDefn* Parser::varDecl(Kind kind) {
  if (_token != TOKEN_ID) {
    _reporter.error(location()) << "Variable name expected.";
    _recovering = true;
  }
  StringRef name = copyOf(tokenValue());
  Location loc = location();
  next();

  ast::ValueDefn* val = new (_arena) ast::ValueDefn(kind, loc, name);
  if (match(TOKEN_COLON)) {
    if (match(TOKEN_ELLIPSIS)) {
      assert(false && "Implement pre-ellipsis");
    }
    Node* valType = typeExpression();
    if (valType == nullptr) {
      skipUntil({TOKEN_SEMI});
      next();
    } else {
      val->setType(valType);
    }
  }

  return val;
}

// Declaration Modifiers

Node* Parser::attribute() {
  Location loc = location();
  if (match(TOKEN_ATSIGN)) {
    if (_token != TOKEN_ID) {
      _reporter.error(location()) << "Attribute name expected.";
      skipUntil({
        TOKEN_ATSIGN, TOKEN_CLASS, TOKEN_STRUCT, TOKEN_INTERFACE, TOKEN_ENUM,
        TOKEN_DEF, TOKEN_OVERRIDE, TOKEN_VAR, TOKEN_LET});
      return nullptr;
    }
    Node* attr = dottedIdent();
    assert(attr != nullptr);
    if (match(TOKEN_LPAREN)) {
      ast::NodeListBuilder args(_arena);
      Location callLoc = loc;
      if (!callingArgs(args, callLoc)) {
        return nullptr;
      }
      ast::Oper* call = new (_arena) ast::Oper(Kind::CALL, callLoc, args.build());
      call->setOp(attr);
      attr = call;
    }
    return attr;
  } else if (match(TOKEN_INTRINSIC)) {
    return new (_arena) ast::BuiltInAttribute(loc, ast::BuiltInAttribute::INTRINSIC);
  } else if (match(TOKEN_TRACEMETHOD)) {
    return new (_arena) ast::BuiltInAttribute(loc, ast::BuiltInAttribute::TRACEMETHOD);
  } else if (match(TOKEN_UNSAFE)) {
    return new (_arena) ast::BuiltInAttribute(loc, ast::BuiltInAttribute::UNSAFE);
  } else {
    return nullptr;
  }
}

// Template Params

void Parser::templateParamList(ast::NodeListBuilder& builder) {
  if (match(TOKEN_LBRACKET)) {
    Location loc = location();
    if (match(TOKEN_RBRACKET)) {
      _reporter.error(loc) << "Empty template parameter list.";
      return;
    }
    for (;;) {
      ast::TypeParameter* tp = templateParam();
      if (tp == nullptr) {
        skipUntil({TOKEN_RBRACKET});
        return;
      }
      builder.append(tp);
      if (match(TOKEN_RBRACKET)) {
        return;
      } else if (!match(TOKEN_COMMA)) {
        _reporter.error(loc) << "',' or ']' expected.";
      }
    }
  }
}

ast::TypeParameter* Parser::templateParam() {
  if (_token == TOKEN_ID) {
    ast::TypeParameter* tp = new (_arena) ast::TypeParameter(location(), copyOf(tokenValue()));
    next();

    if (match(TOKEN_COLON)) {
      Node* type = typeExpression();
      if (type == nullptr) {
        skipUntil({TOKEN_RBRACKET, TOKEN_LBRACE});
      } else {
        tp->setType(type);
      }
    } else {
      if (match(TOKEN_TYPE_LE)) {
        Node* super = typeExpression();
        if (super != nullptr) {
          ast::NodeListBuilder builder(_arena);
          builder.append(super);
          tp->setSubtypeConstraints(builder.build());
        }
      } else if (match(TOKEN_ELLIPSIS)) {
        tp->setVariadic(true);
      }

      if (match(TOKEN_ASSIGN)) {
        Node* init = typeExpression();
        if (init == nullptr) {
          skipUntil({TOKEN_RBRACKET, TOKEN_LBRACE});
        } else {
          tp->setInit(init);
        }
      }
    }
    return tp;
  } else {
    _reporter.error(location()) << "Type parameter name expected.";
    return nullptr;
  }
}

// Type Expressions

Node* Parser::typeExpression() {
  return typeUnion();
}

Node* Parser::typeUnion() {
  Node* t = typeTerm();
  if (match(TOKEN_OR)) {
    ast::NodeListBuilder builder(_arena);
    builder.append(t);
    while (_token != TOKEN_END) {
      t = typeTerm();
      if (t == nullptr) {
        return nullptr;
      }
      builder.append(t);
      if (!match(TOKEN_OR)) {
        break;
      }
    }
    return new ast::Oper(Kind::UNION, builder.location(), builder.build());
  }
  return t;
}

Node* Parser::typeTerm(bool allowPartial) {
  Node* t = typePrimary(allowPartial);
  if (t && match(TOKEN_QMARK)) {
    return new (_arena) ast::UnaryOp(Kind::OPTIONAL, t->location(), t);
  }
  return t;
}

Node* Parser::typePrimary(bool allowPartial) {
  switch (_token) {
    case TOKEN_CONST: {
      Location loc = location();
      next();
      Kind kind = Kind::CONST;
      if (match(TOKEN_QMARK)) {
        kind = Kind::PROVISIONAL_CONST;
      }
      Node* t = typePrimary(allowPartial);
      if (t == nullptr) {
        return nullptr;
      }
      return new (_arena) ast::UnaryOp(kind, loc | t->location(), t);
    }
    case TOKEN_FN: {
      next();
      return functionType();
    }
    case TOKEN_LPAREN: {
      Location loc = location();
      next();
      ast::NodeListBuilder members(_arena);
      bool trailingComma = true;
      if (!match(TOKEN_RPAREN)) {
        for (;;) {
          Node* m = typeExpression();
          if (m == nullptr) {
            return nullptr;
          }
          members.append(m);
          loc |= location();
          if (match(TOKEN_RPAREN)) {
            trailingComma = false;
            break;
          } else if (!match(TOKEN_COMMA)) {
            expected("',' or ')'");
          } else if (match(TOKEN_RPAREN)) {
            trailingComma = true;
            break;
          }
        }
      }

      if (members.size() == 1 && !trailingComma) {
        return members[0];
      }
      return new (_arena) ast::Oper(Kind::TUPLE, loc, members.build());
    }
    case TOKEN_ID: return specializedTypeName();
    case TOKEN_VOID: return builtinType(ast::BuiltInType::VOID);
    case TOKEN_BOOL: return builtinType(ast::BuiltInType::BOOL);
    case TOKEN_CHAR: return builtinType(ast::BuiltInType::CHAR);
    case TOKEN_I8: return builtinType(ast::BuiltInType::I8);
    case TOKEN_I16: return builtinType(ast::BuiltInType::I16);
    case TOKEN_I32: return builtinType(ast::BuiltInType::I32);
    case TOKEN_I64: return builtinType(ast::BuiltInType::I64);
    case TOKEN_INT: return builtinType(ast::BuiltInType::INT);
    case TOKEN_U8: return builtinType(ast::BuiltInType::U8);
    case TOKEN_U16: return builtinType(ast::BuiltInType::U16);
    case TOKEN_U32: return builtinType(ast::BuiltInType::U32);
    case TOKEN_U64: return builtinType(ast::BuiltInType::U64);
    case TOKEN_UINT: return builtinType(ast::BuiltInType::UINT);
    case TOKEN_FLOAT32: return builtinType(ast::BuiltInType::F32);
    case TOKEN_FLOAT64: return builtinType(ast::BuiltInType::F64);
    default: {
      if (allowPartial) {
        return &Node::ABSENT;
      }
      _reporter.error(location()) << "Type name expected.";
      return nullptr;
    }
  }
  return nullptr;
}

Node* Parser::functionType() {
  Location loc = location();
  ast::NodeListBuilder params(_arena);
  if (match(TOKEN_LPAREN)) {
    if (!match(TOKEN_RPAREN)) {
      for (;;) {
        Node* arg = typeExpression();
        if (arg == nullptr) {
          return nullptr;
        }
        // TODO: Do we want to support ellipsis here?
        params.append(arg);
        loc |= location();
        if (match(TOKEN_RPAREN)) {
          break;
        } else if (!match(TOKEN_COMMA)) {
          expected("',' or ')'");
          return nullptr;
        }
      }
    }
  }

  ast::Oper *fnType = new (_arena) ast::Oper(
      Kind::FUNCTION_TYPE, loc, params.build());
  if (match(TOKEN_RETURNS)) {
    Node* returnType = typeExpression();
    if (returnType == nullptr) {
      return nullptr;
    }
    loc |= returnType->location();
    fnType->setOp(returnType);
  }
  return fnType;
}

Node* Parser::specializedTypeName() {
  assert(_token == TOKEN_ID);
  Node* type = id();
  assert(type != nullptr);
  for (;;) {
    Location loc = type->location();
    if (match(TOKEN_LBRACKET)) {
      ast::NodeListBuilder builder(_arena);
      loc = loc | location();
      if (!match(TOKEN_RBRACKET)) {
        for (;;) {
          Node* typeArg = typeExpression();
          if (typeArg == nullptr) {
            return nullptr;
          }
          builder.append(typeArg);
          loc = loc | location();
          if (match(TOKEN_RBRACKET)) {
            break;
          } else if (!match(TOKEN_COMMA)) {
            expected("',' or ']'");
            return nullptr;
          }
        }
      }
      ast::Oper* spec = new (_arena) ast::Oper(Kind::SPECIALIZE, loc, builder.build());
      spec->setOp(type);
      type = spec;
    } else if (match(TOKEN_DOT)) {
      if (_token == TOKEN_ID) {
        type = new (_arena) ast::MemberRef(location(), copyOf(tokenValue()), type);
        next();
      } else {
        expected("identifier");
        return nullptr;
      }
    } else {
      return type;
    }
  }
}

Node* Parser::builtinType(ast::BuiltInType::Type t) {
  Node* result = new (_arena) ast::BuiltInType(location(), t);
  next();
  return result;
}

// Statements

// Statements that don't require a terminating semicolon.
static const std::unordered_set<TokenType> BLOCK_ENDING_STMTS = {
  TOKEN_IF, TOKEN_WHILE, TOKEN_LOOP, TOKEN_FOR, TOKEN_SWITCH, TOKEN_MATCH, TOKEN_TRY,
  TOKEN_LET, TOKEN_VAR, TOKEN_DEF, TOKEN_CLASS, TOKEN_STRUCT, TOKEN_INTERFACE, TOKEN_ENUM,
  TOKEN_EXTEND
};

Node* Parser::block() {
  Location loc = location();
  if (match(TOKEN_LBRACE)) {
    ast::NodeListBuilder stmts(_arena);
    for (;;) {
      if (match(TOKEN_RBRACE)) {
        break;
      }
      TokenType stType = _token;
      if (match(TOKEN_SEMI)) {
        continue;
      }
      Node* st = stmt();
      if (st == nullptr) {
        return nullptr;
      }

      // Semicolon is only required *between* statements, and only for some statement types.
      if (!match(TOKEN_SEMI)) {
        if (_token != TOKEN_RBRACE && BLOCK_ENDING_STMTS.find(stType) == BLOCK_ENDING_STMTS.end()) {
          _reporter.error(location()) << "Semicolon expected after statement.";
        }
      }
      stmts.append(st);
    }
    return new (_arena) ast::Oper(Kind::BLOCK, loc, stmts.build());
  }
  assert(false && "Missing opening brace.");
  return nullptr;
}

Node* Parser::requiredBlock() {
  if (_token != TOKEN_LBRACE) {
    _reporter.error(location()) << "Statement block required.";
    return nullptr;
  }
  return block();
}

Node* Parser::stmt() {
  switch (_token) {
    case TOKEN_IF:      return ifStmt();
    case TOKEN_WHILE:   return whileStmt();
    case TOKEN_LOOP:    return loopStmt();
    case TOKEN_FOR:     return forStmt();
    case TOKEN_SWITCH:  return switchStmt();
    case TOKEN_MATCH:   return matchStmt();
    case TOKEN_TRY:     return tryStmt();

    case TOKEN_BREAK: {
      Node* st = new (_arena) Node(Kind::BREAK, location());
      next();
      return st;
    }

    case TOKEN_CONTINUE: {
      Node* st = new (_arena) Node(Kind::CONTINUE, location());
      next();
      return st;
    }

    case TOKEN_RETURN:  return returnStmt();
    case TOKEN_THROW:   return throwStmt();

    case TOKEN_LET:
    case TOKEN_VAR:
      return varOrLetDefn();

    case TOKEN_DEF:
    case TOKEN_CLASS:
    case TOKEN_STRUCT:
    case TOKEN_INTERFACE:
    case TOKEN_ENUM:
    case TOKEN_EXTEND:
      _reporter.error(location()) << "here";
      assert(false && "Implement local type");
      break;
    default: {
      return assignStmt();
    }
  }
}

Node* Parser::assignStmt() {
  Node* left = exprList();
  if (left == nullptr) {
    return nullptr;
  }

  Kind kind = Kind::ABSENT;
  switch (_token) {
    case TOKEN_ASSIGN: kind = Kind::ASSIGN; break;
    case TOKEN_ASSIGN_PLUS: kind = Kind::ASSIGN_ADD; break;
    case TOKEN_ASSIGN_MINUS: kind = Kind::ASSIGN_SUB; break;
    case TOKEN_ASSIGN_MUL: kind = Kind::ASSIGN_MUL; break;
    case TOKEN_ASSIGN_DIV: kind = Kind::ASSIGN_DIV; break;
    case TOKEN_ASSIGN_MOD: kind = Kind::ASSIGN_MOD; break;
    case TOKEN_ASSIGN_RSHIFT: kind = Kind::ASSIGN_RSHIFT; break;
    case TOKEN_ASSIGN_LSHIFT: kind = Kind::ASSIGN_LSHIFT; break;
    case TOKEN_ASSIGN_BITAND: kind = Kind::ASSIGN_BIT_AND; break;
    case TOKEN_ASSIGN_BITOR: kind = Kind::ASSIGN_BIT_OR; break;
    case TOKEN_ASSIGN_BITXOR: kind = Kind::ASSIGN_BIT_XOR; break;
    default:
      return left;
  }

  next();
  Node* right = exprList();
  if (right == nullptr) {
    return nullptr;
  }

  ast::NodeListBuilder operands(_arena);
  operands.append(left);
  operands.append(right);

  return new (_arena) ast::Oper(kind, left->location() | right->location(), operands.build());
}

Node* Parser::ifStmt() {
  next();
  ast::NodeListBuilder builder(_arena);
  Node* test = expression();
  if (test == nullptr) {
    return nullptr;
  }
  Node* thenBlk = requiredBlock();
  if (thenBlk == nullptr) {
    return nullptr;
  }
  builder.append(thenBlk);
  if (match(TOKEN_ELSE)) {
    Node* elseBlk = nullptr;
    if (_token == TOKEN_IF) {
      elseBlk = ifStmt();
    } else {
      elseBlk = requiredBlock();
    }
    if (elseBlk != nullptr) {
      builder.append(elseBlk);
    }
  }

  return new (_arena) ast::ControlStmt(Kind::IF, test->location(), test, builder.build());
}

Node* Parser::whileStmt() {
  next();
  ast::NodeListBuilder builder(_arena);
  Node* test = expression();
  if (test == nullptr) {
    return nullptr;
  }
  Node* body = requiredBlock();
  if (body == nullptr) {
    return nullptr;
  }
  builder.append(body);
  return new (_arena) ast::ControlStmt(Kind::WHILE, test->location(), test, builder.build());
}

Node* Parser::loopStmt() {
  Location loc = location();
  next();
  ast::NodeListBuilder builder(_arena);
  Node* body = requiredBlock();
  if (body == nullptr) {
    return nullptr;
  }
  builder.append(body);
  return new (_arena) ast::ControlStmt(Kind::LOOP, loc, nullptr, builder.build());
}

Node* Parser::forStmt() {
  Location loc = location();
  next();

  ast::NodeListBuilder builder(_arena);

  if (_token == TOKEN_SEMI) {
    builder.append(&Node::ABSENT);
  } else {
    ast::ValueDefn* var = varDeclList(Kind::LET);

    builder.append(var);

    // It's a for-in statement
    if (match(TOKEN_IN)) {
      // Iterator expression
      Node* iter = expression();
      if (iter == nullptr) {
        return nullptr;
      }
      builder.append(iter);

      // Loop body
      Node* body = requiredBlock();
      if (body == nullptr) {
        return nullptr;
      }
      builder.append(body);
      return new (_arena) ast::ControlStmt(Kind::FOR_IN, loc, nullptr, builder.build());
    } else  if (match(TOKEN_ASSIGN)) {
      // Initializer
      Node* init = exprList();
      if (init != nullptr) {
        var->setInit(init);
      }
    }
  }

  if (!match(TOKEN_SEMI)) {
    expected("';'");
    return nullptr;
  }

  if (_token == TOKEN_SEMI) {
    builder.append(&Node::ABSENT);
  } else {
    Node* test = exprList();
    if (test == nullptr) {
      return nullptr;
    }
    builder.append(test);
  }

  if (!match(TOKEN_SEMI)) {
    expected("';'");
    return nullptr;
  }

  if (_token == TOKEN_SEMI) {
    builder.append(&Node::ABSENT);
  } else {
    Node* step = assignStmt();
    if (step == nullptr) {
      return nullptr;
    }
    builder.append(step);
  }

  Node* body = requiredBlock();
  if (body == nullptr) {
    return nullptr;
  }

  builder.append(body);
  return new (_arena) ast::ControlStmt(Kind::FOR, loc, nullptr, builder.build());
}

Node* Parser::switchStmt() {
  Location loc = location();
  next();
  Node* test = expression();
  if (test == nullptr) {
    return nullptr;
  }
  Location braceLoc = location();
  if (!match(TOKEN_LBRACE)) {
    expected("'{'");
    return nullptr;
  }
  ast::NodeListBuilder cases(_arena);
  while (!match(TOKEN_RBRACE)) {
    if (_token == TOKEN_END) {
      _reporter.error(braceLoc) << "Incomplete switch.";
    }

    Kind kind = Kind::CASE;
    ast::NodeListBuilder caseValues(_arena);
    if (match(TOKEN_ELSE)) {
      // 'else' block
      kind = Kind::ELSE;
    } else {
      // case block
      for (;;) {
        if (_token == TOKEN_END) {
          _reporter.error(braceLoc) << "Incomplete switch.";
        }
        Node* c = caseExpr();
        if (c == nullptr) {
          return nullptr;
        }
        caseValues.append(c);
        if (!match(TOKEN_COMMA)) {
          break;
        }
      }
    }

    if (!match(TOKEN_FAT_ARROW)) {
      expected("'=>'");
      return nullptr;
    }

    // Case block body
    Node* body = caseBody();
    if (body == nullptr) {
      return nullptr;
    }

    ast::Oper* caseSt = new (_arena) ast::Oper(kind, caseValues.location(), caseValues.build());
    caseSt->setOp(body);
    cases.append(caseSt);
  }
  return new (_arena) ast::ControlStmt(Kind::SWITCH, loc, test, cases.build());
}

Node* Parser::caseExpr() {
  Node* e = primary();
  if (e != nullptr && match(TOKEN_RANGE)) {
    Node* e2 = primary();
    if (e2 == nullptr) {
      return nullptr;
    }
    ast::NodeListBuilder rangeBounds(_arena);
    rangeBounds.append(e);
    rangeBounds.append(e2);
    return new (_arena) ast::Oper(Kind::RANGE, rangeBounds.location(), rangeBounds.build());
  }
  return e;
}

Node* Parser::caseBody() {
  // Case block body
  Node* body;
  if (_token == TOKEN_LBRACE) {
    return block();
  } else {
    body = expression();
    if (body != nullptr && !match(TOKEN_SEMI)) {
      expected("';'");
      return nullptr;
    }
  }
  return body;
}

Node* Parser::matchStmt() {
  Location loc = location();
  next();
  Node* test = expression();
  if (test == nullptr) {
    return nullptr;
  }
  Location braceLoc = location();
  if (!match(TOKEN_LBRACE)) {
    expected("'{'");
    return nullptr;
  }

  ast::NodeListBuilder patterns(_arena);
  while (!match(TOKEN_RBRACE)) {
    if (_token == TOKEN_END) {
      _reporter.error(braceLoc) << "Incomplete match statement.";
    }

    ast::NodeListBuilder patternArgs(_arena);
    Location patternLoc = location();
    Kind kind;
    if (match(TOKEN_ELSE)) {
      kind = Kind::ELSE;
    } else if (_token == TOKEN_ID) {
      kind = Kind::PATTERN;
      Node* name = &Node::ABSENT;
      Node* type = typeTerm();
      if (type && type->kind() == Kind::IDENT && match(TOKEN_COLON)) {
        name = type;
        type = typeTerm();
        if (type == nullptr) {
          return nullptr;
        }
      }
      patternArgs.append(name);
      patternArgs.append(type);
    } else {
      Node* type = typeTerm(false);
      if (type == nullptr) {
        _reporter.error(location()) << "Match pattern expected.";
        return nullptr;
      }
      patternArgs.append(&Node::ABSENT);
      patternArgs.append(type);
    }

    if (!match(TOKEN_FAT_ARROW)) {
      expected("'=>'");
      return nullptr;
    }

    // Pattern block body
    Node* body = caseBody();
    if (body == nullptr) {
      return nullptr;
    }
    patternArgs.append(body);
    ast::Oper* pattern = new (_arena) ast::Oper(kind, patternLoc, patternArgs.build());
    patterns.append(pattern);
  }

  return new (_arena) ast::ControlStmt(Kind::MATCH, loc, test, patterns.build());
}

Node* Parser::tryStmt() {
  _reporter.error(location()) << "here";
  assert(false && "Implement tryStmt");
}

Node* Parser::returnStmt() {
  Location loc = location();
  next();
  Node* returnVal = nullptr;
  if (_token != TOKEN_SEMI && _token != TOKEN_LBRACE) {
    returnVal = exprList();
  }
  return new (_arena) ast::UnaryOp(Kind::RETURN, loc, returnVal);
}

Node* Parser::throwStmt() {
  Location loc = location();
  next();
  Node* ex = expression();
  return new (_arena) ast::UnaryOp(Kind::THROW, loc, ex);
}

#if 0
  # All statements that end with a closing brace
  def p_closing_brace_stmt(self, p):
    '''closing_brace_stmt : block
                          | if_stmt
                          | while_stmt
                          | loop_stmt
                          | for_in_stmt
                          | for_stmt
                          | switch_stmt
                          | match_stmt
                          | try_stmt'''
    p[0] = p[1]

  def p_try_stmt(self, p):
    '''try_stmt : TRY block catch_list finally'''
    p[0] = ast.Try()

  def p_catch_list(self, p):
    '''catch_list : catch_list catch
                  | empty'''

  def p_catch(self, p):
    '''catch : CATCH ID COLON type_expr block
             | CATCH type_expr block
             | CATCH block'''

  def p_finally(self, p):
    '''finally : FINALLY block
               | empty'''

  def p_assign_stmt(self, p):
    '''assign_stmt : expr_or_tuple ASSIGN assign_stmt
                   | expr_or_tuple ASSIGN if_stmt
                   | expr_or_tuple ASSIGN expr_or_tuple'''
    p[0] = ast.Assign(location = self.location(p, 1, 3))
    p[0].setLeft(p[1])
    p[0].setRight(p[3])

#endif

// Expressions

Node* Parser::expression() {
  return binary();
}

Node* Parser::exprList() {
  Node* expr = binary();
  if (match(TOKEN_COMMA)) {
    ast::NodeListBuilder builder(_arena);
    builder.append(expr);
    while (_token != TOKEN_END) {
      expr = binary();
      if (expr == nullptr) {
        return nullptr;
      }
      builder.append(expr);
      if (!match(TOKEN_COMMA)) {
        break;
      }
    }
    return new (_arena) ast::Oper(Kind::TUPLE, builder.location(), builder.build());
  }
  return expr;
}

Node* Parser::binary() {
  Node* e0 = unary();
  if (e0 == nullptr) {
    return nullptr;
  }

  OperatorStack opstack(e0, _arena);
  for (;;) {
    switch (_token) {
      case TOKEN_PLUS:
        opstack.pushOperator(Kind::ADD, PREC_ADD_SUB);
        next();
        break;

      case TOKEN_MINUS:
        opstack.pushOperator(Kind::SUB, PREC_ADD_SUB);
        next();
        break;

      case TOKEN_MUL:
        opstack.pushOperator(Kind::MUL, PREC_MUL_DIV);
        next();
        break;

      case TOKEN_DIV:
        opstack.pushOperator(Kind::DIV, PREC_MUL_DIV);
        next();
        break;

      case TOKEN_MOD:
        opstack.pushOperator(Kind::MOD, PREC_MUL_DIV);
        next();
        break;

      case TOKEN_AMP:
        opstack.pushOperator(Kind::BIT_AND, PREC_BIT_AND);
        next();
        break;

      case TOKEN_VBAR:
        opstack.pushOperator(Kind::BIT_OR, PREC_BIT_OR);
        next();
        break;

      case TOKEN_CARET:
        opstack.pushOperator(Kind::BIT_XOR, PREC_BIT_XOR);
        next();
        break;

      case TOKEN_AND:
        opstack.pushOperator(Kind::LOGICAL_AND, PREC_LOGICAL_AND);
        next();
        break;

      case TOKEN_OR:
        opstack.pushOperator(Kind::LOGICAL_OR, PREC_LOGICAL_OR);
        next();
        break;

      case TOKEN_LSHIFT:
        opstack.pushOperator(Kind::LSHIFT, PREC_SHIFT);
        next();
        break;

      case TOKEN_RSHIFT:
        opstack.pushOperator(Kind::RSHIFT, PREC_SHIFT);
        next();
        break;

        case TOKEN_RANGE:
        opstack.pushOperator(Kind::RANGE, PREC_RANGE);
        next();
        break;

      case TOKEN_EQ:
        opstack.pushOperator(Kind::EQUAL, PREC_RELATIONAL);
        next();
        break;

      case TOKEN_NE:
        opstack.pushOperator(Kind::NOT_EQUAL, PREC_RELATIONAL);
        next();
        break;

      case TOKEN_REF_EQ:
        opstack.pushOperator(Kind::REF_EQUAL, PREC_RELATIONAL);
        next();
        break;

      case TOKEN_LT:
        opstack.pushOperator(Kind::LESS_THAN, PREC_RELATIONAL);
        next();
        break;

      case TOKEN_GT:
        opstack.pushOperator(Kind::GREATER_THAN, PREC_RELATIONAL);
        next();
        break;

      case TOKEN_LE:
        opstack.pushOperator(Kind::LESS_THAN_OR_EQUAL, PREC_RELATIONAL);
        next();
        break;

      case TOKEN_GE:
        opstack.pushOperator(Kind::GREATER_THAN_OR_EQUAL, PREC_RELATIONAL);
        next();
        break;

//       case TOKEN_RETURNS: {
//         opstack.pushOperator(Kind::RETURNS, PREC_RETURNS);
//         next();
//         break;
//       }

      case TOKEN_AS: {
        // The second argument to 'as' is a type expression.
        opstack.pushOperator(Kind::AS_TYPE, PREC_IS_AS);
        next();
        Node* t1 = typeExpression();
        if (t1 == nullptr) {
          return nullptr;
        }
        opstack.pushOperand(t1);
        continue;
      }

      case TOKEN_IS: {
        next();

        // Handle 'is not'
        if (match(TOKEN_NOT)) {
          opstack.pushOperator(Kind::IS_NOT, PREC_IS_AS);
        } else {
          opstack.pushOperator(Kind::IS, PREC_IS_AS);
        }

        // Second argument is a type
        Node* t1 = typeExpression();
        if (t1 == nullptr) {
          return nullptr;
        }
        opstack.pushOperand(t1);
        continue;
      }

      case TOKEN_COLON: {
        // Used is for tuples that are actually parameter lists.
        opstack.pushOperator(Kind::EXPR_TYPE, PREC_RANGE);
        next();
        Node* t1 = typeExpression();
        if (t1 == nullptr) {
          return nullptr;
        }
        opstack.pushOperand(t1);
        continue;
      }

      case TOKEN_IN:
        opstack.pushOperator(Kind::IN, PREC_IN);
        next();
        break;

      case TOKEN_FAT_ARROW:
        opstack.pushOperator(Kind::LAMBDA, PREC_FAT_ARROW);
        next();
        break;

      case TOKEN_NOT: {
        // Negated operators
        next();
        Location loc = location();
        if (match(TOKEN_IN)) {
          opstack.pushOperator(Kind::NOT_IN, PREC_IN);
        } else {
          _reporter.error(loc) << "'in' expected after 'not'";
        }
        break;
      }

      default:
        goto done;
    }

    Node* e1 = unary();
    if (e1 == nullptr) {
      return nullptr;
    }
    opstack.pushOperand(e1);
  }

done:
  if (!opstack.reduceAll()) {
    return e0;
  }

  return opstack.expression();
}

#if 0
  def p_fluent_member_ref(self, p):
    '''fluent_member_ref : call_op DOT LBRACE RBRACE'''
    p[0] = ast.FluentMember(location = self.location(p, 1, 3))
    p[0].setBase(p[1])
    p[0].setName(p[3])

  def p_primary(self, p):
    '''primary : tuple_expr
               | array_lit
.               | specialize
               | type_fn
.               | member_ref
               | fluent_member_ref
.               | dotid
.               | id
.               | self
.               | super
.               | true
.               | false
.               | null
.               | string_lit
.               | char_lit
.               | dec_int
.               | hex_int
.               | float
.               | primitive_type'''
    p[0] = p[1]
    assert isinstance(p[0], ast.Node), type(p[0])
#endif

Node* Parser::unary() {
  Location loc = location();
  if (match(TOKEN_MINUS)) {
    Node* expr = primary();
    if (expr == nullptr) {
      return nullptr;
    }
    return new (_arena) ast::UnaryOp(Kind::NEGATE, loc | expr->location(), expr);
  } else if (match(TOKEN_PLUS)) {
    return primary();
  } else if (match(TOKEN_INC)) {
    Node* expr = primary();
    if (expr == nullptr) {
      return nullptr;
    }
    return new (_arena) ast::UnaryOp(Kind::PRE_INC, loc | expr->location(), expr);
  } else if (match(TOKEN_DEC)) {
    Node* expr = primary();
    if (expr == nullptr) {
      return nullptr;
    }
    return new (_arena) ast::UnaryOp(Kind::PRE_DEC, loc | expr->location(), expr);
  } else if (match(TOKEN_NOT)) {
    Node* expr = primary();
    if (expr == nullptr) {
      return nullptr;
    }
    return new (_arena) ast::UnaryOp(Kind::LOGICAL_NOT, loc | expr->location(), expr);
  }

  Node* expr = primary();
  if (expr == nullptr) {
    return nullptr;
  }
  if (match(TOKEN_INC)) {
    return new (_arena) ast::UnaryOp(Kind::POST_INC, loc | expr->location(), expr);
  } else if (match(TOKEN_DEC)) {
    return new (_arena) ast::UnaryOp(Kind::POST_DEC, loc | expr->location(), expr);
  } else {
    return expr;
  }
}

Node* Parser::primary() {
  switch (_token) {
    case TOKEN_LPAREN: {
      ast::NodeListBuilder builder(_arena);
      Location loc = location();
      next();
      Location finalLoc = loc;
      bool trailingComma = true;
      if (!match(TOKEN_RPAREN)) {
        for (;;) {
          Node* n = expression();
          if (Node::isError(n)) {
            return n;
          }
          builder.append(n);
          finalLoc = location();
          if (match(TOKEN_RPAREN)) {
            trailingComma = false;
            break;
          } else if (!match(TOKEN_COMMA)) {
            expected("',' or ')'");
          } else if (match(TOKEN_RPAREN)) {
            trailingComma = true;
            break;
          }
        }
      }
      loc = loc | finalLoc;
      if (builder.size() == 1 && !trailingComma) {
        return builder[0];
      }
      return new (_arena) ast::Oper(Kind::TUPLE, loc, builder.build());
    }
    case TOKEN_TRUE: return node(Kind::BOOLEAN_TRUE);
    case TOKEN_FALSE: return node(Kind::BOOLEAN_FALSE);
    case TOKEN_NULL: return node(Kind::NULL_LITERAL);
    case TOKEN_STRING_LIT: return stringLit();
    case TOKEN_CHAR_LIT: return charLit();
    case TOKEN_DEC_INT_LIT:
    case TOKEN_HEX_INT_LIT: return integerLit();
    case TOKEN_FLOAT_LIT: return floatLit();

    case TOKEN_ID:
    case TOKEN_SELF:
    case TOKEN_SUPER:
    case TOKEN_VOID:
    case TOKEN_BOOL:
    case TOKEN_CHAR:
    case TOKEN_I8:
    case TOKEN_I16:
    case TOKEN_I32:
    case TOKEN_I64:
    case TOKEN_INT:
    case TOKEN_U8:
    case TOKEN_U16:
    case TOKEN_U32:
    case TOKEN_U64:
    case TOKEN_UINT:
    case TOKEN_FLOAT32:
    case TOKEN_FLOAT64: {
      Node* e = namedPrimary();
      if (e == nullptr) {
        return nullptr;
      }
      return primarySuffix(e);
    }

    case TOKEN_IF: return ifStmt();
    case TOKEN_SWITCH: return switchStmt();
    case TOKEN_MATCH: return matchStmt();
    case TOKEN_TRY: return tryStmt();

    case TOKEN_DOT: {
      next();
      if (_token == TOKEN_ID) {
        Node* expr = id();
        if (expr == nullptr) {
          return nullptr;
        }
        auto e = new (_arena) ast::UnaryOp(Kind::SELF_NAME_REF, expr->location(), expr);
        return primarySuffix(e);
      } else {
        expected("identifier");
        return nullptr;
      }
    }

    default:
      _reporter.error(location()) << "Expression expected, not: " << _token << ".";
      break;
  }
  return nullptr;
}

Node* Parser::namedPrimary() {
  switch (_token) {
    case TOKEN_ID: return id();
    case TOKEN_SELF: {
      Node* n = new (_arena) Node(Kind::SELF, location());
      next();
      return n;
    }
    case TOKEN_SUPER: {
      Node* n = new (_arena) Node(Kind::SUPER, location());
      next();
      return n;
    }
    case TOKEN_VOID: return builtinType(ast::BuiltInType::VOID);
    case TOKEN_BOOL: return builtinType(ast::BuiltInType::BOOL);
    case TOKEN_CHAR: return builtinType(ast::BuiltInType::CHAR);
    case TOKEN_I8: return builtinType(ast::BuiltInType::I8);
    case TOKEN_I16: return builtinType(ast::BuiltInType::I16);
    case TOKEN_I32: return builtinType(ast::BuiltInType::I32);
    case TOKEN_I64: return builtinType(ast::BuiltInType::I64);
    case TOKEN_INT: return builtinType(ast::BuiltInType::INT);
    case TOKEN_U8: return builtinType(ast::BuiltInType::U8);
    case TOKEN_U16: return builtinType(ast::BuiltInType::U16);
    case TOKEN_U32: return builtinType(ast::BuiltInType::U32);
    case TOKEN_U64: return builtinType(ast::BuiltInType::U64);
    case TOKEN_UINT: return builtinType(ast::BuiltInType::UINT);
    case TOKEN_FLOAT32: return builtinType(ast::BuiltInType::F32);
    case TOKEN_FLOAT64: return builtinType(ast::BuiltInType::F64);
    default: assert(false);
  }
}

Node* Parser::primarySuffix(Node* expr) {
  Location openLoc = expr->location();
  while (_token != TOKEN_END) {
    if (match(TOKEN_DOT)) {
      if (_token == TOKEN_ID) {
        expr = new (_arena) ast::MemberRef(openLoc | location(), copyOf(tokenValue()), expr);
        next();
      } else {
        expected("identifier");
      }
    } else if (match(TOKEN_LPAREN)) {
      ast::NodeListBuilder args(_arena);
      Location callLoc = openLoc;
      if (!callingArgs(args, callLoc)) {
        return nullptr;
      }
      ast::Oper* call = new (_arena) ast::Oper(Kind::CALL, callLoc, args.build());
      call->setOp(expr);
      expr = call;
    } else if (match(TOKEN_LBRACKET)) {
      ast::NodeListBuilder typeArgs(_arena);
      Location fullLoc = openLoc;
      if (!match(TOKEN_RBRACKET)) {
        for (;;) {
          if (_token == TOKEN_END) {
            _reporter.error(openLoc) << "Unmatched bracket.";
            return nullptr;
          }
          Node* arg = expression();
          if (arg == nullptr) {
            return nullptr;
          }
          typeArgs.append(arg);
          fullLoc |= location();
          if (match(TOKEN_RBRACKET)) {
            break;
          } else if (!match(TOKEN_COMMA)) {
            expected("',' or ']'");
            return nullptr;
          }
        }
      }
      ast::Oper* spec = new (_arena) ast::Oper(Kind::SPECIALIZE, fullLoc, typeArgs.build());
      spec->setOp(expr);
      expr = spec;
    } else {
      return expr;
    }
  }
  return expr;
}

bool Parser::callingArgs(ast::NodeListBuilder &args, Location& argsLoc) {
  if (!match(TOKEN_RPAREN)) {
    for (;;) {
      if (_token == TOKEN_END) {
        _reporter.error(argsLoc) << "Unmatched paren.";
        return false;
      }
      Node* arg = expression();
      if (arg == nullptr) {
        return false;
      }

      // Keyword argument.
      if (match(TOKEN_ASSIGN)) {
        Node* kwValue = expression();
        if (kwValue == nullptr) {
          return false;
        }
        ast::NodeListBuilder kwArg(_arena);
        kwArg.append(arg);
        kwArg.append(kwValue);
        arg = new (_arena) ast::Oper(
            Kind::KEYWORD_ARG, arg->location() | kwValue->location(), kwArg.build());
      }

      args.append(arg);
      argsLoc |= location();
      if (match(TOKEN_RPAREN)) {
        break;
      } else if (!match(TOKEN_COMMA)) {
        expected("',' or ')'");
        return false;
      }
    }
  }
  return true;
}

#if 0

  def p_tuple_expr(self, p):
    '''tuple_expr : LPAREN arg_list COMMA RPAREN
                  | LPAREN opt_arg_list RPAREN'''
    if len(p) == 5 or len(p[2]) != 1:
      p[0] = ast.Tuple(location = self.location(p, 1, 3))
      p[0].mutableArgs.extend(p[2])
    else:
      p[0] = p[2][0]

  def p_array_lit(self, p):
    '''array_lit : LBRACKET arg_list COMMA RBRACKET
                 | LBRACKET opt_arg_list RBRACKET'''
    p[0] = ast.ArrayLiteral(location = self.location(p, 1, 3))
    p[0].mutableArgs.extend(p[2])

#endif

/** Terminals */

Node* Parser::dottedIdent() {
  Node* result = id();
  if (result) {
    while (match(TOKEN_DOT)) {
      if (_token == TOKEN_ID) {
        result = new (_arena) ast::MemberRef(
            result->location() | location(), copyOf(tokenValue()), result);
        next();
      } else {
        expected("identifier");
      }
    }
  }
  return result;
}

Node* Parser::id() {
  assert(_token == TOKEN_ID);
  Node* node = new (_arena) ast::Ident(location(), copyOf(tokenValue()));
  next();
  return node;
}

Node* Parser::stringLit() {
  assert(_token == TOKEN_STRING_LIT);
  Node* node = new (_arena) ast::TextLiteral(
      Kind::STRING_LITERAL, location(), copyOf(tokenValue()));
  next();
  return node;
}

Node* Parser::charLit() {
  assert(_token == TOKEN_CHAR_LIT);
  Node* node = new (_arena) ast::TextLiteral(
      Kind::CHAR_LITERAL, location(), copyOf(tokenValue()));
  next();
  return node;
}

Node* Parser::integerLit() {
  assert(_token == TOKEN_DEC_INT_LIT || _token == TOKEN_HEX_INT_LIT);
  bool uns = false;
  int64_t value;
  if (_token == TOKEN_DEC_INT_LIT) {
    value = strtoll(_lexer.tokenValue().c_str(), nullptr, 10);
  } else {
    value = strtoll(_lexer.tokenValue().c_str(), nullptr, 16);
  }
  if (_lexer.tokenSuffix().empty()) {
    for (char ch : _lexer.tokenSuffix()) {
      assert(ch == 'u');
      uns = true;
    }
  }
  ast::IntegerLiteral* node = new (_arena) ast::IntegerLiteral(location(), value, uns);
  next();
  return node;
}

Node* Parser::floatLit() {
  assert(_token == TOKEN_FLOAT_LIT);
  double d = strtod(_lexer.tokenValue().c_str(), nullptr);
  Node* node = new (_arena) ast::FloatLiteral(location(), d);
  next();
  return node;
}

StringRef Parser::copyOf(const StringRef& str) {
  support::Arena::value_type* data = _arena.allocate(str.size());
  std::copy(str.begin(), str.end(), data);
  return StringRef((char*) data, str.size());
}

// TODO: Get rid of this
Node* Parser::node(Kind kind) {
  Node* n = new (_arena) Node(kind, location());
  next();
  return n;
}

void Parser::expected(const StringRef& tokens) {
  _reporter.error(location()) << "Expected " << tokens << ".";
}

bool Parser::skipUntil(const std::unordered_set<TokenType>& tokens) {
  while (_token != TOKEN_END) {
    if (tokens.find(_token) != tokens.end()) {
      return true;
    }
    next();
  }
  return false;
}

static const std::unordered_set<TokenType> DEFN_TOKENS = {
  TOKEN_CLASS, TOKEN_STRUCT, TOKEN_INTERFACE, TOKEN_ENUM, TOKEN_EXTEND,
  TOKEN_DEF, TOKEN_UNDEF, TOKEN_OVERRIDE,
  TOKEN_VAR, TOKEN_LET
};

bool Parser::skipOverDefn() {
  int nesting = 0;
  while (_token != TOKEN_END) {
    if (match(TOKEN_LBRACE)) {
      nesting += 1;
    } else if (match(TOKEN_RBRACE)) {
      if (nesting > 0) {
        nesting -= 1;
      }
    } else if (nesting == 0 && DEFN_TOKENS.find(_token) != DEFN_TOKENS.end()) {
      return true;
    }
    next();
  }
  return false;
}

#if 0
  def takeComment(self, defn):
    if self.lexer.commentLines:
      dc = graph.DocComment()
      dc.setLocation(self.lexer.commentLoc)
      dc.setColumn(self.lexer.commentColumn)
      dc.getMutableText().extend(self.lexer.commentLines)
      self.lexer.commentLines = []
      defn.setDocComment(dc)

  def noComment(self):
    if self.lexer.commentLines:
#       self.errorReporter.errorAt(self.lexer.commentLoc, "Doc comment ignored.")
      self.lexer.commentLines = []
#endif

void OperatorStack::pushOperand(Node* operand) {
  assert(_entries.back().operand == nullptr);
  _entries.back().operand = operand;
}

bool OperatorStack::pushOperator(ast::Kind oper, int16_t prec, bool rightAssoc) {
  assert(_entries.back().operand != nullptr);
  if (!reduce(prec, rightAssoc)) {
    return false;
  }

  _entries.push_back(Entry());
  _entries.back().oper = oper;
  _entries.back().precedence = prec;
  _entries.back().rightAssoc = rightAssoc;
  _entries.back().operand = nullptr;
  return true;
}

bool OperatorStack::reduce(int16_t precedence, bool rightAssoc) {
  while (_entries.size() > 1) {
    Entry back = _entries.back();
    if (back.precedence < precedence) {
      break;
    }
    assert(back.operand != nullptr);
    _entries.pop_back();
    ast::NodeListBuilder args(_arena);
    args.append(_entries.back().operand);
    args.append(back.operand);
    Location loc = _entries.back().operand->location() | back.operand->location();
    Node* combined = new (_arena) ast::Oper(back.oper, loc, args.build());
    _entries.back().operand = combined;
  }
  return true;
}

bool OperatorStack::reduceAll() {
  if (!reduce(0, 0)) {
    return false;
  }
  assert(_entries.size() == 1);
  return true;
}

}}
