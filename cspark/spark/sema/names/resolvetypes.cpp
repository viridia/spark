#include "spark/ast/node.h"
#include "spark/ast/defn.h"
#include "spark/ast/ident.h"
#include "spark/ast/module.h"
#include "spark/error/reporter.h"
#include "spark/scope/scopestack.h"
#include "spark/sema/names/resolvetypes.h"
#include "spark/semgraph/type.h"

namespace spark {
namespace sema {
namespace names {
using namespace semgraph;

semgraph::Type* ResolveTypes::exec(const ast::Node* node) {
  switch (node->kind()) {
    case ast::Kind::IDENT:
      return visitIdent(static_cast<const ast::Ident*>(node));
    default:
      _reporter.fatal() << "Invalid AST node kind: " << node->kind();
      return NULL;
  }
}

Type* ResolveTypes::visitIdent(const ast::Ident* ident) {
  scope::NameLookupResult lookupResult = _scopeStack->find(ident->name());
  if (lookupResult.members.empty()) {
    // Do fuzzy lookup.
    _reporter.error(ident->location()) << "Name lookup failed: " << ident->name();
  }
  (void)_typeStore;
  return &Type::ERROR;
}

//   @accept(ast.Ident)
//   def visitIdent(self, node, emptyResultOk=False, multiOk=False):
//     '@type node: spark.graph.ast.Ident'
//     name = node.name
//     assert name
//     if node.implicitSelf:
//       result = self.nameLookup.scopeStack.findPrivate(name)
//     else:
//       result = self.nameLookup.scopeStack.find(name)
//
//     if (result is None or len(result.members) == 0) and emptyResultOk:
//       mlist = graph.MemberList().setLocation(node.location).setName(name)
//       mlist.setListType(graph.MemberListType.INCOMPLETE)
//       return mlist
//
//     return self.createMemberListExpr(node.location, result, name, emptyResultOk=emptyResultOk)

//   @accept(ast.Ident)
//   def visitIdent(self, node, **kwargs):
//     return self.getTypeFromExpr(self.resolveExprs.visitIdent(node), True)
//
//   @accept(ast.MemberRef)
//   def visitMemberRef(self, node, **kwargs):
//     return self.getTypeFromExpr(self.resolveExprs.visitMemberRef(node), True)
//
//   @accept(ast.Specialize)
//   def visitSpecialize(self, node, **kwargs):
//     return self.getTypeFromExpr(self.resolveExprs.visitSpecialize(node, **kwargs), True)
//
//   @accept(ast.BuiltinType)
//   def visitBuiltinType(self, node):
//     return node.type
//
//   @accept(ast.Modified)
//   def visitModified(self, node):
//     mods = set()
//     if node.const:
//       mods.add(graph.ModifiedType.Modifiers.CONST)
//     if node.transitiveConst:
//       mods.add(graph.ModifiedType.Modifiers.TRANSITIVE_CONST)
//     if node.variadic:
//       mods.add(graph.ModifiedType.Modifiers.VARIADIC)
//     if node.ref:
//       mods.add(graph.ModifiedType.Modifiers.REF)
//     if len(node.args) > 0:
//       base = self.visit(node.args[0])
//       if self.isErrorExpr(base):
//         return base
//       return self.typeStore.newModifiedType(base, mods)
//     else:
//       return graph.ModifiedType().setModifiers(mods)
//
//   @accept(ast.FunctionType)
//   def visitFunctionType(self, node):
//     return self.typeStore.newFunctionType(
//         [self.visit(t) for t in node.paramTypes],
//         self.visit(node.returnType) if node.hasReturnType else None)
//
//   @accept(ast.LogicalOr)
//   def visitLogicalOr(self, node):
//     types = self.getFlatUnionType(node)
//     for t in types:
//       if self.isErrorType(t):
//         return t
//     return self.typeStore.newUnionType(types)
//
//   @accept(ast.Tuple)
//   def visitTuple(self, node):
//     return self.typeStore.newTupleType(self.visitList(node.args))

}}}
