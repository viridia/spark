// ============================================================================
// semgraph/type.h: Graph traversal for types.
// ============================================================================

#ifndef SPARK_SEMGRAPH_TYPEVISITOR_H
#define SPARK_SEMGRAPH_TYPEVISITOR_H 1

#ifndef SPARK_SEMGRAPH_TYPE_H
  #include "spark/semgraph/type.h"
#endif

#ifndef SPARK_SEMGRAPH_PRIMITIVETYPE_H
  #include "spark/semgraph/primitivetype.h"
#endif

namespace spark {
namespace semgraph {

template<typename ReturnType, typename... Args>
class TypeVisitor {
public:
  /** Call the appropriate visitor function for this definition. */
  ReturnType operator()(Type* t, Args&&... args) {
    return exec(t, std::forward<Args>(args)...);
  }

  /** Call the appropriate visitor function for this definition. */
  ReturnType exec(Type* t, Args&&... args) {
    switch (t->kind()) {
      case Type::Kind::INVALID:
        return visitInvalidType(t, std::forward<Args>(args)...);
      case Type::Kind::IGNORED:
        return visitIgnoredType(t, std::forward<Args>(args)...);
      case Type::Kind::NO_RETURN:
        return visitNoReturnType(t, std::forward<Args>(args)...);
      case Type::Kind::VOID:
        return visitVoidType(static_cast<VoidType*>(t), std::forward<Args>(args)...);
      case Type::Kind::NULLPTR:
        return visitNullPtrType(static_cast<NullPtrType*>(t), std::forward<Args>(args)...);
      case Type::Kind::BOOLEAN:
        return visitBooleanType(static_cast<BooleanType*>(t), std::forward<Args>(args)...);
      case Type::Kind::INTEGER:
        return visitIntegerType(static_cast<IntegerType*>(t), std::forward<Args>(args)...);
      case Type::Kind::FLOAT:
        return visitFloatType(static_cast<FloatType*>(t), std::forward<Args>(args)...);
      case Type::Kind::CLASS:
        return visitClass(static_cast<Composite*>(t), std::forward<Args>(args)...);
      case Type::Kind::STRUCT:
        return visitStruct(static_cast<Composite*>(t), std::forward<Args>(args)...);
      case Type::Kind::INTERFACE:
        return visitInterface(static_cast<Composite*>(t), std::forward<Args>(args)...);
      case Type::Kind::ENUM:
        return visitEnum(static_cast<Composite*>(t), std::forward<Args>(args)...);
      case Type::Kind::EXTENSION:
        return visitExtension(static_cast<Composite*>(t), std::forward<Args>(args)...);
      case Type::Kind::UNION:
        return visitUnionType(static_cast<UnionType*>(t), std::forward<Args>(args)...);
      case Type::Kind::TUPLE:
        return visitTupleType(static_cast<TupleType*>(t), std::forward<Args>(args)...);
      case Type::Kind::FUNCTION:
        return visitFunctionType(static_cast<FunctionType*>(t), std::forward<Args>(args)...);
      case Type::Kind::TYPE_VAR:
        assert(false && "Implement type variable.");
      case Type::Kind::ALIAS:
        assert(false && "Implement alias.");
      case Type::Kind::CONST:
        assert(false && "Implement const.");
      case Type::Kind::SPECIALIZED:
        assert(false && "Implement specialized.");
      case Type::Kind::VALUE_PARAM:
        assert(false && "Implement value param.");
      case Type::Kind::TYPESET:
        assert(false && "Implement typeset.");
    }
  }

  /** Call the appropriate visitor functions for a collection of definitions. */
  void exec(const collections::ArrayRef<Type*>& types, Args&&... args) {
    for (Type* t : types) {
      exec(t, std::forward<Args>(args)...);
    }
  }

  virtual ReturnType visitType(Type* t, Args&&... args) { return ReturnType(); }
  virtual ReturnType visitInvalidType(Type* t, Args&&... args) {
    return visitType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitIgnoredType(Type* t, Args&&... args) {
    return visitType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitPrimitiveType(PrimitiveType* t, Args&&... args) {
    return visitType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitComposite(Composite* t, Args&&... args) {
    return visitType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitNoReturnType(Type* t, Args&&... args) {
    return visitType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitVoidType(VoidType* t, Args&&... args) {
    return visitPrimitiveType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitBooleanType(BooleanType* t, Args&&... args) {
    return visitPrimitiveType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitIntegerType(IntegerType* t, Args&&... args) {
    return visitPrimitiveType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitFloatType(FloatType* t, Args&&... args) {
    return visitPrimitiveType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitNullPtrType(NullPtrType* t, Args&&... args) {
    return visitPrimitiveType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitClass(Composite* t, Args&&... args) {
    return visitComposite(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitStruct(Composite* t, Args&&... args) {
    return visitComposite(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitInterface(Composite* t, Args&&... args) {
    return visitComposite(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitEnum(Composite* t, Args&&... args) {
    return visitComposite(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitExtension(Composite* t, Args&&... args) {
    return visitComposite(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitUnionType(UnionType* t, Args&&... args) {
    return visitType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitTupleType(TupleType* t, Args&&... args) {
    return visitType(t, std::forward<Args>(args)...);
  }
  virtual ReturnType visitFunctionType(FunctionType* t, Args&&... args) {
    return visitType(t, std::forward<Args>(args)...);
  }
};

}}

#endif
