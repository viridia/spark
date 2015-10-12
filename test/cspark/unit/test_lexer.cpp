/* ================================================================== *
 * Unit test for spark::source::Lexer
 * ================================================================== */

#include "gtest/gtest.h"
#include "spark/source/location.h"
#include "spark/source/programsource.h"
#include "spark/parse/lexer.h"

namespace spark {
namespace parse {
  
class TestSource : public source::StringSource {
public:
  TestSource(collections::StringRef source)
    : source::StringSource("test.txt", source)
  {}
};

class LexerTest : public testing::Test {
protected:
  LexerTest() {}

  virtual void SetUp() {}
  virtual void TearDown() {}

  /** A function that scans a single token. */
  TokenType LexToken(const char* srcText) {
      TestSource    src(srcText);
      Lexer         lex(&src);

      TokenType result = lex.next();
      EXPECT_EQ(TOKEN_END, lex.next());
      return result;
  }

  /** A function that scans a single token and returns an error. */
  TokenType LexTokenError(const char* srcText) {
      source::StringSource  src("test.txt", srcText);
      Lexer                 lex(&src);
      return lex.next();
  }
};

TEST_F(LexerTest, SingleTokens) {
  // Whitespace
  SCOPED_TRACE("SingleTokens");
  EXPECT_EQ(TOKEN_END, LexToken(""));
  EXPECT_EQ(TOKEN_END, LexToken(" "));
  EXPECT_EQ(TOKEN_END, LexToken("\t"));
  EXPECT_EQ(TOKEN_END, LexToken("\r\n"));

  // Idents
  EXPECT_EQ(TOKEN_ID, LexToken("_"));
  EXPECT_EQ(TOKEN_ID, LexToken("a"));
  EXPECT_EQ(TOKEN_ID, LexToken("z"));
  EXPECT_EQ(TOKEN_ID, LexToken("azAZ_01"));
  EXPECT_EQ(TOKEN_ID, LexToken(" z "));

  // Numbers
  EXPECT_EQ(TOKEN_DEC_INT_LIT, LexToken("0"));
  EXPECT_EQ(TOKEN_DEC_INT_LIT, LexToken(" 0 "));
  EXPECT_EQ(TOKEN_DEC_INT_LIT, LexToken("1"));
  EXPECT_EQ(TOKEN_DEC_INT_LIT, LexToken("9"));
  EXPECT_EQ(TOKEN_DEC_INT_LIT, LexToken("10"));
  EXPECT_EQ(TOKEN_HEX_INT_LIT, LexToken("0x10af"));
  EXPECT_EQ(TOKEN_HEX_INT_LIT, LexToken("0X10af"));
  EXPECT_EQ(TOKEN_FLOAT_LIT, LexToken("0."));
  EXPECT_EQ(TOKEN_FLOAT_LIT, LexToken(" 0. "));
  EXPECT_EQ(TOKEN_FLOAT_LIT, LexToken(".0"));
  EXPECT_EQ(TOKEN_FLOAT_LIT, LexToken("0f"));
  EXPECT_EQ(TOKEN_FLOAT_LIT, LexToken("0e12"));
  EXPECT_EQ(TOKEN_FLOAT_LIT, LexToken("0e+12"));
  EXPECT_EQ(TOKEN_FLOAT_LIT, LexToken("0e-12"));
  EXPECT_EQ(TOKEN_FLOAT_LIT, LexToken("0.0e12f"));

  // Grouping tokens
  EXPECT_EQ(TOKEN_LBRACE, LexToken("{"));
  EXPECT_EQ(TOKEN_RBRACE, LexToken("}"));
  EXPECT_EQ(TOKEN_LPAREN, LexToken("("));
  EXPECT_EQ(TOKEN_RPAREN, LexToken(")"));
  EXPECT_EQ(TOKEN_LBRACKET, LexToken("["));
  EXPECT_EQ(TOKEN_RBRACKET, LexToken("]"));

  // Delimiters
  EXPECT_EQ(TOKEN_SEMI, LexToken(";"));
  EXPECT_EQ(TOKEN_COLON, LexToken(":"));
  EXPECT_EQ(TOKEN_COMMA, LexToken(","));
  EXPECT_EQ(TOKEN_ATSIGN, LexToken("@"));

  // Operator tokens
  EXPECT_EQ(TOKEN_ASSIGN, LexToken("="));
  EXPECT_EQ(TOKEN_ASSIGN_PLUS, LexToken("+="));
  EXPECT_EQ(TOKEN_ASSIGN_MINUS, LexToken("-="));
  EXPECT_EQ(TOKEN_ASSIGN_MUL, LexToken("*="));
  EXPECT_EQ(TOKEN_ASSIGN_DIV, LexToken("/="));
  EXPECT_EQ(TOKEN_ASSIGN_MOD, LexToken("%="));
  EXPECT_EQ(TOKEN_ASSIGN_RSHIFT, LexToken(">>="));
  EXPECT_EQ(TOKEN_ASSIGN_LSHIFT, LexToken("<<="));
  EXPECT_EQ(TOKEN_ASSIGN_BITAND, LexToken("&="));
  EXPECT_EQ(TOKEN_ASSIGN_BITOR, LexToken("|="));
  EXPECT_EQ(TOKEN_ASSIGN_BITXOR, LexToken("^="));
  //EXPECT_EQ(TOKEN_ASSIGNOP, LexToken(""));
  EXPECT_EQ(TOKEN_RETURNS, LexToken("->"));
  EXPECT_EQ(TOKEN_FAT_ARROW, LexToken("=>"));
  EXPECT_EQ(TOKEN_PLUS, LexToken("+"));
  EXPECT_EQ(TOKEN_MINUS, LexToken("-"));
  EXPECT_EQ(TOKEN_MUL, LexToken("*"));
  EXPECT_EQ(TOKEN_DIV, LexToken("/"));
  EXPECT_EQ(TOKEN_AMP, LexToken("&"));
  EXPECT_EQ(TOKEN_MOD, LexToken("%"));
  EXPECT_EQ(TOKEN_VBAR, LexToken("|"));
  EXPECT_EQ(TOKEN_CARET, LexToken("^"));
//   EXPECT_EQ(TOKEN_TILDE, LexToken("~"));
//   EXPECT_EQ(TOKEN_EXCLAM, LexToken("!"));
  EXPECT_EQ(TOKEN_QMARK, LexToken("?"));
  EXPECT_EQ(TOKEN_INC, LexToken("++"));
  EXPECT_EQ(TOKEN_DEC, LexToken("--"));
//   EXPECT_EQ(TOKEN_DOUBLEAMP, LexToken("&&"));
//   EXPECT_EQ(TOKEN_DOUBLEBAR, LexToken("||"));
//   EXPECT_EQ(TOKEN_DOUBLECOLON, LexToken("::"));

  // Relational operators
  EXPECT_EQ(TOKEN_LT, LexToken("<"));
  EXPECT_EQ(TOKEN_GT, LexToken(">"));
  EXPECT_EQ(TOKEN_LE, LexToken("<="));
  EXPECT_EQ(TOKEN_GE, LexToken(">="));
  EXPECT_EQ(TOKEN_EQ, LexToken("=="));
  EXPECT_EQ(TOKEN_NE, LexToken("!="));
  EXPECT_EQ(TOKEN_REF_EQ, LexToken("==="));
  EXPECT_EQ(TOKEN_TYPE_LE, LexToken("<:"));
  EXPECT_EQ(TOKEN_TYPE_GE, LexToken(":>"));

  EXPECT_EQ(TOKEN_LSHIFT, LexToken("<<"));
  EXPECT_EQ(TOKEN_RSHIFT, LexToken(">>"));
  //EXPECT_EQ(TOKEN_SCOPE, LexToken(""));

  // Joiners
  EXPECT_EQ(TOKEN_DOT, LexToken("."));
  EXPECT_EQ(TOKEN_RANGE, LexToken(".."));
  EXPECT_EQ(TOKEN_ELLIPSIS, LexToken("..."));

  // Operator keywords
  EXPECT_EQ(TOKEN_AND, LexToken("and"));
  EXPECT_EQ(TOKEN_OR, LexToken("or"));
  EXPECT_EQ(TOKEN_NOT, LexToken("not"));
  EXPECT_EQ(TOKEN_AS, LexToken("as"));
  EXPECT_EQ(TOKEN_IS, LexToken("is"));
  EXPECT_EQ(TOKEN_IN, LexToken("in"));

  // Access Keywords
  EXPECT_EQ(TOKEN_PUBLIC, LexToken("public"));
  EXPECT_EQ(TOKEN_PRIVATE, LexToken("private"));
  EXPECT_EQ(TOKEN_PROTECTED, LexToken("protected"));
  EXPECT_EQ(TOKEN_INTERNAL, LexToken("internal"));

  // Primtypes
  EXPECT_EQ(TOKEN_BOOL, LexToken("bool"));
  EXPECT_EQ(TOKEN_CHAR, LexToken("char"));
  EXPECT_EQ(TOKEN_INT, LexToken("int"));
  EXPECT_EQ(TOKEN_I8, LexToken("i8"));
  EXPECT_EQ(TOKEN_I16, LexToken("i16"));
  EXPECT_EQ(TOKEN_I32, LexToken("i32"));
  EXPECT_EQ(TOKEN_I64, LexToken("i64"));
  EXPECT_EQ(TOKEN_U8, LexToken("u8"));
  EXPECT_EQ(TOKEN_U16, LexToken("u16"));
  EXPECT_EQ(TOKEN_U32, LexToken("u32"));
  EXPECT_EQ(TOKEN_U64, LexToken("u64"));
  EXPECT_EQ(TOKEN_FLOAT, LexToken("float"));
  EXPECT_EQ(TOKEN_FLOAT32, LexToken("float32"));
  EXPECT_EQ(TOKEN_FLOAT64, LexToken("float64"));

  // Metatypes
  EXPECT_EQ(TOKEN_CLASS, LexToken("class"));
  EXPECT_EQ(TOKEN_STRUCT, LexToken("struct"));
  EXPECT_EQ(TOKEN_ENUM, LexToken("enum"));
  EXPECT_EQ(TOKEN_VAR, LexToken("var"));
  EXPECT_EQ(TOKEN_LET, LexToken("let"));
  EXPECT_EQ(TOKEN_DEF, LexToken("def"));

  EXPECT_EQ(TOKEN_IMPORT, LexToken("import"));

  // Statement keywords
  EXPECT_EQ(TOKEN_IF, LexToken("if"));
  EXPECT_EQ(TOKEN_ELSE, LexToken("else"));
  EXPECT_EQ(TOKEN_LOOP, LexToken("loop"));
  EXPECT_EQ(TOKEN_FOR, LexToken("for"));
  EXPECT_EQ(TOKEN_WHILE, LexToken("while"));
  EXPECT_EQ(TOKEN_RETURN, LexToken("return"));

  EXPECT_EQ(TOKEN_TRY, LexToken("try"));
  EXPECT_EQ(TOKEN_CATCH, LexToken("catch"));
  EXPECT_EQ(TOKEN_FINALLY, LexToken("finally"));
  EXPECT_EQ(TOKEN_SWITCH, LexToken("switch"));
  EXPECT_EQ(TOKEN_MATCH, LexToken("match"));

  // String literals
  EXPECT_EQ(TOKEN_STRING_LIT, LexToken("\"\""));
  EXPECT_EQ(TOKEN_CHAR_LIT, LexToken("'a'"));

  // Erroneous tokens
  EXPECT_EQ(TOKEN_ERROR, LexTokenError("#"));
}

TEST_F(LexerTest, StringLiterals) {
  SCOPED_TRACE("StringLiterals");

  {
    TestSource  src("\"\"");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_STRING_LIT, lex.next());
    EXPECT_EQ((size_t)0, lex.tokenValue().length());
  }

