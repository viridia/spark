// ============================================================================
// sema/essentialtypes.h: Class for creating and managing derived types.
// ============================================================================

#ifndef SPARK_SEMA_TYPES_ESSENTIALTYPES_H
#define SPARK_SEMA_TYPES_ESSENTIALTYPES_H 1

#ifndef SPARK_SEMA_TYPES_TYPESTORE_H
  #include "spark/sema/types/typestore.h"
#endif

namespace spark {
namespace compiler { class Context; }
namespace sema {
namespace types {
using collections::StringRef;

class Essentials {
public:
  enum class Type {
    ANY,
    ENUM,
    OBJECT,

    MAX = OBJECT,
  };

  Essentials(compiler::Context* context);

  void load();
  semgraph::Composite* get(Type id);

private:
  semgraph::Member* findAbsoluteSymbol(const StringRef& path);

  compiler::Context* _context;
  semgraph::Composite* _types[(int)Type::MAX + 1];
};

}}}

#endif
