// ============================================================================
// parser.h: Spark source file parser.
// ============================================================================

#ifndef SPARK_PARSE_PARSER_H
#define SPARK_PARSE_PARSER_H 1

#include "spark/config.h"

#ifndef SPARK_PARSE_LEXER_H
  #include "spark/parse/lexer.h"
#endif

#ifndef SPARK_AST_NODE_H
  #include "spark/ast/node.h"
#endif

#ifndef SPARK_AST_BUILDER_H
  #include "spark/ast/builder.h"
#endif

#ifndef SPARK_AST_DEFN_H
  #include "spark/ast/defn.h"
#endif

#ifndef SPARK_AST_IDENT_H
  #include "spark/ast/ident.h"
#endif

#ifndef SPARK_SUPPORT_ARENA_H
  #include "spark/support/arena.h"
#endif

#if SPARK_HAVE_UNORDERED_SET
  #include <unordered_set>
#endif

namespace spark {
namespace ast {
class Defn;
class Module;
}
namespace error {
class Reporter;
}
namespace parse {
using spark::source::DocComment;
using spark::source::ProgramSource;
using spark::source::Location;
using spark::error::Reporter;

/** Stack used for operator precedence parsing. */
class OperatorStack {
public:
  /** Contains an operator/operand pair. The bottom element of the stack contains only an
      operand:

      [NULL value][op value][op value] ...
   */
  struct Entry {
    ast::Node* operand;
    ast::Node::Kind oper;
    int16_t precedence;
    bool rightAssoc;

    Entry()
      : operand(NULL)
      , oper(ast::Node::KIND_ABSENT)
      , precedence(0)
      , rightAssoc(false)
    {}

    Entry(const Entry& entry)
      : operand(entry.operand)
      , oper(entry.oper)
      , precedence(entry.precedence)
      , rightAssoc(entry.rightAssoc)
    {}
  };

  OperatorStack(ast::Node* initialExpr, support::Arena& arena)
    : _arena(arena)
  {
    _entries.push_back(Entry());
    _entries.back().operand = initialExpr;
  }

  void pushOperand(ast::Node* operand);
  bool pushOperator(ast::Node::Kind oper, int16_t prec, bool rightAssoc = false);
  bool reduce(int16_t precedence, bool rightAssoc = false);
  bool reduceAll();

  ast::Node * expression() const {
    return _entries.front().operand;
  }
private:
  support::Arena& _arena;
  std::vector<Entry> _entries;
};

/** Spark source parser. */
class Parser {
public:
  Parser(Reporter& reporter, ProgramSource* source, support::Arena& arena);

  ast::Module* module();
  ast::Node* expression();
  ast::Node* typeExpression();
private:
  Reporter&         _reporter;
  ProgramSource*    _source;
  support::Arena&   _arena;
  Lexer             _lexer;
  TokenType         _token;
  bool              _recovering;

  bool declaration(ast::NodeListBuilder& decls, bool isProtected = false, bool isPrivate = false);
  ast::Node* attribute();
  ast::Defn* memberDef();
  collections::StringRef methodName();
  ast::Defn* compositeTypeDef();
  bool classBody(ast::TypeDefn* d);
  bool classMember(ast::NodeListBuilder &members, ast::NodeListBuilder &friends);
  ast::Defn* enumTypeDef();
  bool enumMember(ast::NodeListBuilder &members);
  ast::Defn* methodDef();
  ast::Node* methodBody();
  void templateParamList(ast::NodeListBuilder& params);
  ast::TypeParameter* templateParam();

  bool requirements(ast::NodeListBuilder& requires);
  ast::Node* requirement();
  ast::Node* requireBinaryOp(ast::Node::Kind kind, ast::Node* left);
  ast::Node* requireCall(ast::Node::Kind kind, ast::Node* fn);
  bool paramList(ast::NodeListBuilder& params);

  ast::Defn* varOrLetDefn();
  ast::ValueDefn* varDeclList(ast::Node::Kind kind);
  ast::ValueDefn* varDecl(ast::Node::Kind kind);

  ast::Node* typeUnion();
  ast::Node* typeTerm(bool allowPartial = false);
  ast::Node* typePrimary(bool allowPartial = false);
  ast::Node* functionType();
  ast::Node* specializedTypeName();
  ast::Node* builtinType(ast::BuiltInType::Type t);

  ast::Node* exprList();
  ast::Node* binary();
  ast::Node* unary();
  ast::Node* primary();
  ast::Node* namedPrimary();
  ast::Node* primarySuffix(ast::Node* expr);
  bool callingArgs(ast::NodeListBuilder &args, source::Location& argsLoc);

  ast::Node* block();
  ast::Node* requiredBlock();
  ast::Node* stmt();
  ast::Node* assignStmt();
  ast::Node* ifStmt();
  ast::Node* whileStmt();
  ast::Node* loopStmt();
  ast::Node* forStmt();
  ast::Node* switchStmt();
  ast::Node* caseExpr();
  ast::Node* caseBody();
  ast::Node* matchStmt();
  ast::Node* tryStmt();
  ast::Node* returnStmt();
  ast::Node* throwStmt();
  ast::Node* localDefn();

  ast::Node* dottedIdent();
  ast::Node* id();
  ast::Node* stringLit();
  ast::Node* charLit();
  ast::Node* integerLit();
  ast::Node* floatLit();

  /** Read the next token. */
  void next();

  /** Match a token. */
  bool match(TokenType tok);

  /** Location of current token. */
  const Location& location() const { return _lexer.tokenLocation(); }

  /** String value of current token. */
  const std::string& tokenValue() const { return _lexer.tokenValue(); }

  /** Make a copy of this string within the current arena. */
  collections::StringRef copyOf(const collections::StringRef& str);

  /** Print an error message about expected tokens. */
  void expected(const collections::StringRef& tokens);

  /** Skip until one of the given tokens is encountered. */
  bool skipUntil(const std::unordered_set<TokenType>& tokens);

  /** Skip to the start of the next defn. */
  bool skipOverDefn();

  /** Create a new node with the given kind and the current token location. Also consume the
      current token. */
  ast::Node* node(ast::Node::Kind kind);

  // Delete copy constructor.
  Parser(const Parser&);
};

}}

#endif
