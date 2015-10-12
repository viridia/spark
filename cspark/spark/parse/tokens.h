// ============================================================================
// ident.h: AST representing identifiers
// ============================================================================

#ifndef SPARK_PARSE_TOKENS_H
#define SPARK_PARSE_TOKENS_H 1

#if SPARK_HAVE_OSTREAM
  #include <ostream>
#endif

namespace spark {
namespace parse {

#ifdef DEFINE_TOKEN
#undef DEFINE_TOKEN
#endif

#define DEFINE_TOKEN(x) TOKEN_##x,

enum TokenType {
  #include "tokens.txt"
  TOKEN_LAST
};

// Return the name of the specified token.
const char* GetTokenName(TokenType tt);

// How to print a token type.
inline ::std::ostream& operator<<(::std::ostream& os, TokenType tt) {
  return os << GetTokenName(tt);
}

}}

namespace std {

/** Compute a hash for a StringRef. */
template<>
struct hash<spark::parse::TokenType> {
  inline std::size_t operator()(spark::parse::TokenType value) const {
    return std::hash<int32_t>()((int32_t) value);
  }
};
}

#endif
