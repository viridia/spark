#include "spark/ast/node.h"
#include "spark/ast/defn.h"
#include "spark/ast/ident.h"
#include "spark/ast/literal.h"
#include "spark/ast/module.h"
#include "spark/ast/oper.h"
#include "spark/error/reporter.h"
#include "spark/scope/closematch.h"
#include "spark/scope/scopestack.h"
#include "spark/sema/names/fillmemberset.h"
#include "spark/sema/names/memberlookup.h"
#include "spark/sema/names/resolveexprs.h"
#include "spark/sema/names/resolvetypes.h"
#include "spark/sema/names/subject.h"
#include "spark/semgraph/expr.h"
#include "spark/semgraph/module.h"
#include "spark/semgraph/primitivetype.h"
#include "spark/support/arena.h"
#include "spark/support/casting.h"

namespace spark {
namespace sema {
namespace names {
using namespace semgraph;
using support::dyn_cast;

Expr* ResolveExprs::exec(const ast::Node* node) {
  switch (node->kind()) {
    case ast::Kind::IDENT:
      return visitIdent(static_cast<const ast::Ident*>(node), false);
    case ast::Kind::MEMBER:
      return visitMemberRef(static_cast<const ast::MemberRef*>(node));
    case ast::Kind::SELF_NAME_REF:
      return visitSelfRef(static_cast<const ast::UnaryOp*>(node));
    case ast::Kind::SELF:
      return visitSelf(node);
    case ast::Kind::BUILTIN_TYPE:
      return visitBuiltinType(static_cast<const ast::BuiltInType*>(node));
    case ast::Kind::KEYWORD_ARG:
      return visitKeywordArg(static_cast<const ast::Oper*>(node));
    case ast::Kind::NULL_LITERAL:
      return visitNullLiteral(node);
    case ast::Kind::BOOLEAN_TRUE:
      return visitBooleanLiteral(node, true);
    case ast::Kind::BOOLEAN_FALSE:
      return visitBooleanLiteral(node, false);
    case ast::Kind::INTEGER_LITERAL:
      return visitIntegerLiteral(static_cast<const ast::IntegerLiteral*>(node));
    case ast::Kind::FLOAT_LITERAL:
      return visitFloatLiteral(static_cast<const ast::FloatLiteral*>(node));
    case ast::Kind::STRING_LITERAL:
      return visitStringLiteral(static_cast<const ast::TextLiteral*>(node));
    case ast::Kind::CHAR_LITERAL:
      return visitCharLiteral(static_cast<const ast::TextLiteral*>(node));
    case ast::Kind::NEGATE:
      return visitUnaryOp(static_cast<const ast::UnaryOp*>(node), Expr::Kind::NEGATE);
    case ast::Kind::COMPLEMENT:
      return visitUnaryOp(static_cast<const ast::UnaryOp*>(node), Expr::Kind::COMPLEMENT);
    case ast::Kind::LOGICAL_NOT:
      return visitUnaryOp(static_cast<const ast::UnaryOp*>(node), Expr::Kind::LOGICAL_NOT);
    case ast::Kind::OPTIONAL:
      return visitUnaryOp(static_cast<const ast::UnaryOp*>(node), Expr::Kind::OPTIONAL);
//   STATIC,
    case ast::Kind::CONST:
    case ast::Kind::PROVISIONAL_CONST:
      return visitConst(static_cast<const ast::UnaryOp*>(node));
//   OPTIONAL,
    case ast::Kind::ADD:
      return visitAdd(static_cast<const ast::Oper*>(node));
    case ast::Kind::SUB:
      return visitSub(static_cast<const ast::Oper*>(node));
    case ast::Kind::MUL:
      return visitMul(static_cast<const ast::Oper*>(node));
    case ast::Kind::DIV:
      return visitDiv(static_cast<const ast::Oper*>(node));
    case ast::Kind::MOD:
      return visitMod(static_cast<const ast::Oper*>(node));
    case ast::Kind::BIT_AND:
      return visitBitAnd(static_cast<const ast::Oper*>(node));
    case ast::Kind::BIT_OR:
      return visitBitOr(static_cast<const ast::Oper*>(node));
    case ast::Kind::BIT_XOR:
      return visitBitXor(static_cast<const ast::Oper*>(node));
    case ast::Kind::RSHIFT:
      return visitRShift(static_cast<const ast::Oper*>(node));
    case ast::Kind::LSHIFT:
      return visitLShift(static_cast<const ast::Oper*>(node));

    case ast::Kind::EQUAL:
      return visitEqual(static_cast<const ast::Oper*>(node));
    case ast::Kind::REF_EQUAL:
      return visitRefEqual(static_cast<const ast::Oper*>(node));
    case ast::Kind::NOT_EQUAL:
      return visitNotEqual(static_cast<const ast::Oper*>(node));
    case ast::Kind::LESS_THAN:
      return visitLessThan(static_cast<const ast::Oper*>(node));
    case ast::Kind::GREATER_THAN:
      return visitGreaterThan(static_cast<const ast::Oper*>(node));
    case ast::Kind::LESS_THAN_OR_EQUAL:
      return visitLessThanOrEqual(static_cast<const ast::Oper*>(node));
    case ast::Kind::GREATER_THAN_OR_EQUAL:
      return visitGreaterThanOrEqual(static_cast<const ast::Oper*>(node));
//   IS_SUB_TYPE,
//   IS_SUPER_TYPE,
    case ast::Kind::ASSIGN:
      return visitAssign(static_cast<const ast::Oper*>(node));
    case ast::Kind::ASSIGN_ADD:
      return visitAssignAdd(static_cast<const ast::Oper*>(node));
    case ast::Kind::ASSIGN_SUB:
      return visitAssignSub(static_cast<const ast::Oper*>(node));
    case ast::Kind::ASSIGN_MUL:
      return visitAssignMul(static_cast<const ast::Oper*>(node));
    case ast::Kind::ASSIGN_DIV:
      return visitAssignDiv(static_cast<const ast::Oper*>(node));
    case ast::Kind::ASSIGN_MOD:
      return visitAssignMod(static_cast<const ast::Oper*>(node));
    case ast::Kind::ASSIGN_BIT_AND:
      return visitAssignBitAnd(static_cast<const ast::Oper*>(node));
    case ast::Kind::ASSIGN_BIT_OR:
      return visitAssignBitOr(static_cast<const ast::Oper*>(node));
    case ast::Kind::ASSIGN_BIT_XOR:
      return visitAssignBitXor(static_cast<const ast::Oper*>(node));
    case ast::Kind::ASSIGN_RSHIFT:
      return visitAssignRShift(static_cast<const ast::Oper*>(node));
    case ast::Kind::ASSIGN_LSHIFT:
      return visitAssignLShift(static_cast<const ast::Oper*>(node));
    case ast::Kind::LOGICAL_AND:
      return visitLogicalAnd(static_cast<const ast::Oper*>(node));
    case ast::Kind::LOGICAL_OR:
      return visitLogicalOr(static_cast<const ast::Oper*>(node));
    case ast::Kind::PRE_INC:
      return visitPreInc(static_cast<const ast::UnaryOp*>(node));
    case ast::Kind::POST_INC:
      return visitPostInc(static_cast<const ast::UnaryOp*>(node));
    case ast::Kind::PRE_DEC:
      return visitPreDec(static_cast<const ast::UnaryOp*>(node));
    case ast::Kind::POST_DEC:
      return visitPostDec(static_cast<const ast::UnaryOp*>(node));
    case ast::Kind::RANGE:
      return visitRange(static_cast<const ast::Oper*>(node));
    case ast::Kind::AS_TYPE:
      return visitAsType(static_cast<const ast::Oper*>(node));
    case ast::Kind::IS:
      return visitIs(static_cast<const ast::Oper*>(node));
    case ast::Kind::IS_NOT:
      return visitIsNot(static_cast<const ast::Oper*>(node));
//   IN,
//   NOT_IN,
//   RETURNS,
//   LAMBDA,
//   EXPR_TYPE,
    case ast::Kind::RETURN:
      return visitReturn(static_cast<const ast::UnaryOp*>(node));
    case ast::Kind::THROW:
      return visitThrow(static_cast<const ast::UnaryOp*>(node));
    case ast::Kind::TUPLE:
      return visitTuple(static_cast<const ast::Oper*>(node));
    case ast::Kind::UNION:
      return visitUnion(static_cast<const ast::Oper*>(node));
    case ast::Kind::SPECIALIZE:
      return visitSpecialize(static_cast<const ast::Oper*>(node));
    case ast::Kind::CALL:
      return visitCall(static_cast<const ast::Oper*>(node));

//   FLUENT_MEMBER,
//   ARRAY_LITERAL,
//   LIST_LITERAL,
//   SET_LITERAL,

//   /* Misc statements */
    case ast::Kind::BLOCK:
      return visitBlock(static_cast<const ast::Oper*>(node));
    case ast::Kind::IF:
      return visitIf(static_cast<const ast::ControlStmt*>(node));
    case ast::Kind::WHILE:
      return visitWhile(static_cast<const ast::ControlStmt*>(node));
    case ast::Kind::LOOP:
      return visitLoop(static_cast<const ast::ControlStmt*>(node));
    case ast::Kind::FOR:
      return visitFor(static_cast<const ast::ControlStmt*>(node));
    case ast::Kind::FOR_IN:
      return visitForIn(static_cast<const ast::ControlStmt*>(node));
    case ast::Kind::SWITCH:
      return visitSwitch(static_cast<const ast::ControlStmt*>(node));
    case ast::Kind::MATCH:
      return visitMatch(static_cast<const ast::ControlStmt*>(node));
//   TRY,        // try (test, body, cases...)
//   CATCH,      // catch (except-list, body)
//   FINALLY,    // finally block for try
    case ast::Kind::VAR:
    case ast::Kind::LET:
      return visitLocalVar(static_cast<const ast::ValueDefn*>(node));
    case ast::Kind::BREAK:
      return visitBreak(node);
    case ast::Kind::CONTINUE:
      return visitContinue(node);

  /* Type operators */
//   MODIFIED,
    case ast::Kind::FUNCTION_TYPE:
      return visitFunctionType(static_cast<const ast::Oper*>(node));

    default:
      _reporter.error(node->location()) << "Invalid AST node kind: " << node->kind();
      assert(false && "Implement node kind.");
      return NULL;
  }
}

Expr* ResolveExprs::visitIdent(const ast::Ident* ident, bool emptyResultOk) {
  scope::NameLookupResult lookupResult;
  _scopeStack->find(ident->name(), lookupResult);
  auto mset = new (_arena) MemberSet(_arena.copyOf(ident->name()));
  mset->setStem(lookupResult.stem);
  mset->setLocation(ident->location());
  if (lookupResult.members.empty()) {
    if (emptyResultOk) {
      return mset;
    }
    scope::CloseMatchFinder matcher(ident->name());
    _scopeStack->forAllNames(matcher);
    _scopeStack->find(ident->name(), lookupResult);
    if (matcher.closest().empty()) {
      _reporter.error(ident->location()) << "Name '" << ident->name() << "' not found.";
    } else {
      _reporter.error(ident->location()) << "Name '" << ident->name() <<
          "' not found, did you mean '" << matcher.closest() << "'?";
    }
    return &Expr::ERROR;
  }
  FillMemberSet fms(_reporter, _subject, _arena);
  fms.fill(mset, lookupResult.members);
  return mset;
}

Expr* ResolveExprs::visitMemberRef(const ast::MemberRef* ast) {
  Expr* stem = exec(ast->base());
  if (Expr::isError(stem)) {
    return stem;
  }

  if (stem->kind() == Expr::Kind::MEMBER_SET) {
    auto stemMembers = static_cast<MemberSet*>(stem);
    if (stemMembers->genus() == MemberSet::Genus::NAMESPACE ||
        stemMembers->genus() == MemberSet::Genus::TYPE) {
      //std::vector<scope::SymbolScope*> scopes;
      collections::SmallSet<Member*, 8> members;

      MemberLookup memberLookup(_reporter, _typeStore);
      memberLookup.lookup(ast->name(), stemMembers->members(), true, members);

//       interceptScopes = self.nameLookup.getInterceptScopesForMemberList(base)
//       scopes = self.nameLookup.getStaticScopesForMemberList(base.getMembers())
      // lookup the name in all of the scopes
//       members = []
//       for s in interceptScopes:
//         members.extend(s.lookupName(name))

//       if (members.empty()) {
//         for (auto s : scopes) {
//           s->lookupName(ast->name(), members);
//         }
//       }

#if 0
      // Handle static, no-arg properties with templated class param
      if (stemMembers->genus() == MemberSet::Genus::TYPE) {
//     if (isinstance(m, graph.Property)
//         and m.isStatic()
//         and len(m.getParams()) == 0
//         and len(m.getTypeParams()) == 1):
//       assert len(base.getMembers()) == 1
//       baseMember = base.getMembers()[0]
//       im = graph.InstantiatedMember().setName(m.getName()).setBase(m)
//       envir = {}
//       envir[m.getTypeParams()[0].getTypeVar()] = defns.getMemberType(baseMember)
//       im.setEnv(self.typeStore.newEnv(envir))
//       return im
//     else:
//       return m
        }
        assert(false && "implement");
//         members = [self.instantiateNoArgPropertyWithClassArg(m, base) for m in members]
      }
#endif

      if (members.empty()) {
        scope::CloseMatchFinder matcher(ast->name());
        memberLookup.forAllNames(stemMembers->members(), matcher);
        if (stemMembers->members().size() == 1) {
          Member* baseMember = stemMembers->members()[0];
          if (matcher.closest().empty()) {
            _reporter.error(ast->location()) << "Name '" << ast->name() << "' not found in " <<
                baseMember->qualifiedName() << "'.";
          } else {
            _reporter.error(ast->location()) << "Name '" << ast->name() << "' not found in " <<
                baseMember->qualifiedName() << "', did you mean '" << matcher.closest() << "'?";
          }
        } else {
          _reporter.error(ast->location()) << "Name '" << ast->name() << "' not found in:";
          for (auto m : stemMembers->members()) {
            if (auto d = m->asDefn()) {
              _reporter.info(d->location()) << d->qualifiedName();
            } else {
              _reporter.info() << m->qualifiedName();
            }
          }
        }
        return &Expr::ERROR;
      }

      auto mset = new (_arena) MemberSet(ast->name());
      mset->setLocation(ast->location());
      mset->setStem(stem);
      FillMemberSet fms(_reporter, _subject, _arena);
      fms.fill(mset, members);
      return mset;
    }
  }

