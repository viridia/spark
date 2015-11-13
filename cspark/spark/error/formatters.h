// ============================================================================
// formatters.h: Various formatting helpers.
// ============================================================================

#ifndef SPARK_ERROR_FORMATTERS_H
#define SPARK_ERROR_FORMATTERS_H 1

#ifndef SPARK_SEMGRAPH_TYPEVISITOR_H
  #include "spark/semgraph/typevisitor.h"
#endif

#if SPARK_HAVE_SSTREAM
  #include <sstream>
#endif

namespace spark {
namespace error {
using semgraph::Type;

/** Format helper for type expressions. */
class FormattedType : public semgraph::TypeVisitor<void, ::std::ostream&> {
public:
  FormattedType(Type* type) : _type(type) {}

  void format(::std::ostream& os) const { const_cast<FormattedType*>(this)->exec(_type, os); }

  void visitType(Type *t, ::std::ostream& os);
private:
  Type* _type;
};

inline FormattedType formatted(Type* t) { return FormattedType(t); }

// Format helper for types.
inline ::std::ostream& operator<<(::std::ostream& os, const FormattedType& ft) {
  ft.format(os);
  return os;
}

}}

#endif
