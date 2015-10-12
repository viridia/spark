// ============================================================================
// Token definitions.
// ============================================================================

#include <spark/config.h>
#include <spark/parse/tokens.h>

#if SPARK_HAVE_STDINT_H
  #include <stdint.h>
#endif

#ifdef DEFINE_TOKEN
#undef DEFINE_TOKEN
#endif

#define DEFINE_TOKEN(x) #x,

namespace spark {
namespace parse {

const char* TOKEN_NAMES[] = {
  #include "spark/parse/tokens.txt"
};

const char* GetTokenName(TokenType tt) {
  uint32_t index = (uint32_t)tt;
  if (index < TOKEN_LAST) {
    return TOKEN_NAMES[index];
  }

  return "<Invalid Token>";
}
  
}}
