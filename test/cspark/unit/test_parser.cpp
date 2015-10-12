/* ================================================================== *
 * Unit test for spark::source::Lexer
 * ================================================================== */

#include "gtest/gtest.h"
#include "spark/ast/node.h"
#include "spark/ast/ident.h"
#include "spark/ast/literal.h"
#include "spark/parse/parser.h"
#include "mocks.h"

namespace spark {
namespace parse {
using spark::ast::Node;
using spark::error::MockReporter;
  
class ParserTest : public testing::Test {
protected:
  ParserTest() {}

  virtual void SetUp() {}
  virtual void TearDown() {}
  
  support::Arena    _arena;
  MockReporter      _reporter;

  template <class T>
  T* parse(T* (Parser::*parseFunc)(), const char* srctext, int expectedErrors) {
//     if (expectedErrors != 0) {
//       diag.setMinSeverity(error::Off);
//     }

    source::StringSource src("test.txt", srctext);
    Parser parser(_reporter, &src, _arena);
    T* result = (parser.*parseFunc)();
//     if (!expectedErrors) {
//       EXPECT_TRUE(result != NULL) << "[src = " << srctext << "]";
//     }

//     if (expectedErrors != diag.getErrorCount()) {
//       EXPECT_EQ(expectedErrors, diag.getErrorCount()) << "[src = " << srctext << "]";
//     }
// 
//     diag.reset();
//     diag.setMinSeverity(Diagnostics::DEBUG);
    return result;
  }

  Node* parseExpression(const char* srctext, int expectedErrors = 0) {
    return parse(&Parser::expression, srctext, expectedErrors);
  }

  Node* parseType(const char* srctext, int expectedErrors = 0) {
    return parse(&Parser::typeExpression, srctext, expectedErrors);
  }

//   Stmt* parseStatement(const char * srctext, int expectedErrors = 0) {
//     return parse(&Parser::statement, srctext, expectedErrors);
//   }
// 
//   ASTDecl * parseDeclaration(const char * srctext, int expectedErrors = 0) {
//     return parse(&Parser::declaration, srctext, expectedErrors);
//   }
};

TEST_F(ParserTest, Terminals) {
  ast::Node* n;

  // true
  n = parseExpression("true");
  ASSERT_EQ(ast::Node::KIND_BOOLEAN_TRUE, n->kind());
  ASSERT_EQ(1, n->location().startLine);
  ASSERT_EQ(1, n->location().startCol);
  ASSERT_EQ(1, n->location().endLine);
  ASSERT_EQ(5, n->location().endCol);
  
  // true
  n = parseExpression("false");
  ASSERT_EQ(ast::Node::KIND_BOOLEAN_FALSE, n->kind());
  ASSERT_EQ(1, n->location().startLine);
  ASSERT_EQ(1, n->location().startCol);
  ASSERT_EQ(1, n->location().endLine);
  ASSERT_EQ(6, n->location().endCol);
  
  // super
  n = parseExpression("super");
  ASSERT_EQ(ast::Node::KIND_SUPER, n->kind());
  ASSERT_EQ(1, n->location().startLine);
  ASSERT_EQ(1, n->location().startCol);
  ASSERT_EQ(1, n->location().endLine);
  ASSERT_EQ(6, n->location().endCol);
  
  // true
  n = parseExpression("self");
  ASSERT_EQ(ast::Node::KIND_SELF, n->kind());
  ASSERT_EQ(1, n->location().startLine);
  ASSERT_EQ(1, n->location().startCol);
  ASSERT_EQ(1, n->location().endLine);
  ASSERT_EQ(5, n->location().endCol);
  
  // true
  n = parseExpression("null");
  ASSERT_EQ(ast::Node::KIND_NULL, n->kind());
  ASSERT_EQ(1, n->location().startLine);
  ASSERT_EQ(1, n->location().startCol);
  ASSERT_EQ(1, n->location().endLine);
  ASSERT_EQ(5, n->location().endCol);
  
  // id
  n = parseExpression("X");
  ASSERT_EQ(ast::Node::KIND_IDENT, n->kind());
  ast::Ident* id = static_cast<ast::Ident*>(n);
  ASSERT_EQ("X", id->name());
  ASSERT_EQ(1, id->location().startLine);
  ASSERT_EQ(1, id->location().startCol);
  ASSERT_EQ(1, id->location().endLine);
  ASSERT_EQ(2, id->location().endCol);
  
  // integer
  n = parseExpression("23");
  ASSERT_EQ(ast::Node::KIND_INTEGER_LITERAL, n->kind());
  ast::IntegerLiteral* il = static_cast<ast::IntegerLiteral*>(n);
  ASSERT_EQ(23, il->value());
  ASSERT_EQ(1, n->location().startLine);
  ASSERT_EQ(1, n->location().startCol);
  ASSERT_EQ(1, n->location().endLine);
  ASSERT_EQ(3, n->location().endCol);

  n = parseExpression("0x10");
  ASSERT_EQ(ast::Node::KIND_INTEGER_LITERAL, n->kind());
  il = static_cast<ast::IntegerLiteral*>(n);
  ASSERT_EQ(16, il->value());

  n = parseExpression("0x100000000");
  ASSERT_EQ(ast::Node::KIND_INTEGER_LITERAL, n->kind());
  il = static_cast<ast::IntegerLiteral*>(n);
  ASSERT_EQ(4294967296, il->value());

  // Floats
  n = parseExpression("1.0");
  ASSERT_EQ(ast::Node::KIND_FLOAT_LITERAL, n->kind());
//   ASSERT_FLOAT_EQ(1.0, dyn_cast<ASTDoubleLiteral>(ast)->value().convertToDouble());
//   EXPECT_EQ(0u, ast->location().begin);
//   EXPECT_EQ(3u, ast->location().end);

//   ast = parseExpression("1.0f");
//   ASSERT_EQ(ASTNode::LitFloat, ast->nodeType());
//   ASSERT_FLOAT_EQ(1.0, dyn_cast<ASTFloatLiteral>(ast)->value().convertToFloat());
// 
//   ast = parseExpression("5.0e3f");
//   ASSERT_EQ(ASTNode::LitFloat, ast->nodeType());
//   ASSERT_FLOAT_EQ(5.0e3f, dyn_cast<ASTFloatLiteral>(ast)->value().convertToFloat());

  // Character literals
  n = parseExpression("'c'");
  ASSERT_EQ(ast::Node::KIND_CHAR_LITERAL, n->kind());
  ast::TextLiteral* tl = static_cast<ast::TextLiteral*>(n);
  ASSERT_EQ("c", tl->value());
//   EXPECT_EQ(0u, ast->location().begin);
//   EXPECT_EQ(3u, ast->location().end);

  // String literals
  n = parseExpression("\"c\"");
  ASSERT_EQ(ast::Node::KIND_STRING_LITERAL, n->kind());
  tl = static_cast<ast::TextLiteral*>(n);
  ASSERT_EQ("c", tl->value());
//   EXPECT_EQ(0u, ast->location().begin);
//   EXPECT_EQ(3u, ast->location().end);
}

}}