  // Create an incomplete member list - we'll fill it in later.
  auto mset = new (_arena) MemberSet(ast->name());
  mset->setLocation(ast->location());
  mset->setStem(stem);
  return mset;
}

Expr* ResolveExprs::visitSelfRef(const ast::UnaryOp* ast) {
  assert(ast->arg()->kind() == ast::Kind::IDENT);
  auto mset = new (_arena) MemberSet(
      _arena.copyOf(static_cast<const ast::Ident*>(ast->arg())->name()));
  mset->setLocation(ast->location());
  mset->setStem(new (_arena) Expr(Expr::Kind::SELF, ast->location()));
  return mset;
}

Expr* ResolveExprs::visitSelf(const ast::Node* ast) {
  return new (_arena) Expr(Expr::Kind::SELF, ast->location());
}

Expr* ResolveExprs::visitBuiltinType(const ast::BuiltInType* ast) {
  TypeDefn* td = nullptr;
  switch (ast->type()) {
    case ast::BuiltInType::VOID: td = semgraph::VoidType::VOID.defn(); break;
    case ast::BuiltInType::BOOL: td = semgraph::BooleanType::BOOL.defn(); break;
    case ast::BuiltInType::CHAR: td = semgraph::IntegerType::CHAR.defn(); break;
    case ast::BuiltInType::I8: td = semgraph::IntegerType::I8.defn(); break;
    case ast::BuiltInType::I16: td = semgraph::IntegerType::I16.defn(); break;
    case ast::BuiltInType::I32: td = semgraph::IntegerType::I32.defn(); break;
    case ast::BuiltInType::I64: td = semgraph::IntegerType::I64.defn(); break;
    case ast::BuiltInType::INT:
      // TODO: This should really be a type alias - we want to keep it separate.
      // TODO: Select size based on target machine pointer width
      if (sizeof(void*) == 4) {
        td = semgraph::IntegerType::I32.defn();
      } else {
        td = semgraph::IntegerType::I64.defn();
      }
      break;
    case ast::BuiltInType::U8: td = semgraph::IntegerType::U8.defn(); break;
    case ast::BuiltInType::U16: td = semgraph::IntegerType::U16.defn(); break;
    case ast::BuiltInType::U32: td = semgraph::IntegerType::U32.defn(); break;
    case ast::BuiltInType::U64: td = semgraph::IntegerType::U64.defn(); break;
    case ast::BuiltInType::UINT:
      // TODO: Select size based on target machine pointer width
      if (sizeof(void*) == 4) {
        td = semgraph::IntegerType::U32.defn();
      } else {
        td = semgraph::IntegerType::U64.defn();
      }
      break;
    case ast::BuiltInType::F32: td = semgraph::FloatType::F32.defn(); break;
    case ast::BuiltInType::F64: td = semgraph::FloatType::F64.defn(); break;
    default:
      assert(false && "Invalid builtin type.");
  }

  support::ArrayBuilder<Member*> builder(_arena, { td });
  auto mset = new (_arena) MemberSet(td->name());
  mset->setGenus(MemberSet::Genus::TYPE);
  mset->setLocation(ast->location());
  mset->setMembers(builder.build());
  return mset;
}

Expr* ResolveExprs::visitKeywordArg(const ast::Oper* node) {
  auto kw = node->operands()[0];
  auto arg = exec(node->operands()[1]);
  if (Expr::isError(arg)) {
    return arg;
  }
  if (kw->kind() != ast::Kind::IDENT) {
    _reporter.error(kw->location()) << "Expected argument keyword to be an identifier.";
    return &Expr::ERROR;
  }
  return new (_arena) KeywordArg(
      node->location(), _arena.copyOf(static_cast<const ast::Ident*>(kw)->name()), arg);
}

Expr* ResolveExprs::visitNullLiteral(const ast::Node* node) {
  return new (_arena) Expr(Expr::Kind::NIL, node->location());
}

Expr* ResolveExprs::visitBooleanLiteral(const ast::Node* ast, bool value) {
  return new (_arena) BooleanLiteral(ast->location(), value);
}

Expr* ResolveExprs::visitIntegerLiteral(const ast::IntegerLiteral* ast) {
  return new (_arena) IntegerLiteral(ast->location(), ast->value(), ast->isUnsigned());
//   def visitIntegerLiteral(self, node):
//     sint = graph.IntegerLiteral().setLocation(node.location)
//     s = node.strValue
//     if s.startswith('0x') or s.startswith('0X'):
//       graphtools.encodeInt(sint, int(s[2:], 16))
//     else:
//       graphtools.encodeInt(sint, int(s))
//     if node.unsigned:
//       sint.setUnsigned(True)
//     return sint
}

Expr* ResolveExprs::visitFloatLiteral(const ast::FloatLiteral* ast) {
  return new (_arena) FloatLiteral(ast->location(), ast->value());
}

Expr* ResolveExprs::visitStringLiteral(const ast::TextLiteral* ast) {
  return new (_arena) StringLiteral(ast->location(), _arena.copyOf(ast->value()));
}

Expr* ResolveExprs::visitCharLiteral(const ast::TextLiteral* ast) {
  // TODO: Get the full unicode value.
  assert(ast->value().size() > 0);
  return new (_arena) IntegerLiteral(ast->location(), ast->value()[0], false);
}

Expr* ResolveExprs::visitUnaryOp(const ast::UnaryOp* ast, Expr::Kind kind) {
  Expr* arg = exec(ast->arg());
  if (Expr::isError(arg)) {
    return arg;
  }
  return new (_arena) UnaryOp(kind, ast->location(), arg);
}

Expr* ResolveExprs::visitAdd(const ast::Oper* ast) {
  return arithmeticOp(ast, "+");
}

Expr* ResolveExprs::visitSub(const ast::Oper* ast) {
  return arithmeticOp(ast, "-");
}

Expr* ResolveExprs::visitMul(const ast::Oper* ast) {
  return arithmeticOp(ast, "*");
}

Expr* ResolveExprs::visitDiv(const ast::Oper* ast) {
  return arithmeticOp(ast, "/");
}

Expr* ResolveExprs::visitMod(const ast::Oper* ast) {
  return arithmeticOp(ast, "%");
}

Expr* ResolveExprs::visitBitAnd(const ast::Oper* ast) {
  return arithmeticOp(ast, "&");
}

Expr* ResolveExprs::visitBitOr(const ast::Oper* ast) {
  return arithmeticOp(ast, "|");
}

Expr* ResolveExprs::visitBitXor(const ast::Oper* ast) {
  return arithmeticOp(ast, "^");
}

Expr* ResolveExprs::visitLShift(const ast::Oper* ast) {
  return arithmeticOp(ast, "<<");
}

Expr* ResolveExprs::visitRShift(const ast::Oper* ast) {
  return arithmeticOp(ast, ">>");
}

Expr* ResolveExprs::visitEqual(const ast::Oper* ast) {
  return relationalOp(ast, "==");
}

Expr* ResolveExprs::visitRefEqual(const ast::Oper* ast) {
  assert(ast->operands().size() == 2);
  _reporter.error(ast->location()) << "Here.";
  assert(false && "Implement visitRefEqual.");
}

Expr* ResolveExprs::visitNotEqual(const ast::Oper* ast) {
  return relationalOp(ast, "!=");
}

Expr* ResolveExprs::visitLessThan(const ast::Oper* ast) {
  return relationalOp(ast, "<");
}

Expr* ResolveExprs::visitGreaterThan(const ast::Oper* ast) {
  return relationalOp(ast, ">");
}

Expr* ResolveExprs::visitLessThanOrEqual(const ast::Oper* ast) {
  return relationalOp(ast, "<=");
}

Expr* ResolveExprs::visitGreaterThanOrEqual(const ast::Oper* ast) {
  return relationalOp(ast, ">=");
}

Expr* ResolveExprs::visitIs(const ast::Oper* node) {
  assert(node->operands().size() == 2);
  auto arg = exec(node->operands()[0]);
  auto ty = exec(node->operands()[1]);
  if (Expr::isError(arg) || Expr::isError(ty)) {
    return &Expr::ERROR;
  }
  ResolveTypes rt(_reporter, _typeStore, _arena);
  return new (_arena) TypeOp(Expr::Kind::TYPE_TEST, node->location(), arg, rt.exec(ty));
}

Expr* ResolveExprs::visitIsNot(const ast::Oper* node) {
  return new (_arena) UnaryOp(Expr::Kind::LOGICAL_NOT, node->location(), visitIs(node));
}

Expr* ResolveExprs::visitAsType(const ast::Oper* node) {
  assert(node->operands().size() == 2);
  auto arg = exec(node->operands()[0]);
  auto ty = exec(node->operands()[1]);
  if (Expr::isError(arg) || Expr::isError(ty)) {
    return &Expr::ERROR;
  }
  ResolveTypes rt(_reporter, _typeStore, _arena);
  auto cast = new (_arena) TypeOp(Expr::Kind::TYPE_CAST, node->location(), arg, nullptr);
  cast->setType(rt.exec(ty));
  return cast;
}

Expr* ResolveExprs::visitAssign(const ast::Oper* ast) {
  assert(ast->operands().size() == 2);
  if (ast->operands()[0]->kind() == spark::ast::Kind::TUPLE) {
    auto leftAst = static_cast<const spark::ast::Oper*>(ast->operands()[0]);
    ExprArrayBuilder vars(_arena);
    for (auto lv : leftAst->operands()) {
      auto lval = exec(lv);
      if (Expr::isError(lval)) {
        return lval;
      }
      vars.append(lval);
    }
    auto right = exec(ast->operands()[1]);
    return new (_arena) Unpack(ast->location(), right, vars.build());
  }
  return binaryOp(ast, Expr::Kind::ASSIGN);
}

Expr* ResolveExprs::visitAssignAdd(const ast::Oper* node) {
  return augmentedAssign(node, "+");
}

Expr* ResolveExprs::visitAssignSub(const ast::Oper* node) {
  return augmentedAssign(node, "-");
}

Expr* ResolveExprs::visitAssignMul(const ast::Oper* node) {
  return augmentedAssign(node, "*");
}

Expr* ResolveExprs::visitAssignDiv(const ast::Oper* node) {
  return augmentedAssign(node, "/");
}

Expr* ResolveExprs::visitAssignMod(const ast::Oper* node) {
  return augmentedAssign(node, "%");
}

Expr* ResolveExprs::visitAssignBitAnd(const ast::Oper* node) {
  return augmentedAssign(node, "&");
}

Expr* ResolveExprs::visitAssignBitOr(const ast::Oper* node) {
  return augmentedAssign(node, "|");
}

Expr* ResolveExprs::visitAssignBitXor(const ast::Oper* node) {
  return augmentedAssign(node, "^");
}

Expr* ResolveExprs::visitAssignLShift(const ast::Oper* node) {
  return augmentedAssign(node, "<<");
}

Expr* ResolveExprs::visitAssignRShift(const ast::Oper* node) {
  return augmentedAssign(node, ">>");
}

Expr* ResolveExprs::visitPreInc(const ast::UnaryOp* node) {
  return preModify(node, "+");
}

Expr* ResolveExprs::visitPostInc(const ast::UnaryOp* node) {
  return postModify(node, "+");
}

Expr* ResolveExprs::visitPreDec(const ast::UnaryOp* node) {
  return preModify(node, "-");
}

Expr* ResolveExprs::visitPostDec(const ast::UnaryOp* node) {
  return postModify(node, "-");
}

Expr* ResolveExprs::visitRange(const ast::Oper* node) {
  auto first = exec(node->operands()[0]);
  auto last = exec(node->operands()[1]);
  if (Expr::isError(first) || Expr::isError(last)) {
    return &Expr::ERROR;
  }
  return new (_arena) BinaryOp(Expr::Kind::RANGE, node->location(), first, last);
}

Expr* ResolveExprs::visitSpecialize(const ast::Oper* ast) {
  Expr* base = exec(ast->op());
  if (Expr::isError(base)) {
    return base;
  }

  ExprArrayBuilder builder(_arena);
  for (auto arg : ast->operands()) {
    Expr* argExpr = exec(arg);
    if (Expr::isError(argExpr)) {
      return argExpr;
    }
    builder.append(argExpr);
  }

  auto call = new (_arena) Call(Expr::Kind::SPECIALIZE, ast->location());
  call->setCallable(base);
  call->setArguments(builder.build());
  return call;
}

//     if (result is None or len(result.members) == 0) and emptyResultOk:
//       mset = graph.MemberList().setLocation(node.location).setName(name)
//       mset.setListType(graph.MemberListType.INCOMPLETE)
//       return mset
//
//     return self.createMemberListExpr(node.location, result, name, emptyResultOk=emptyResultOk)

Expr* ResolveExprs::visitCall(const ast::Oper* ast) {
  Expr* base = exec(ast->op());
  if (Expr::isError(base)) {
    return base;
  }

  ExprArrayBuilder builder(_arena);
  for (auto arg : ast->operands()) {
    Expr* argExpr = exec(arg);
    if (Expr::isError(argExpr)) {
      return argExpr;
    }
    builder.append(argExpr);
  }
  auto call = new (_arena) Call(Expr::Kind::CALL, ast->location());
  call->setCallable(base);
  call->setArguments(builder.build());
  return call;
}

Expr* ResolveExprs::visitFunctionType(const ast::Oper* ast) {
  Expr* base = ast->op() ? exec(ast->op()) : &Expr::IGNORED;
  if (Expr::isError(base)) {
    return base;
  }

  ExprArrayBuilder builder(_arena);
  for (auto arg : ast->operands()) {
    Expr* argExpr = exec(arg);
    if (Expr::isError(argExpr)) {
      return argExpr;
    }
    builder.append(argExpr);
  }
  auto call = new (_arena) Call(Expr::Kind::FUNCTION_TYPE, ast->location());
  call->setCallable(base);
  call->setArguments(builder.build());
  return call;
}

Expr* ResolveExprs::visitLogicalAnd(const ast::Oper* ast) {
  return binaryOp(ast, Expr::Kind::LOGICAL_AND);
}

Expr* ResolveExprs::visitLogicalOr(const ast::Oper* ast) {
  return binaryOp(ast, Expr::Kind::LOGICAL_OR);
}

Expr* ResolveExprs::visitReturn(const ast::UnaryOp* node) {
  Expr* arg = nullptr;
  if (node->arg() != nullptr) {
    arg = exec(node->arg());
    if (Expr::isError(arg)) {
      return arg;
    }
  }
  return new (_arena) UnaryOp(Expr::Kind::RETURN, node->location(), arg);
}

Expr* ResolveExprs::visitThrow(const ast::UnaryOp* node) {
  auto arg = exec(node->arg());
  if (Expr::isError(arg)) {
    return arg;
  }
  return new (_arena) UnaryOp(Expr::Kind::THROW, node->location(), arg);
}

Expr* ResolveExprs::visitTuple(const ast::Oper* ast) {
  ExprArrayBuilder builder(_arena);
  for (auto node : ast->operands()) {
    Expr* arg = exec(node);
    if (Expr::isError(arg)) {
      return arg;
    }
    builder.append(arg);
  }
  return new (_arena) MultiArgOp(Expr::Kind::PACK, ast->location(), builder.build());
}

Expr* ResolveExprs::visitUnion(const ast::Oper* ast) {
  ExprArrayBuilder builder(_arena);
  for (auto node : ast->operands()) {
    Expr* arg = exec(node);
    if (Expr::isError(arg)) {
      return arg;
    }
    builder.append(arg);
  }
  return new (_arena) MultiArgOp(Expr::Kind::UNION_TYPE, ast->location(), builder.build());
}

Expr* ResolveExprs::visitConst(const ast::UnaryOp* ast) {
  Expr* arg = exec(ast->arg());
  if (Expr::isError(arg)) {
    return arg;
  }

  if (ast->kind() == ast::Kind::CONST) {
    return new (_arena) UnaryOp(Expr::Kind::CONST_TYPE, ast->location(), arg);
  } else if (ast->kind() == ast::Kind::PROVISIONAL_CONST) {
    return new (_arena) UnaryOp(Expr::Kind::PROVISIONAL_CONST_TYPE, ast->location(), arg);
  }
  _reporter.error(ast->location()) << "Here.";
  assert(false);
}

Expr* ResolveExprs::visitBlock(const ast::Oper* oper) {
  scope::SymbolScope* blockScope = nullptr;
  ExprArrayBuilder stmts(_arena);
  std::vector<Defn*> localDefs;
  for (auto node : oper->operands()) {
    if (node->kind() == ast::Kind::VAR || node->kind() == ast::Kind::LET) {
      auto v = static_cast<const ast::ValueDefn*>(node);
      if (blockScope == nullptr) {
        blockScope = new scope::StandardScope(scope::SymbolScope::LOCAL, "local scope");
        _scopeStack->push(blockScope);
        _localScopes.push_back(blockScope);
      }

      // See if this name is already defined
      Defn* alreadyDefined = nullptr;
      std::vector<Member*> lookupResult;
      bool sameScope = true;
      for (auto it = _localScopes.end(); it != _localScopes.begin(); ) {
        --it;
        (*it)->lookupName(v->name(), lookupResult);
        if (!lookupResult.empty()) {
          alreadyDefined = static_cast<Defn*>(lookupResult.front());
          break;
        }
        sameScope = false;
      }
      if (alreadyDefined) {
        if (sameScope) {
          _reporter.error(node->location()) << "Variable '" << v->name() <<
              "' already defined in this scope.";
        } else {
          _reporter.error(node->location()) << "Variable '" << v->name() <<
              "' shadows a variable with the same name in an enclosing scope.";
        }
        _reporter.error(alreadyDefined->location()) << "Defined here.";
      }

      auto vdef = new ValueDefn(
          node->kind() == ast::Kind::VAR ? Defn::Kind::VAR : Defn::Kind::LET,
          v->location(),
          v->name());
      vdef->setAst(node);
      vdef->setDefined(false);
      blockScope->addMember(vdef);
      localDefs.push_back(vdef);
    }
  }

  auto it = localDefs.begin();
  for (auto node : oper->operands()) {
    if (node->kind() == ast::Kind::VAR || node->kind() == ast::Kind::LET) {
      auto v = static_cast<const ast::ValueDefn*>(node);
      auto vdef = static_cast<ValueDefn*>(*it++);
      assert(v->name() == vdef->name());
      if (v->init() != nullptr) {
        vdef->setInit(exec(v->init()));
      }
      if (v->type() != nullptr) {
        auto varType = exec(v->type());
        if (!Expr::isError(varType)) {
          ResolveTypes rt(_reporter, _typeStore, _arena);
          vdef->setType(rt.exec(varType));
        }
      }
      Expr* localDef = new (_arena) LocalDefn(node->location(), vdef);
      stmts.append(localDef);
      vdef->setDefined(true);
    } else {
      Expr* stmt = exec(node);
      if (Expr::isError(stmt)) {
        return stmt;
      }
      stmts.append(stmt);
    }
#if 0
    # Inline progs into blocks
    if isinstance(st, graph.Prog):
      block.getMutableStatements().extend(st.getArgs())
    else:
      block.getMutableStatements().append(st)
#endif
  }

  if (blockScope) {
    _scopeStack->pop();
    _localScopes.pop_back();
  }

  auto block = new (_arena) Block(oper->location(), stmts.build());
  return block;
}

Expr* ResolveExprs::visitLocalVar(const ast::ValueDefn* ast) {
  assert(false && "Local variables can only be children of blocks.");
  return nullptr;
}

Expr* ResolveExprs::visitIf(const ast::ControlStmt* ast) {
  auto test = exec(ast->test());
  if (Expr::isError(test)) {
    return test;
  }
  assert(ast->outcomes().size() >= 1);
  assert(ast->outcomes().size() <= 2);
  auto thenBlock = exec(ast->outcomes()[0]);
  if (Expr::isError(thenBlock)) {
    return thenBlock;
  }
  Expr* elseBlock = nullptr;
  if (ast->outcomes().size() > 1) {
    elseBlock = exec(ast->outcomes()[1]);
    if (Expr::isError(elseBlock)) {
      return elseBlock;
    }
  }
  return new (_arena) IfStmt(ast->location(), test, thenBlock, elseBlock);
}

Expr* ResolveExprs::visitWhile(const ast::ControlStmt* ast) {
  auto test = exec(ast->test());
  if (Expr::isError(test)) {
    return test;
  }
  assert(ast->outcomes().size() == 1);
  auto body = exec(ast->outcomes()[0]);
  if (Expr::isError(body)) {
    return body;
  }
  return new (_arena) WhileStmt(ast->location(), test, body);
}

Expr* ResolveExprs::visitLoop(const ast::ControlStmt* ast) {
  assert(ast->outcomes().size() == 1);
  auto body = exec(ast->outcomes()[0]);
  if (Expr::isError(body)) {
    return body;
  }
  return new (_arena) UnaryOp(Expr::Kind::LOOP, ast->location(), body);
}

Expr* ResolveExprs::visitFor(const ast::ControlStmt* ast) {
  auto stmt = new (_arena) ForStmt(ast->location());
  assert(ast->outcomes().size() == 4);
  auto varsAst = static_cast<const ast::ValueDefn*>(ast->outcomes()[0]);
  auto initAst = varsAst->init();
  auto testAst = ast->outcomes()[1];
  auto stepAst = ast->outcomes()[2];
  auto bodyAst = ast->outcomes()[3];

  stmt->setInit(initAst ? exec(initAst) : nullptr);

  support::ArrayBuilder<ValueDefn*> vars(_arena);
  stmtVars(varsAst, vars);
  stmt->setVars(vars.build());

  scope::SymbolScope* forScope = new scope::StandardScope(scope::SymbolScope::LOCAL, "for scope");
  _scopeStack->push(forScope);
  _localScopes.push_back(forScope);
  for (auto vd : stmt->vars()) {
    forScope->addMember(vd);
  }

  stmt->setTest(testAst ? exec(testAst) : nullptr);
  stmt->setStep(stepAst ? exec(stepAst) : nullptr);
  stmt->setBody(bodyAst ? exec(bodyAst) : nullptr);

  _scopeStack->pop();
  _localScopes.pop_back();
  return stmt;
}

Expr* ResolveExprs::visitForIn(const ast::ControlStmt* ast) {
  auto stmt = new (_arena) ForInStmt(ast->location());

  assert(ast->outcomes().size() == 3);
  auto varsAst = static_cast<const ast::ValueDefn*>(ast->outcomes()[0]);
  auto iterAst = ast->outcomes()[1];
  auto bodyAst = ast->outcomes()[2];

  stmt->setIter(iterAst ? exec(iterAst) : nullptr);

  support::ArrayBuilder<ValueDefn*> vars(_arena);
  stmtVars(varsAst, vars);
  stmt->setVars(vars.build());

  scope::SymbolScope* forScope = new scope::StandardScope(scope::SymbolScope::LOCAL, "for scope");
  _scopeStack->push(forScope);
  _localScopes.push_back(forScope);
  for (auto vd : stmt->vars()) {
    forScope->addMember(vd);
  }

  stmt->setBody(bodyAst ? exec(bodyAst) : nullptr);

  _scopeStack->pop();
  _localScopes.pop_back();
  return stmt;
}

Expr* ResolveExprs::visitSwitch(const ast::ControlStmt* node) {
  auto stmt = new (_arena) SwitchStmt(node->location());
  auto testExpr = exec(node->test());
  if (Expr::isError(testExpr)) {
    return testExpr;
  }
  if (!isSimpleExpr(testExpr)) {
    testExpr = storeTemp(testExpr);
  }
  stmt->setTestExpr(testExpr);

  auto visitCaseConstant = [this](const ast::Node* node) -> Expr* {
    if (node->kind() == ast::Kind::IDENT) {
      return visitIdent(static_cast<const ast::Ident*>(node), true);
    } else {
      return exec(node);
    }
  };

  for (auto caseNode : node->outcomes()) {
    if (caseNode->kind() == ast::Kind::CASE) {
      auto caseOp = static_cast<const ast::Oper*>(caseNode);
      auto body = exec(caseOp->op());
      ExprArrayBuilder values(_arena);
      ExprArrayBuilder rangeValues(_arena);
      for (auto val : caseOp->operands()) {
        if (val->kind() == ast::Kind::RANGE) {
          auto first = visitCaseConstant(static_cast<const ast::Oper*>(val)->operands()[0]);
          auto last = visitCaseConstant(static_cast<const ast::Oper*>(val)->operands()[1]);
          if (Expr::isError(first) || Expr::isError(last)) {
            continue;
          }
          rangeValues.append(
              new (_arena) BinaryOp(Expr::Kind::RANGE, node->location(), first, last));
        } else {
          auto valExpr = visitCaseConstant(val);
          values.append(valExpr);
        }
      }
      auto caseStmt = new (_arena) CaseStmt(caseOp->location());
      caseStmt->setValues(values.build());
      caseStmt->setRangeValues(rangeValues.build());
      caseStmt->setBody(body);
    } else if (caseNode->kind() == ast::Kind::ELSE) {
      auto caseOp = static_cast<const ast::Oper*>(caseNode);
      auto body = exec(caseOp->op());
      if (stmt->elseBody() != nullptr) {
        _reporter.error(caseNode->location()) <<
            "Switch statement can have at most one 'else' block.";
      } else {
        stmt->setElseBody(body);
      }
    } else {
      assert(false && "Invalid case node.");
    }
  }
  return stmt;
}

Expr* ResolveExprs::visitMatch(const ast::ControlStmt* node) {
  ResolveTypes rt(_reporter, _typeStore, _arena);
  auto stmt = new (_arena) MatchStmt(node->location());
  auto testExpr = exec(node->test());
  if (Expr::isError(testExpr)) {
    return testExpr;
  }
  if (!isSimpleExpr(testExpr)) {
    testExpr = storeTemp(testExpr);
  }
  stmt->setTestExpr(testExpr);

  StringRef defaultName;
  if (node->test()->kind() == ast::Kind::IDENT) {
    defaultName = static_cast<const ast::Ident*>(node->test())->name();
  }

  support::ArrayBuilder<Pattern*> builder(_arena);
  for (auto pn : node->outcomes()) {
    if (pn->kind() == ast::Kind::PATTERN) {
      auto pat = static_cast<const ast::Oper*>(pn);
      auto nameAst = pat->operands()[0];
      auto typeAst = pat->operands()[1];
      auto bodyAst = pat->operands()[2];
      auto name = defaultName;
      if (ast::Node::isPresent(nameAst)) {
        assert(nameAst->kind() == ast::Kind::IDENT);
        name = static_cast<const ast::Ident*>(nameAst)->name();
      }
      auto var = new ValueDefn(Member::Kind::LET, pn->location(), name);
      Type* type = nullptr;
      if (ast::Node::isPresent(typeAst)) {
        type = rt.exec(exec(typeAst));
        if (Type::isError(type)) {
          continue;
        }
        var->setType(type);
      }
      var->setInit(testExpr);
      auto patternStmt = new (_arena) Pattern(pat->location());
      patternStmt->setVar(var);
      auto scope = new scope::StandardScope(scope::SymbolScope::LOCAL, "for scope");
      if (!var->name().empty()) {
        scope->addMember(var);
      }
      _scopeStack->push(scope);
      _localScopes.push_back(scope);
      patternStmt->setBody(exec(bodyAst));
      _scopeStack->pop();
      _localScopes.pop_back();
      delete scope;

      builder.append(patternStmt);
    } else if (pn->kind() == ast::Kind::ELSE) {
      auto elseAst = static_cast<const ast::Oper*>(pn);
      auto body = exec(elseAst->operands()[0]);
      if (stmt->elseBody() != nullptr) {
        _reporter.error(elseAst->location()) <<
            "Match statement can have at most one 'else' block.";
      } else {
        stmt->setElseBody(body);
      }
    }
  }
  stmt->setPatterns(builder.build());
  return stmt;
}

#if 0
  @accept(ast.Match)
  def visitMatch(self, node):
    '@type node: spark.graph.ast.Match'
//     stmt = graph.Match().setLocation(node.location)
//     testExpr = self.visit(node.testExpr)
//     testExprInit, testExprCached = self.cacheExpr(testExpr)
//     stmt.setTestExpr(testExprInit)

//     if isinstance(node.testExpr, ast.Ident):
//       defaultName = node.testExpr.name
//     else:
//       defaultName = None
    # TODO: See if any of the types are the same
    # TODO: See if any of the types are unreachable.
    for patternNode in node.patterns:
      patternStmt = graph.Match.Pattern().setLocation(patternNode.location)
      if patternNode.hasType:
        nodeType = self.resolveTypes.visit(patternNode.type)
        var = parser.Let().setLocation(patternNode.location)
        var.setType(nodeType)
        if patternNode.name:
          var.setName(patternNode.name)
        elif defaultName:
          var.setName(defaultName)
        var.setInit(testExprCached)
        patternStmt.setVar(var)

