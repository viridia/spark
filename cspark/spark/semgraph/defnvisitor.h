// ============================================================================
// semgraph/defnvisitor.h: Graph traversal for defns.
// ============================================================================

#ifndef SPARK_SEMGRAPH_DEFNVISITOR_H
#define SPARK_SEMGRAPH_DEFNVISITOR_H 1

#ifndef SPARK_SEMGRAPH_DEFN_H
  #include "spark/semgraph/defn.h"
#endif

namespace spark {
namespace semgraph {

template<typename ReturnType, typename... Args>
class DefnVisitor {
public:
  /** Call the appropriate visitor function for this definition. */
  ReturnType operator()(Defn* d, Args&&... args) {
    return exec(d, std::forward<Args>(args)...);
  }

  /** Call the appropriate visitor function for this definition. */
  ReturnType exec(Defn* d, Args&&... args) {
    switch (d->kind()) {
      case Member::Kind::TYPE:
        return visitTypeDefn(static_cast<TypeDefn*>(d), std::forward<Args>(args)...);
      case Member::Kind::LET:
      case Member::Kind::VAR:
      case Member::Kind::ENUM_VAL:
        return visitValueDefn(static_cast<ValueDefn*>(d), std::forward<Args>(args)...);
      case Member::Kind::PARAM:
        return visitParameter(static_cast<Parameter*>(d), std::forward<Args>(args)...);
      case Member::Kind::TYPE_PARAM:
        return visitTypeParameter(static_cast<TypeParameter*>(d), std::forward<Args>(args)...);
      case Member::Kind::FUNCTION:
        return visitFunction(static_cast<Function*>(d), std::forward<Args>(args)...);
      case Member::Kind::PROPERTY:
        return visitProperty(static_cast<Property*>(d), std::forward<Args>(args)...);
      default:
        assert(false);
    }
  }

  /** Call the appropriate visitor functions for a collection of definitions. */
  void exec(const collections::ArrayRef<Defn*>& defns, Args&&... args) {
    for (Defn* d : defns) {
      exec(d, std::forward<Args>(args)...);
    }
  }

  /** Call the appropriate visitor functions for a collection of definitions. */
  void exec(const collections::ArrayRef<Member*>& defns, Args&&... args) {
    for (Member* m : defns) {
      if (m->kind() >= Member::Kind::TYPE && m->kind() <= Member::Kind::PROPERTY) {
        exec(static_cast<Defn*>(m), std::forward<Args>(args)...);
      }
    }
  }

  virtual ReturnType visitDefn(Defn* d, Args&&... args) { return ReturnType(); }
  virtual ReturnType visitValueDefn(ValueDefn* v, Args&&... args) { return visitDefn(v); }
  virtual ReturnType visitTypeDefn(TypeDefn* t, Args&&... args) { return visitDefn(t); }
  virtual ReturnType visitTypeParameter(TypeParameter* f, Args&&... args) { return visitDefn(f); }
  virtual ReturnType visitParameter(Parameter* f, Args&&... args) { return visitDefn(f); }
  virtual ReturnType visitFunction(Function* f, Args&&... args) { return visitDefn(f); }
  virtual ReturnType visitProperty(Property* p, Args&&... args) { return visitDefn(p); }
};

}}

#endif
