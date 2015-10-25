#include "spark/ast/node.h"
#include "spark/ast/defn.h"
#include "spark/ast/ident.h"
#include "spark/ast/module.h"
#include "spark/ast/oper.h"
#include "spark/error/reporter.h"
#include "spark/scope/closematch.h"
#include "spark/scope/scopestack.h"
#include "spark/sema/names/fillmemberset.h"
#include "spark/sema/names/resolveexprs.h"
#include "spark/semgraph/expr.h"
#include "spark/semgraph/primitivetype.h"
#include "spark/support/arraybuilder.h"
#include "spark/support/arena.h"

namespace spark {
namespace sema {
namespace names {
using namespace semgraph;

Expr* ResolveExprs::exec(const ast::Node* node) {
  switch (node->kind()) {
    case ast::Kind::IDENT:
      return visitIdent(static_cast<const ast::Ident*>(node), false);
    case ast::Kind::MEMBER:
      return visitMemberRef(static_cast<const ast::MemberRef*>(node));
    case ast::Kind::SPECIALIZE:
      return visitSpecialize(static_cast<const ast::Oper*>(node));
    case ast::Kind::BUILTIN_TYPE:
      return visitBuiltinType(static_cast<const ast::BuiltInType*>(node));
    default:
      _reporter.fatal() << "Invalid AST node kind: " << node->kind();
      return NULL;
  }
}

Expr* ResolveExprs::visitIdent(const ast::Ident* ident, bool emptyResultOk) {
  scope::NameLookupResult lookupResult = _scopeStack->find(ident->name());
  auto mset = new (_arena) MemberSet(_arena.copyOf(ident->name()));
  mset->setStem(lookupResult.stem);
  mset->setLocation(ident->location());
  if (lookupResult.members.empty()) {
    if (emptyResultOk) {
      return mset;
    }
    scope::CloseMatchFinder matcher(ident->name());
    _scopeStack->forAllNames(matcher);
    if (matcher.closest().empty()) {
      _reporter.error(ident->location()) << "Name '" << ident->name() << "' not found.";
    } else {
      _reporter.error(ident->location()) << "Name '" << ident->name() <<
          "' not found, did you mean '" << matcher.closest() << "'?";
    }
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
  assert(false && "Implement member ref");
  return nullptr;
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

}}}
