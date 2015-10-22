// ============================================================================
// ast/nodevisitor.h: Traversal for ASTs.
// ============================================================================

#ifndef SPARK_AST_NODEVISITOR_H
#define SPARK_AST_NODEVISITOR_H 1

#ifndef SPARK_AST_NODE_H
  #include "spark/ast/node.h"
#endif

#ifndef SPARK_AST_DEFN_H
  #include "spark/ast/defn.h"
#endif

#ifndef SPARK_AST_IDENT_H
  #include "spark/ast/ident.h"
#endif

#ifndef SPARK_AST_LITERAL_H
  #include "spark/ast/literal.h"
#endif

#ifndef SPARK_AST_OPER_H
  #include "spark/ast/oper.h"
#endif

namespace spark {
namespace semgraph {

template<typename ReturnType, typename... Args>
class NodeVisitor {
public:
  /** Call the appropriate visitor function for this definition. */
  ReturnType operator()(Node* n, Args&&... args) {
    return exec(n, std::forward<Args>(args)...);
  }

  /** Call the appropriate visitor function for this definition. */
  ReturnType exec(Node* n, Args&&... args) {
    switch (n->kind()) {
      case Kind::ERROR: return visitError(n, std::forward<Args>(args)...);
      case Kind::ABSENT: return visitAbsent(n, std::forward<Args>(args)...);
      case Kind::NULL: return visitNull(n, std::forward<Args>(args)...);
      case Kind::SELF: return visitSelf(n, std::forward<Args>(args)...);
      case Kind::SUPER: return visitSuper(n, std::forward<Args>(args)...);

      // Idents
      case Kind::IDENT:
          return visitIdent(static_cast<Ident*>(n), std::forward<Args>(args)...);
      case Kind::MEMBER:
          return visitMemberRef(static_cast<MemberRef*>(n), std::forward<Args>(args)...);
      case Kind::SELF_NAME_REF:
          return visitSelfNameRef(static_cast<MemberRef*>(n), std::forward<Args>(args)...);
      case Kind::BUILTIN_ATTRIBUTE: return visitBuiltinAttribute(
          static_cast<BuiltinAttribute*>(n), std::forward<Args>(args)...);
      case Kind::BUILTIN_TYPE: return visitBuiltinType(
          static_cast<BuiltinType*>(n), std::forward<Args>(args)...);
      case Kind::KEYWORD_ARG: return visitKeywordArg(
          static_cast<KeywordArg*>(n), std::forward<Args>(args)...);

      // Literals
      case Kind::BOOLEAN_TRUE: return visitBooleanTrue(n, std::forward<Args>(args)...);
      case Kind::BOOLEAN_FALSE: return visitBooleanFalse(n, std::forward<Args>(args)...);
      case Kind::CHAR_LITERAL:
          return visitCharLiteral(static_cast<TextLiteral*>(n), std::forward<Args>(args)...);
      case Kind::STRING_LITERAL:
          return visitStringLiteral(static_cast<TextLiteral*>(n), std::forward<Args>(args)...);
      case Kind::FLOAT_LITERAL:
          return visitFloatLiteral(static_cast<FloatLiteral*>(n), std::forward<Args>(args)...);
      case Kind::INTEGER_LITERAL:
          return visitIntegerLiteral(static_cast<IntegerLiteral*>(n), std::forward<Args>(args)...);

      // Unary operators
      case Kind::NEGATE:
          return visitNegate(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);
      case Kind::COMPLEMENT:
          return visitComplement(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);
      case Kind::LOGICAL_NOT:
          return visitLogicalNot(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);
      case Kind::PRE_INC:
          return visitPreInc(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);
      case Kind::POST_INC:
          return visitPostInc(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);
      case Kind::PRE_DEC:
          return visitPreDec(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);
      case Kind::POST_DEC:
          return visitPostDec(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);
      case Kind::STATIC:
          return visitStatic(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);
      case Kind::CONST:
          return visitConst(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);
      case Kind::INHERITED_CONST:
          return visitInheritedConst(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);
      case Kind::OPTIONAL:
          return visitOptional(static_cast<UnaryOp*>(n), std::forward<Args>(args)...);

      // Binary operators
      case Kind::ADD:
          return visitAdd(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::SUB:
          return visitSub(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::MUL:
          return visitMul(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::DIV:
          return visitDiv(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::MOD:
          return visitMod(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::BIT_AND:
          return visitBitAnd(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::BIT_OR:
          return visitBitOr(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::BIT_XOR:
          return visitBitXor(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::RSHIFT:
          return visitRShift(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::LSHIFT:
          return visitLShift(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::EQUAL:
          return visitEqual(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::REF_EQUAL:
          return visitRefEqual(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::NOT_EQUAL:
          return visitNotEqual(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::LESS_THAN:
          return visitLessThan(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::GREATER_THAN:
          return visitGreaterThan(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::LESS_THAN_OR_EQUAL:
          return visitLessThanOrEqual(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::GREATER_THAN_OR_EQUAL:
          return visitGreaterThanOrEqual(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::IS_SUB_TYPE:
          return visitIsSubType(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::IS_SUPER_TYPE:
          return visitIsSuperType(static_cast<Oper*>(n), std::forward<Args>(args)...);
      case Kind::ASSIGN:
          return visitAssign(static_cast<Oper*>(n), std::forward<Args>(args)...);

      case Kind::ASSIGN_ADD:
      case Kind::ASSIGN_SUB:
      case Kind::ASSIGN_MUL:
      case Kind::ASSIGN_DIV:
      case Kind::ASSIGN_MOD:
      case Kind::ASSIGN_BIT_AND:
      case Kind::ASSIGN_BIT_OR:
      case Kind::ASSIGN_BIT_XOR:
      case Kind::ASSIGN_RSHIFT:
      case Kind::ASSIGN_LSHIFT:
          return visitAssignOp(static_cast<Oper*>(n), std::forward<Args>(args)...);

  NODE_KIND(LOGICAL_AND)
  NODE_KIND(LOGICAL_OR)
  NODE_KIND(RANGE)
  NODE_KIND(AS_TYPE)
  NODE_KIND(IS)
  NODE_KIND(IS_NOT)
  NODE_KIND(IN)
  NODE_KIND(NOT_IN)
  NODE_KIND(RETURNS)
  NODE_KIND(LAMBDA)
  NODE_KIND(EXPR_TYPE)
  NODE_KIND(RETURN)
  NODE_KIND(THROW)

  /* N-ary operators */

  NODE_KIND(TUPLE)
  NODE_KIND(UNION)
  NODE_KIND(SPECIALIZE)
  NODE_KIND(CALL)
  NODE_KIND(FLUENT_MEMBER)
  NODE_KIND(ARRAY_LITERAL)
  NODE_KIND(LIST_LITERAL)
  NODE_KIND(SET_LITERAL)  NODE_KIND()
  NODE_KIND()

  NODE_KIND(CALL_REQUIRED)
  NODE_KIND(CALL_REQUIRED_STATIC)
  NODE_KIND(LIST)       // List of opions for switch cases, catch blocks, etc.

  /* Misc statements */

  NODE_KIND(BLOCK)      // A statement block
  NODE_KIND(VAR_DEFN)   // A single variable definition (ident, type, init)
  NODE_KIND(ELSE)       // default for match or switch
  NODE_KIND(FINALLY)    // finally block for try

NODE_KIND(OPER_END)

NODE_KIND(CTRL_START)
  NODE_KIND(IF)         // if-statement (test, thenBlock, elseBlock)
  NODE_KIND(WHILE)      // while-statement (test, body)
  NODE_KIND(LOOP)       // loop (body)
  NODE_KIND(FOR)        // for (vars, init, test, step, body)
  NODE_KIND(FOR_IN)     // for in (vars, iter, body)
  NODE_KIND(TRY)        // try (test, body, cases...)
  NODE_KIND(CATCH)      // catch (except-list, body)
  NODE_KIND(SWITCH)     // switch (test, cases...)
  NODE_KIND(CASE)       // switch case (values | [values...]), body
  NODE_KIND(MATCH)      // match (test, cases...)
  NODE_KIND(PATTERN)    // match pattern (pattern, body)
NODE_KIND(CTRL_END)

/* Type operators */

NODE_KIND(MODIFIED)
NODE_KIND(FUNCTION_TYPE)

/* Other statements */

NODE_KIND(BREAK)
NODE_KIND(CONTINUE)

/* Definitions */
/* TODO: Move this outside */

NODE_KIND(VISIBILITY)

NODE_KIND(DEFN)
NODE_KIND(TYPE_DEFN)
NODE_KIND(CLASS_DEFN)
NODE_KIND(STRUCT_DEFN)
NODE_KIND(INTERFACE_DEFN)
NODE_KIND(EXTEND_DEFN)
NODE_KIND(OBJECT_DEFN)
NODE_KIND(ENUM_DEFN)
NODE_KIND(VAR)
NODE_KIND(LET)
NODE_KIND(VAR_LIST)   // A list of variable definitions
NODE_KIND(ENUM_VALUE)
NODE_KIND(PARAMETER)
NODE_KIND(TYPE_PARAMETER)
NODE_KIND(FUNCTION)
NODE_KIND(PROPERTY)
NODE_KIND(DEFN_END)

NODE_KIND(MODULE)
NODE_KIND(IMPORT)

      case Member::TYPE:
        return visitTypeNode(static_cast<TypeNode*>(n), std::forward<Args>(args)...);
      case Member::LET:
      case Member::VAR:
      case Member::ENUM_VAL:
        return visitValueNode(static_cast<ValueNode*>(n), std::forward<Args>(args)...);
      case Member::PARAM:
        return visitParameter(static_cast<Parameter*>(n), std::forward<Args>(args)...);
      case Member::TYPE_PARAM:
        return visitTypeParameter(static_cast<TypeParameter*>(n), std::forward<Args>(args)...);
      case Member::FUNCTION:
        return visitFunction(static_cast<Function*>(n), std::forward<Args>(args)...);
      case Member::PROPERTY:
        return visitProperty(static_cast<Property*>(n), std::forward<Args>(args)...);
      default:
        assert(false);
    }
  }

  /** Call the appropriate visitor functions for a collection of definitions. */
  void exec(const collections::ArrayRef<Node*>& defns, Args&&... args) {
    for (Node* n : defns) {
      exec(n, std::forward<Args>(args)...);
    }
  }

  /** Call the appropriate visitor functions for a collection of definitions. */
  void exec(const collections::ArrayRef<Member*>& defns, Args&&... args) {
    for (Member* m : defns) {
      if (m->kind() >= Member::TYPE && m->kind() <= Member::PROPERTY) {
        exec(static_cast<Node*>(m), std::forward<Args>(args)...);
      }
    }
  }

  virtual ReturnType visitNode(Node* n, Args&&... args) { return ReturnType(); }
  virtual ReturnType visitValueNode(ValueNode* v, Args&&... args) { return visitNode(v); }
  virtual ReturnType visitTypeNode(TypeNode* t, Args&&... args) { return visitNode(t); }
  virtual ReturnType visitTypeParameter(TypeParameter* f, Args&&... args) { return visitNode(f); }
  virtual ReturnType visitParameter(Parameter* f, Args&&... args) { return visitNode(f); }
  virtual ReturnType visitFunction(Function* f, Args&&... args) { return visitNode(f); }
  virtual ReturnType visitProperty(Property* p, Args&&... args) { return visitNode(p); }
};

}}

#endif