        body = graph.Block().setLocation(patternNode.body.location)
        localScope = StandardScope(description='match var', scopeType=ScopeType.LOCAL)
        localScope.addMember(var)
        body.setLocalScope(localScope)
        self.nameLookup.scopeStack.push(localScope)
        b = self.visit(patternNode.body)
        if self.isErrorExpr(b):
          return b
        body.getMutableStatements().append(b)
        self.nameLookup.scopeStack.pop()
      else:
        body = self.visit(patternNode.body)

      if self.isErrorExpr(body):
        return body

      patternStmt.setBody(body)
      if patternNode.hasType:
        stmt.getMutablePatterns().append(patternStmt)
      elif stmt.hasElseBody():
        self.errorAt(patternStmt, "Match statement have at most one 'else' clause.")
      else:
        stmt.setElseBody(body)
    return stmt

  @accept(ast.LocalDefn)
  def visitLocalDefn(self, node):
    self.resolveMembers.visit(node.defn)
    vdef = node.defn
    stmt = graph.LocalDefn().setLocation(node.location).setDefn(node.defn)
    # TODO: Check if shadows local
    self.nameLookup.scopeStack.current().addMember(stmt.getDefn())

    if isinstance(node.defn, graph.TypeDefn):
      if len(node.defn.getTypeParams()) > 0:
        self.errorAt(node.defn.getLocation(),
            'Types defined in local scope may not have type parameters.')
      elif len(node.defn.getAstRequirements()) > 0:
        self.errorAt(node.defn.getLocation(),
            'Types defined in local scope may not have type requirements.')
    elif isinstance(node.defn, graph.Function):
      if len(node.defn.getTypeParams()) > 0:
        self.errorAt(node.defn.getLocation(),
            'Functions defined in local scope may not have type parameters.')
      elif len(node.defn.getAstRequirements()) > 0:
        self.errorAt(node.defn.getLocation(),
            'Functions defined in local scope may not have type requirements.')
    elif isinstance(node.defn, graph.ValueDefn):
      if not vdef.astInit and not vdef.astType:
        self.errorAt(vdef, "Variable must have either an explicit type or an initializer.")
      if len(node.defn.getMembers()) > 0:
        result = graph.Prog().setLocation(node.location)
        if vdef.hasInit():
          initExpr, initCached = self.cacheExpr(vdef.getInit())
          result.getMutableArgs().append(initExpr)
          tupleMember = graph.TupleMemberRef().setLocation(initExpr.getLocation())
          tupleMember.setArg(initCached)
          tupleMember.setIndex(0)
          vdef.setInit(tupleMember)
        result.getMutableArgs().append(stmt)
        for index, member in enumerate(node.defn.getMembers()): # @type member: spark.ast.Defn
          self.nameLookup.scopeStack.current().addMember(member)
          tupleMember = graph.TupleMemberRef().setLocation(initExpr.getLocation())
          tupleMember.setArg(initCached)
          tupleMember.setIndex(index + 1)
          member.setInit(tupleMember)
          stmt = graph.LocalDefn().setLocation(node.location).setDefn(member)
          result.getMutableArgs().append(stmt)
        return result
    return stmt
#endif

Expr* ResolveExprs::visitBreak(const ast::Node* node) {
  return new (_arena) Expr(Expr::Kind::BREAK, node->location());
}

Expr* ResolveExprs::visitContinue(const ast::Node* node) {
  return new (_arena) Expr(Expr::Kind::CONTINUE, node->location());
}

Expr* ResolveExprs::relationalOp(const ast::Oper* ast, const StringRef& name) {
  assert(ast->operands().size() == 2);

  Expr* left = exec(ast->operands()[0]);
  Expr* right = exec(ast->operands()[1]);
  if (Expr::isError(left) || Expr::isError(right)) {
    return &Expr::ERROR;
  }
  Expr* result = evalRelational(ast->location(), ast->kind(), left, right);
  if (result != nullptr) {
    return result;
  }
  Call* call = new (_arena) Call(Expr::Kind::CALL, ast->location());
  Expr* callable = new (_arena) MemberSet(name, ast->location());
  call->setCallable(callable);
  ExprArrayBuilder builder(_arena);
  builder.append(left);
  builder.append(right);
  call->setArguments(builder.build());
  return call;
}

Expr* ResolveExprs::evalRelational(const Location& loc, ast::Kind kind, Expr* left, Expr* right) {
  if (left->kind() == Expr::Kind::INTEGER_LITERAL) {
    IntegerLiteral* leftInt = static_cast<IntegerLiteral*>(left);
    if (right->kind() == Expr::Kind::INTEGER_LITERAL) {
      IntegerLiteral* rightInt = static_cast<IntegerLiteral*>(right);
      if (leftInt->isUnsigned() != rightInt->isUnsigned()) {
        _reporter.error(loc) << "Signed / unsigned mismatch.";
      }
      assert(!leftInt->isUnsigned() && "Implement unsigned.");
      int64_t leftValue = leftInt->value();
      int64_t rightValue = rightInt->value();
      bool result;
      switch (kind) {
        case ast::Kind::EQUAL:
          result = leftValue == rightValue;
          break;
        case ast::Kind::NOT_EQUAL:
          result = leftValue != rightValue;
          break;
        case ast::Kind::LESS_THAN:
          result = leftValue < rightValue;
          break;
        case ast::Kind::GREATER_THAN:
          result = leftValue > rightValue;
          break;
        case ast::Kind::LESS_THAN_OR_EQUAL:
          result = leftValue <= rightValue;
          break;
        case ast::Kind::GREATER_THAN_OR_EQUAL:
          result = leftValue >= rightValue;
          break;
        default:
          assert(false && "Invalid relational operator.");
      }

      return new (_arena) BooleanLiteral(loc, result);
    }
  }
  return nullptr;
}

Expr* ResolveExprs::arithmeticOp(const ast::Oper* ast, const StringRef& name) {
  assert(ast->operands().size() == 2);

  Expr* left = exec(ast->operands()[0]);
  Expr* right = exec(ast->operands()[1]);
  if (Expr::isError(left) || Expr::isError(right)) {
    return &Expr::ERROR;
  }
  Expr* result = evalArithmetic(ast->location(), ast->kind(), left, right);
  if (result != nullptr) {
    return result;
  }
  Call* call = new (_arena) Call(Expr::Kind::CALL, ast->location());
  Expr* callable = new (_arena) MemberSet(name, ast->location());
  call->setCallable(callable);
  ExprArrayBuilder builder(_arena);
  builder.append(left);
  builder.append(right);
  call->setArguments(builder.build());
  return call;
}

Expr* ResolveExprs::evalArithmetic(const Location& loc, ast::Kind kind, Expr* left, Expr* right) {
  if (left->kind() == Expr::Kind::INTEGER_LITERAL) {
    IntegerLiteral* leftInt = static_cast<IntegerLiteral*>(left);
    if (right->kind() == Expr::Kind::INTEGER_LITERAL) {
      IntegerLiteral* rightInt = static_cast<IntegerLiteral*>(right);
      if (leftInt->isUnsigned() != rightInt->isUnsigned()) {
        _reporter.error(loc) << "Signed / unsigned mismatch.";
      }
      assert(!leftInt->isUnsigned() && "Implement unsigned.");
      int64_t leftValue = leftInt->value();
      int64_t rightValue = rightInt->value();
      int64_t result; // TODO: Replace with APA.
      switch (kind) {
        case ast::Kind::ADD:
          result = leftValue + rightValue;
          break;
        case ast::Kind::SUB:
          result = leftValue - rightValue;
          break;
        case ast::Kind::MUL:
          result = leftValue * rightValue;
          break;
        case ast::Kind::DIV:
          result = leftValue / rightValue;
          break;
        case ast::Kind::MOD:
          result = leftValue % rightValue;
          break;
        case ast::Kind::BIT_AND:
          result = leftValue & rightValue;
          break;
        case ast::Kind::BIT_OR:
          result = leftValue | rightValue;
          break;
        case ast::Kind::BIT_XOR:
          result = leftValue ^ rightValue;
          break;
        case ast::Kind::LSHIFT:
          result = leftValue << rightValue;
          break;
        case ast::Kind::RSHIFT:
          result = leftValue >> rightValue;
          break;
        default:
          assert(false && "Invalid arithmetic operator.");
      }

      return new (_arena) IntegerLiteral(loc, result, leftInt->isUnsigned());
    }
  }
  return nullptr;
}

Expr* ResolveExprs::binaryOp(const ast::Oper* ast, Expr::Kind kind) {
  assert(ast->operands().size() == 2);

  Expr* left = exec(ast->operands()[0]);
  Expr* right = exec(ast->operands()[1]);
  if (Expr::isError(left) || Expr::isError(right)) {
    return &Expr::ERROR;
  }
  BinaryOp* expr = new (_arena) BinaryOp(kind, ast->location());
  expr->setLeft(left);
  expr->setRight(right);
  return expr;
}

Expr* ResolveExprs::augmentedAssign(const ast::Oper* ast, const StringRef& name) {
  assert(ast->operands().size() == 2);

  Expr* left = exec(ast->operands()[0]);
  Expr* right = exec(ast->operands()[1]);
  if (Expr::isError(left) || Expr::isError(right)) {
    return &Expr::ERROR;
  }

  Expr* lval;
  Expr* rval;
  std::tie(lval, rval) = getReadModifyWriteLocation(left);

  auto call = new (_arena) Call(Expr::Kind::CALL, ast->location());
  Expr* callable = new (_arena) MemberSet(name, ast->location());
  call->setCallable(callable);
  ExprArrayBuilder builder(_arena);
  builder.append(left);
  builder.append(right);
  call->setArguments(builder.build());

  return new (_arena) BinaryOp(Expr::Kind::ASSIGN, ast->location(), lval, call);
}

Expr* ResolveExprs::preModify(const ast::UnaryOp* ast, const StringRef& opName) {
  auto arg = exec(ast->arg());
  if (Expr::isError(arg)) {
    return arg;
  }
  Expr* lval;
  Expr* rval;
  std::tie(lval, rval) = getReadModifyWriteLocation(arg);

  // lval = call('+', rval, 1)
  auto call = new (_arena) Call(Expr::Kind::CALL, ast->location());
  Expr* callable = new (_arena) MemberSet(opName, ast->location());
  call->setCallable(callable);
  ExprArrayBuilder builder(_arena);
  builder.append(rval);
  builder.append(new (_arena) IntegerLiteral(ast->location(), 1, false));
  call->setArguments(builder.build());

  return new (_arena) BinaryOp(Expr::Kind::ASSIGN, ast->location(), lval, call);
}

Expr* ResolveExprs::postModify(const ast::UnaryOp* ast, const StringRef& opName) {
  auto arg = exec(ast->arg());
  if (Expr::isError(arg)) {
    return arg;
  }
  Expr* lval;
  Expr* rval;
  std::tie(lval, rval) = getReadModifyWriteLocation(arg);
  rval = storeTemp(rval); // We are going to use rval twice.

  // lval = call('+', rval, 1)
  auto call = new (_arena) Call(Expr::Kind::CALL, ast->location());
  Expr* callable = new (_arena) MemberSet(opName, ast->location());
  call->setCallable(callable);
  ExprArrayBuilder builder(_arena);
  builder.append(rval);
  builder.append(new (_arena) IntegerLiteral(ast->location(), 1, false));
  call->setArguments(builder.build());

  auto assign = new (_arena) BinaryOp(Expr::Kind::ASSIGN, ast->location(), lval, call);

  ExprArrayBuilder builder2(_arena);
  builder2.append(assign);
  builder2.append(rval);
  return new (_arena) MultiArgOp(Expr::Kind::PROG, ast->location(), builder2.build());
}

#if 0
  def unaryOpFn(self, location, name, arg):
    assert name
    assert arg
    argExpr = self.visit(arg)
    assert isinstance(argExpr, graph.Expr)
    if isinstance(argExpr, graph.IntegerLiteral):
      if name == '-':
        result = graph.IntegerLiteral().merge(argExpr)
        graphtools.encodeInt(result, -graphtools.decodeInt(argExpr))
        return result
    elif isinstance(argExpr, graph.FloatLiteral):
      if name == '-':
        result = graph.FloatLiteral().merge(argExpr).setValue(-argExpr.getValue())
        return result
    call = graph.Call().setLocation(location)
    oper = graph.MemberList().setLocation(location).setName(name)
    call.getMutableArgs().append(oper)
    call.getMutableArgs().append(argExpr)
    return call