  {
    std::string expected("abc\n\r$");
    TestSource  src("\"abc\\n\\r\\$\"");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_STRING_LIT, lex.next());
    EXPECT_EQ(expected, lex.tokenValue());
  }

  {
    std::string expected("\x01\xAA\xBB");
    TestSource  src("\"\\x01\\xAA\\xBB\"");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_STRING_LIT, lex.next());
    EXPECT_EQ(expected, lex.tokenValue());
  }

#if 0
  {
    std::string expected("\x01\u00AA\u00BB");
    TestSource  src("\"\\x01\\uAA\\uBB\"");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_STRING_LIT, lex.next());
    EXPECT_EQ(expected, lex.tokenValue());
  }

  {
    std::string expected("\u2105");
    TestSource  src("\"\\u2105\"");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_STRING_LIT, lex.next());
    EXPECT_EQ(expected, lex.tokenValue());
  }

  {
    std::string expected("\U00012100");
    TestSource  src("\"\\U00012100\"");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_STRING_LIT, lex.next());
    EXPECT_EQ(expected, lex.tokenValue());
  }
#endif
}

TEST_F(LexerTest, CharLiterals) {
  SCOPED_TRACE("CharLiterals");

  {
    TestSource  src("\'a\'");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_CHAR_LIT, lex.next());
    EXPECT_EQ((size_t)1, lex.tokenValue().length());
  }

  {
    std::string expected("\x01");
    TestSource  src("'\\x01'");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_CHAR_LIT, lex.next());
    EXPECT_EQ(expected, lex.tokenValue());
  }

  {
    std::string expected("\xAA");
    TestSource  src("'\\xAA'");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_CHAR_LIT, lex.next());
    EXPECT_EQ(expected, lex.tokenValue());
  }
