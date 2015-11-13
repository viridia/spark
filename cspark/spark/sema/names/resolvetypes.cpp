#include "spark/ast/node.h"
#include "spark/ast/defn.h"
#include "spark/ast/ident.h"
#include "spark/ast/module.h"
#include "spark/collections/smallset.h"
#include "spark/collections/map.h"
#include "spark/error/reporter.h"
#include "spark/scope/scopestack.h"
#include "spark/sema/names/resolvetypes.h"
#include "spark/sema/types/applyenv.h"
#include "spark/sema/types/typestore.h"
#include "spark/semgraph/type.h"
#include "spark/semgraph/types.h"
#include "spark/support/casting.h"

#if SPARK_HAVE_ALGORITHM
  #include <algorithm>
#endif

namespace spark {
namespace sema {
namespace names {
using namespace semgraph;
using support::dyn_cast;

Type* ResolveTypes::visitExpr(Expr* e) {
  _reporter.error(e->location()) << "Expression is not a type.";
  return &Type::ERROR;
}

Type* ResolveTypes::visitInvalid(Expr* e) {
  return &Type::ERROR;
}

Type* ResolveTypes::visitSpecialize(Call* e) {
  // Start by identifying the generic members to be specialized.
  if (e->callable()->kind() != Expr::Kind::MEMBER_SET) {
    _reporter.error(e->location()) << "Expression cannot be specialized.";
    return &Type::ERROR;
  }

  // Filter the list of symbols - only include templates that accept the correct number of params.
  auto mset = static_cast<MemberSet*>(e->callable());
  if (mset->genus() == MemberSet::Genus::VARIABLE) {
    _reporter.error(e->location()) << "Variables cannot be specialized.";
    return &Type::ERROR;
  } else if (mset->genus() == MemberSet::Genus::NAMESPACE) {
    _reporter.error(e->location()) << "Modules and packages cannot be specialized.";
    return &Type::ERROR;
  } else if (mset->genus() != MemberSet::Genus::TYPE) {
    _reporter.error(e->location()) << "Expression is not a type.";
    return &Type::ERROR;
  }

  // If the member list couldn't be filled in this phase, we'll need to work out the
  // specialization in the type inference pass.
  if (mset->genus() == MemberSet::Genus::INCOMPLETE) {
    assert(false && "Implement lazy specialization.");
//     result = graph.ExplicitSpecialize().setLocation(node.location)
//     result.setBase(mlist)
//     result.getMutableTypeArgs().extend(templateArgs)
//     return result
    return &Type::ERROR;
  }

  std::vector<Type*> results;
  for (auto m : mset->members()) {
    assert(m->kind() == Defn::Kind::TYPE);
    auto td = static_cast<TypeDefn*>(m);
    Env env;
    if (specialize(td->typeParams(), e->arguments(), env)) {
      auto st = new (_arena) SpecializedType(td->type(), env);
      results.push_back(st);
    }
  }

  if (results.size() == 1) {
    return results.front();
  } else if (results.size() == 0) {
    // TODO: Type parameters cannot be specialized, nor can any other member type
    _reporter.error(e->location()) << "No compatible template definition found for arguments.";
//     if (members.size() > 0) {
//       _reporter.info() << "Templates with matching names:";
//     }
    return &Type::ERROR;
  } else {
    return new (_arena) TypeSet(_arena.copyOf(results));
  }
}

Type* ResolveTypes::visitMemberSet(MemberSet* mset) {
  if (mset->genus() != MemberSet::Genus::TYPE) {
    _reporter.error(mset->location()) << "Expression is not a type: " << mset->name();
    return &Type::ERROR;
  }
  collections::SmallSet<Type*, 6> types;
  for (auto m : mset->members()) {
    Type* t = _typeStore->memberType(m);
    assert(t != nullptr);
    if (!Type::isError(t)) {
      types.insert(t);
    }
  }

  if (types.size() == 1) {
    return *types.begin();
  }

//       else:
//         types = set()
//         for m in expr.getMembers():
//           t = defns.getMemberType(m)
//           if not self.isErrorType(t):
//             types.add(t)
//         if len(types) == 1:
//           (t,) = types
//           return t
//         else:
//           return self.typeStore.newTypeSet(types)
  assert(false && "Implement ResolveTypes::visitMemberSet / TypeSet.");
}

bool ResolveTypes::specialize(
    const ArrayRef<TypeParameter*>& params,
    const ArrayRef<Expr*>& args,
    Env& result) {

  // If there are more type arguments than there are parameters, reject this template.
  if (args.size() > params.size()) {
    return false;
  }

  // If there are more not enough arguments to satisfy non-default params, then also reject.
  int numNonDefaultParams = 0;
  for (auto tp : params) {
    if (tp->defaultType() != nullptr) {
      break;
    }
    numNonDefaultParams += 1;
  }

  if (args.size() < numNonDefaultParams) {
    return false;
  }

  // Assign arguments to parameters.
  // TODO: Keyword assignment?
  collections::SmallMap<TypeVar*, Type*, 8> envMap;
  for (int i = 0; i < params.size(); ++i) {
    TypeParameter* tparam = params[i];
    if (tparam->valueType() != nullptr) {
      // Create a ValueType object here.
      assert(false && "Implement type params with value types.");
    } else {
      Type* typeArg = nullptr;
      if (i < args.size()) {
        // If it's an explicit argument, transform it into a type.
        typeArg = exec(args[i]);
      } else if (tparam->defaultType() != nullptr) {
        // If it's a default argument, apply the partially-built environment map to it.
        typeArg = _apply.exec(tparam->defaultType(), envMap);
      } else {
        assert(false && "arity mismatch - should have been filtered.");
      }
      if (Type::isError(typeArg)) {
        return false;
      }
      envMap[tparam->typeVar()] = typeArg;
    }
  }

  result = _typeStore->createEnv(envMap);
  return true;
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

Type* ResolveTypes::visitPack(MultiArgOp* e) {
  std::vector<Type*> typeArgs;
  for (auto arg : e->arguments()) {
    Type* t = exec(arg);
    if (Type::isError(t)) {
      return t;
    }
    typeArgs.push_back(t);
  }
  return _typeStore->createTupleType(typeArgs);
}

Type* ResolveTypes::visitConstType(UnaryOp* e) {
  Type* t = exec(e->arg());
  if (Type::isError(t)) {
    return t;
  }
  return _typeStore->createConstType(t, false);
}

Type* ResolveTypes::visitProvisionalConstType(UnaryOp* e) {
  Type* t = exec(e->arg());
  if (Type::isError(t)) {
    return t;
  }
  return _typeStore->createConstType(t, true);
}

Type* ResolveTypes::visitUnionType(MultiArgOp* e) {
  std::vector<Type*> typeArgs;
  for (auto arg : e->arguments()) {
    Type* t = exec(arg);
    if (Type::isError(t)) {
      return t;
    }
    typeArgs.push_back(t);
  }
  return _typeStore->createUnionType(typeArgs);
}

Type* ResolveTypes::visitFunctionType(Call* e) {
  Type* ret;
  if (e->callable()->kind() != Expr::Kind::IGNORED) {
    Type* ret = exec(e->callable());
    if (Type::isError(ret)) {
      return ret;
    }
  } else {
    ret = &Type::IGNORED;
  }
  std::vector<Type*> params;
  params.reserve(e->arguments().size());
  for (auto p : e->arguments()) {
    Type* paramType = exec(p);
    if (Type::isError(paramType)) {
      return paramType;
    }
    params.push_back(paramType);
  }
  return _typeStore->createFunctionType(ret, params);
}

Type* ResolveTypes::visitOptionalType(UnaryOp* e) {
  Type* t = exec(e->arg());
  if (Type::isError(t)) {
    return t;
  }
  // TODO: Flatten.
  Type* rawType = semgraph::types::raw(t);
  collections::SmallSet<Type*, 2> unionMembers;
  unionMembers.insert(rawType);
  if (rawType->kind() == Type::Kind::CLASS) {
    unionMembers.insert(&NullPtrType::NULLPTR);
  } else {
    unionMembers.insert(&VoidType::VOID);
  }
  return _typeStore->createUnionType(unionMembers);
}

}}}