  def lowerBinaryOpExpr(self, location, name, left, right):
    assert name
    assert left
    assert right
    call = graph.Call().setLocation(location)
    oper = graph.MemberList().setLocation(location).setName(name)
    call.getMutableArgs().append(oper)
    call.getMutableArgs().append(left)
    call.getMutableArgs().append(right)
    return call
#endif

/** Given an expression which points to a storage location, return two copies of the expression,
    one which will be read from and one which will be written to. This is used to implement
    read-modify-write operations such as 'i++' and 'a |= b'.

    The resulting graph will be structured so as to avoid evaluating parts of the expression
    multiple times. For example, if passed an expression such as 'a.b', a temp variable will
    be created to hold the value 'a'. */
std::tuple<Expr*, Expr*> ResolveExprs::getReadModifyWriteLocation(Expr* expr) {
  if (expr->kind() == Expr::Kind::MEMBER_SET) {
    auto mset = static_cast<MemberSet*>(expr);
    if (mset->stem() != nullptr && mset->stem()->kind() == Expr::Kind::SELF) {
      // TODO: Check for other 'simple' expressions
      // However we want to avoid the case where the base is a property
      return std::make_tuple(expr, expr);
    }
    auto lval = new (_arena) MemberSet(mset->name(), expr->location());
    auto rval = new (_arena) MemberSet(mset->name(), expr->location());
    if (mset->stem()) {
      // Make sure we don't evaluate the stem expression twice.
      auto stem = storeTemp(mset->stem());
      lval->setStem(stem);
      rval->setStem(stem);
    }
    return std::make_tuple(lval, rval);
  }
  _reporter.error(expr->location()) << "Expression is not modifiable.";
  return std::make_tuple(&Expr::ERROR, &Expr::ERROR);
}

// /** Return two copies of a temp variable, one which is initialized to the expression, and
//     one which returns the cached value of the expression. */
// std::tuple<Expr*, Expr*> ResolveExprs::cacheExpr(Expr* expr) {
//   if (isSimpleExpr(expr)) {
//     return std::make_tuple(expr, expr);
//   } else {
//     auto tmp = storeTemp(expr);
//     return std::make_tuple(tmp, tmp);
//   }
// }

// What we mean by a 'simple' expression is one that is so cheap to evaluate that it is not
// worth caching in a temporary variable.
bool ResolveExprs::isSimpleExpr(Expr* expr) {
  switch (expr->kind()) {
    case Expr::Kind::INTEGER_LITERAL:
    case Expr::Kind::BOOL_LITERAL:
    case Expr::Kind::FLOAT_LITERAL:
    case Expr::Kind::STRING_LITERAL:
    case Expr::Kind::SELF:
      return true;
    case Expr::Kind::MEMBER_SET: {
      auto mset = static_cast<MemberSet*>(expr);
      if (mset->stem() != nullptr) {
        // In other words, a.b is worth caching, self.b is not. That's probably not a great
        // heuristic but it will do for the moment.
        // TODO: Rework this algorithm.
        if (mset->stem()->kind() == Expr::Kind::MEMBER_SET) {
          return false;
        }
        return isSimpleExpr(mset->stem());
      }
      return true;
    }
//     case Expr::Kind::DEFN_REF: {
//     }
    default:
      return false;
  }
//   if isinstance(expr, graph.DefnRef):
//     if not expr.hasBase():
//       return True
//     base = expr.getBase()
//     if isinstance(base, graph.DefnRef):
//       if base.hasBase():
//         return False
}

// Store 'value' in a temporary variable and return a reference to that variable.
semgraph::TempVarRef* ResolveExprs::storeTemp(Expr* value) {
  int32_t index = 0;
  for (Member* s = _subject.get(); s != nullptr; s = s->definedIn()) {
    if (auto f = dyn_cast<Function*>(s)) {
      index = f->nextTempVarIndex();
      break;
    } else if (auto m = dyn_cast<Module*>(s)) {
      index = -1 - m->nextTempVarIndex();
      break;
    }
  }
  return new (_arena) TempVarRef(value->location(), value, index);
}

/** Collect all of the named variables in a single variable definition statement. */
void ResolveExprs::stmtVars(const ast::ValueDefn* var, support::ArrayBuilder<ValueDefn*>& result) {
  assert(var->kind() == ast::Kind::LET
      || var->kind() == ast::Kind::VAR
      || var->kind() == ast::Kind::VAR_LIST
      || var->kind() == ast::Kind::ABSENT);
  if (var->kind() == ast::Kind::VAR_LIST) {
    for (auto m : var->members()) {
      stmtVars(static_cast<const ast::ValueDefn*>(m), result);
    }
  } else {
    auto vd = new ValueDefn(
      var->kind() == ast::Kind::LET ? Defn::Kind::LET : Defn::Kind::VAR,
      var->location(),
      var->name());
    result.append(vd);
  }
}

}}}