#if 0
  {
    std::string expected("000000aa");
    TestSource  src("'\\uAA'");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_CHAR_LIT, lex.next());
    EXPECT_EQ(expected, lex.tokenValue());
  }

  {
    std::string expected("00002100");
    TestSource  src("'\\u2100\'");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_CHAR_LIT, lex.next());
    EXPECT_EQ(expected, lex.tokenValue());
  }

  {
    std::string expected("00012100");
    TestSource  src("'\\U00012100'");
    Lexer       lex(&src);

    EXPECT_EQ(TOKEN_CHAR_LIT, lex.next());
    EXPECT_EQ(expected, lex.tokenValue());
  }
#endif
}

TEST_F(LexerTest, Comments) {
  SCOPED_TRACE("Comments");

  // Comments

  EXPECT_EQ(TOKEN_END, LexToken("/* comment */"));
  EXPECT_EQ(TOKEN_END, LexToken(" /* comment */ "));
  EXPECT_EQ(TOKEN_END, LexToken("//"));
  EXPECT_EQ(TOKEN_END, LexToken("// comment\n"));
  EXPECT_EQ(TOKEN_END, LexToken(" //\n "));
  EXPECT_EQ(TOKEN_DEC_INT_LIT, LexToken("  /* comment */10/* comment */ "));
  EXPECT_EQ(TOKEN_DEC_INT_LIT, LexToken("  /* comment */ 10 /* comment */ "));
  EXPECT_EQ(TOKEN_DEC_INT_LIT, LexToken("  /// comment\n 10 // comment\n "));
  EXPECT_EQ(TOKEN_DEC_INT_LIT, LexToken("  /// comment\n10// comment\n "));

  // Unterminated comment
  EXPECT_EQ(TOKEN_ERROR, LexTokenError("/* comment"));
}

TEST_F(LexerTest, Location) {

  TestSource  src("\n\n   aaaaa    ");
  Lexer       lex(&src);

  EXPECT_EQ(TOKEN_ID, lex.next());

  EXPECT_EQ(3u, lex.tokenLocation().startLine);
  EXPECT_EQ(4u, lex.tokenLocation().startCol);
  EXPECT_EQ(3u, lex.tokenLocation().endLine);
  EXPECT_EQ(9u, lex.tokenLocation().endCol);

  std::string line;
  EXPECT_TRUE(src.getLine(2, line));
  EXPECT_EQ("   aaaaa    ", line);
}

}}
