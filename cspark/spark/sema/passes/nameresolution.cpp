#include "spark/ast/node.h"
#include "spark/ast/defn.h"
#include "spark/ast/ident.h"
#include "spark/ast/module.h"
#include "spark/ast/oper.h"
#include "spark/error/formatters.h"
#include "spark/scope/modulepathscope.h"
#include "spark/scope/scopestack.h"
#include "spark/scope/specializedscope.h"
#include "spark/sema/names/resolveexprs.h"
#include "spark/sema/names/resolverequirements.h"
#include "spark/sema/names/resolvetypes.h"
#include "spark/sema/types/essentials.h"
#include "spark/sema/passes/nameresolution.h"
#include "spark/semgraph/expr.h"
#include "spark/semgraph/module.h"
#include "spark/semgraph/package.h"
#include "spark/semgraph/type.h"
#include "spark/semgraph/types.h"
#include "spark/support/casting.h"

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace sema {
namespace passes {
using namespace semgraph;
using support::dyn_cast;
using error::formatted;

NameResolutionPass::NameResolutionPass(compiler::Context* context)
  : Pass(context)
  , _arena(nullptr)
  , _globalScopes(new scope::ScopeStack())
  , _scopeStack(new scope::ScopeStack())
  , _selfType(nullptr)
{
}

void NameResolutionPass::run(Module* mod) {
  assert(mod->ast() != nullptr);
  _arena = &mod->sgArena();

  if (_globalScopes->size() == 0) {
    Package* sparkCore = findPackage("spark.core");
    assert(sparkCore != nullptr);
    _globalScopes->push(sparkCore->memberScope()); // spark.core
    _globalScopes->push(_context->modulePathScope()); // Module search path

    // Load essential types at the same time.
    _context->essentials()->load();
    // And create constants for primitive types.
    semgraph::createConstants(*_arena);
  }

  assert(_scopeStack->size() == 0);
  *_scopeStack = *_globalScopes;
  pushAncestorScopes(mod);
  resolveImports(mod);
  exec(mod->members());
  _scopeStack->clear();
}

void NameResolutionPass::resolveImports(Module* mod) {
  const ast::Module* ast = static_cast<const ast::Module*>(mod->ast());
  for (const ast::Node* impNode : ast->imports()) {
    auto imp = static_cast<const ast::Import*>(impNode);
    std::vector<Member*> members;
    findAbsoluteSymbol(imp->path(), members);
    if (members.empty()) {
      reporter().error(imp->path()->location()) << "Imported name not found.";
      continue;
    }
    StringRef name = members.front()->name();
    if (!imp->alias().empty()) {
      name = imp->alias();
    }
    std::vector<Member*> prevMembers;
    mod->importScope()->lookupName(name, prevMembers);
    if (!prevMembers.empty()) {
      reporter().error(imp->path()->location()) <<
          "Import name '" << name << "' conflicts with previous definition.";
      continue;
    }
    for (auto m : members) {
      mod->importScope()->addMember(m);
    }
  }
}

void NameResolutionPass::visitValueDefn(ValueDefn* v) {
  if (v->type() != nullptr) {
    return;
  }
  auto vAst = static_cast<const ast::ValueDefn*>(v->ast());
  assert(vAst != nullptr);
  if (vAst->type() != nullptr) {
    v->setType(resolveType(vAst->type()));
  }
  if (vAst->init() != nullptr) {
    if (v->type() && v->type()->kind() == Type::Kind::ENUM) {
      _scopeStack->push(static_cast<Composite*>(v->type())->defn()->inheritedMemberScope());
      v->setInit(resolveExpr(vAst->init()));
      _scopeStack->pop();
    } else {
      v->setInit(resolveExpr(vAst->init()));
    }
  }
  processAttributes(v);
}

void NameResolutionPass::visitTypeDefn(TypeDefn* tdef) {
  auto tAst = static_cast<const ast::TypeDefn*>(tdef->ast());
  Defn* prevSubject = _subject.set(tdef);
  _scopeStack->push(tdef->typeParamScope());

  processRequirements(tdef, tAst->requires(), tdef->requiredFunctions());
//   typeDefn.setFriends(self.visitList(typeDefn.getFriends()))
  if (tdef->type()->kind() == Type::Kind::CLASS ||
      tdef->type()->kind() == Type::Kind::STRUCT ||
      tdef->type()->kind() == Type::Kind::INTERFACE ||
      tdef->type()->kind() == Type::Kind::ENUM) {
    resolveClassBases(static_cast<Composite*>(tdef->type()));
    Expr* selfExpr = new (*_arena) Expr(Expr::Kind::SELF, Location());
    selfExpr->setType(tdef->type());
    _scopeStack->push(tdef->inheritedMemberScope(), selfExpr);
  } else {
    _scopeStack->push(tdef->memberScope());
  }

  Type* saveSelfType = _selfType;
  _selfType = tdef->type();
  exec(tdef->members());
  _selfType = saveSelfType;
  processAttributes(tdef);
  for (auto tp : tdef->typeParams()) {
    visitTypeParameter(tp);
  }

  _scopeStack->pop();
  _scopeStack->pop();
  _subject.set(prevSubject);
}

void NameResolutionPass::visitTypeParameter(TypeParameter* t) {
  auto tpAst = static_cast<const ast::TypeParameter*>(t->ast());
  if (tpAst->type()) {
    t->setValueType(resolveType(tpAst->type()));
    assert(tpAst->subtypeConstraints().empty());
  } else {
    if (tpAst->init()) {
      Type* defaultType = resolveType(tpAst->init());
      if (!Type::isError(defaultType)) {
        t->setDefaultType(defaultType);
      }
    }
  }

  std::vector<Type*> constraintList;
  for (auto sc : tpAst->subtypeConstraints()) {
    Type* constraint = resolveType(sc);
    if (Type::isError(constraint)) {
      return;
    }
    assert(constraint != nullptr);
    constraintList.push_back(constraint);
  }
  t->setSubtypeConstraints(_arena->copyOf(constraintList));
}

void NameResolutionPass::visitParameter(Parameter* p) {
  visitValueDefn(p);
}

void NameResolutionPass::visitFunction(Function* f) {
  Defn* prevSubject = _subject.set(f);
  auto fAst = static_cast<const ast::Function*>(f->ast());
  for (auto tp : f->typeParams()) {
    visitTypeParameter(tp);
  }
  _scopeStack->push(f->typeParamScope());
  processRequirements(f, fAst->requires(), f->requiredFunctions());
  Type* returnType = &VoidType::VOID;
  if (fAst->returnType() != nullptr) {
    returnType = resolveType(fAst->returnType());
  }

  processAttributes(f);
  processParamList(f, f->params());
  if (fAst->body() != nullptr && fAst->body()->kind() != ast::Kind::ABSENT) {
    _scopeStack->push(f->paramScope());
    f->setBody(resolveExpr(fAst->body()));
    _scopeStack->pop();
  }

  _scopeStack->pop();
  _subject.set(prevSubject);

  f->setType(_context->typeStore()->createFunctionType(returnType, f->params()));
  assert(f->definedIn() != nullptr);

//     if defns.isGlobalDefn(func):
//       if func.isConstructor():
//         self.errorAt(func,
//             "Constructors cannot be declared at global scope.", func.getName())
//       elif func.hasSelfType():
//         self.errorAt(func,
//             "Global function '(0)' cannot declare a 'self' parameter.", func.getName())
//         func.clearSelfType()
//     elif defns.isStaticDefn(func):
//       if func.hasSelfType():
//         self.errorAt(func,
//             "Static function '(0)' cannot declare a 'self' parameter.", func.getName())
//         funcType.clearSelfType()
//     else:
//       if not func.hasSelfType():
//         enc = defns.getEnclosingTypeDefn(func)
//         assert enc, debug.format(func)
//         func.setSelfType(enc.getType())
//
//     if func.getName() == 'new' and not func.isStatic():
//       assert func.hasSelfType()
}

void NameResolutionPass::visitProperty(Property* p) {
  auto pAst = static_cast<const ast::Property*>(p->ast());
  Defn* prevSubject = _subject.set(p);
  for (auto tp : p->typeParams()) {
    visitTypeParameter(tp);
  }
  _scopeStack->push(p->typeParamScope());
  assert(pAst->type() != nullptr);
  p->setType(resolveType(pAst->type()));
  processAttributes(p);
  processParamList(p, p->params());
  _scopeStack->push(p->paramScope());
  processRequirements(p, pAst->requires(), p->requiredFunctions());

  if (p->getter() != nullptr) {
    p->getter()->setType(
        _context->typeStore()->createFunctionType(p->type(), p->getter()->params()));
    visitFunction(p->getter());
  }

  if (p->setter() != nullptr) {
    if (p->setter()->params().size() == p->params().size()) {
      std::vector<Parameter*>& params = p->setter()->params();
      auto valueParam = new (*_arena) Parameter(p->setter()->location(), "value", p);
      valueParam->setType(p->type());
      params.insert(params.begin(), valueParam);
      p->setter()->paramScope()->addMember(valueParam);
    }
    p->setter()->setType(
        _context->typeStore()->createFunctionType(&VoidType::VOID, p->setter()->params()));
    _scopeStack->push(p->setter()->paramScope());
    visitFunction(p->setter());
    _scopeStack->pop();
  }

  _scopeStack->pop();
  _scopeStack->pop();
  _subject.set(prevSubject);
}

void NameResolutionPass::processParamList(Defn* d, const ArrayRef<Parameter*>& paramList) {
  std::vector<Parameter*> params;
  for (auto param : paramList) {
    if (param->isSelfParam()) {
      processSelfParam(d, param);
    } else if (param->isClassParam()) {
      processClassParam(d, param);
    } else {
      visitParameter(param);
      params.push_back(param);
    }
  }

  if (d->kind() == Defn::Kind::FUNCTION) {
    Function* f = static_cast<Function*>(d);
    f->params() = params;
  } else if (d->kind() == Defn::Kind::PROPERTY) {
    Property* p = static_cast<Property*>(d);
    p->params() = params;
    if (!params.empty()) {
      if (p->getter()) {
        p->getter()->params().insert(p->getter()->params().begin(), params.begin(), params.end());
      }
      if (p->setter()) {
        p->setter()->params().insert(p->setter()->params().begin(), params.begin(), params.end());
      }
    } else if (!p->typeParams().empty() && p->selfType() == nullptr) {
      // Only count non-class parameters.
      int numTypeParams = 0;
      for (auto tp : p->typeParams()) {
        if (!tp->isClassParam()) {
          numTypeParams += 1;
        }
      }
      if (numTypeParams > 0) {
        reporter().error(p->location()) <<
            "Properties with no function parameters cannot have type parameters.";
      }
    }
  }
}

void NameResolutionPass::processSelfParam(Defn* d, Parameter* selfParam) {
  if (d->isStatic()) {
    reporter().error(selfParam->location()) <<
        "Only non-static declarations can have a 'self' parameter.";
  } else if (d->definedIn() == nullptr || d->definedIn()->kind() != Defn::Kind::TYPE) {
    reporter().error(selfParam->location()) <<
        "Members having a 'self' parameter must be members of a class or struct.";
  }
  auto pAst = static_cast<const ast::ValueDefn*>(selfParam->ast());
  assert(pAst != nullptr);
  Type* selfType;
  switch (pAst->type()->kind()) {
    case ast::Kind::CONST:
    case ast::Kind::PROVISIONAL_CONST: {
      auto astType = static_cast<const ast::UnaryOp*>(pAst->type());
      Type* typeArg = _selfType;
      if (astType->arg()->kind() != ast::Kind::ABSENT) {
        typeArg = resolveType(astType->arg());
      }
      selfType = _context->typeStore()->createConstType(
          typeArg, astType->kind() == ast::Kind::PROVISIONAL_CONST);
      break;
    }
    default:
      selfType = resolveType(pAst->type());
      if (auto selfTypeVar = dyn_cast<TypeVar*>(semgraph::types::raw(selfType))) {
        if (selfTypeVar->param()->definedIn() != d) {
          reporter().error(selfParam->location()) <<
              "Type parameter for 'self' parameter must belong to the same definition.";
        }
        selfTypeVar->param()->setSelfParam(true);
        support::ArrayBuilder<Type*> builder(*_arena, selfTypeVar->param()->subtypeConstraints());
        assert(static_cast<TypeDefn*>(d)->type() != nullptr);
        builder.append(static_cast<TypeDefn*>(d)->type());
        selfTypeVar->param()->setSubtypeConstraints(builder.build());
      }
      break;
  }

  if (d->kind() == Defn::Kind::FUNCTION) {
    static_cast<Function*>(d)->setSelfType(selfType);
  } else if (d->kind() == Defn::Kind::PROPERTY) {
    auto prop = static_cast<Property*>(d);
    prop->setSelfType(selfType);
    if (prop->getter()) {
      prop->getter()->setSelfType(selfType);
    }
    if (prop->setter()) {
      prop->setter()->setSelfType(selfType);
    }
  }
}

void NameResolutionPass::processClassParam(Defn* d, Parameter* clsParam) {
  auto pg = dyn_cast<PossiblyGenericDefn*>(d);
  assert(pg != nullptr);
  auto pAst = static_cast<const ast::ValueDefn*>(clsParam->ast());
  Type* paramType = resolveType(pAst->type());
  if (Type::isError(paramType)) {
    return;
  }
  if (auto typeVar = dyn_cast<TypeVar*>(paramType)) {
    if (typeVar->param()->definedIn() != d) {
      reporter().error(clsParam->location()) <<
          "Type parameter for 'class' parameter must belong to the same definition.";
    } else if (!d->isStatic()) {
      reporter().error(clsParam->location()) <<
          "Only static methods and properties can have a 'class' parameter.";
    } else if (d->definedIn() == nullptr || d->definedIn()->kind() != Defn::Kind::TYPE) {
      reporter().error(clsParam->location()) <<
          "Members having a 'class' parameter must be members of a class or struct.";
    } else {
      auto tdef = static_cast<TypeDefn*>(d->definedIn());
      typeVar->param()->setClassParam(true);
      support::ArrayBuilder<Type*> builder(*_arena, typeVar->param()->subtypeConstraints());
      assert(tdef->type() != nullptr);
      builder.append(tdef->type());
      typeVar->param()->setSubtypeConstraints(builder.build());
      assert(!pg->typeParams().empty());
    }
  } else {
    reporter().error(clsParam->location()) <<
        "Type for 'class' parameter must be a template parameter.";
  }
#if 0
    else:
      tparam = selfType.getParam()
      tparam.getMutableSubtypeConstraints().append(enc.getType())
#             debug.write('class param', defn, tparam)
    selfType = None
#endif
}

void NameResolutionPass::processAttributes(Defn* d) {
  auto dAst = static_cast<const ast::Defn*>(d->ast());
  for (auto a : dAst->attributes()) {
    auto attr = resolveExpr(a);
    if (Expr::isError(attr)) {
      continue;
    }
    if (attr->kind() == Expr::Kind::MEMBER_SET &&
        static_cast<MemberSet*>(attr)->genus() == MemberSet::Genus::TYPE) {
      auto call = new (*_arena) Call(Expr::Kind::CALL, dAst->location());
      call->setCallable(attr);
      d->attributes().push_back(call);
    } else {
      d->attributes().push_back(attr);
    }
  }
}

void NameResolutionPass::processRequirements(
    Defn* d,
    const ast::NodeList& astReqs,
    std::vector<RequiredFunction*>& requiredFunctions) {
  names::ResolveRequirements rr(
      _context->reporter(), _subject, _scopeStack.get(), _context->typeStore(), *_arena);
  for (auto req : astReqs) {
    rr.exec(req);
  }
}

void NameResolutionPass::resolveClassBases(Composite* cls) {
  TypeDefn* td = cls->defn();
  Type* primaryBase = nullptr;
  std::vector<Type*> baseTypes;
  // Need essential types
  auto ast = static_cast<const ast::TypeDefn*>(td->ast());
  for (auto astBase : ast->bases()) {
    Type* base = resolveType(astBase);
    if (Type::isError(base)) {
      continue;
    }
    baseTypes.push_back(base);

    Type* genericBase = base;
    Env* baseEnv = nullptr;
    if (SpecializedType* st = dyn_cast<SpecializedType*>(base)) {
      genericBase = st->generic();
      baseEnv = &st->env();
    }

    if (Composite* compBase = dyn_cast<Composite*>(genericBase)) {
      TypeDefn* defnBase = compBase->defn();
      // TODO: Not if class Object
      resolveClassBasesOutOfBand(compBase);

      if (!defnBase->typeParams().empty() && baseEnv == nullptr) {
        reporter().error(astBase->location()) << "Reference to generic type '" <<
            formatted(genericBase) << "' is missing type parameters.";
      }

      if (defnBase->isFinal()) {
        reporter().error(astBase->location()) << "Cannot inherit from type '" <<
            formatted(genericBase) << "' because it is final.";
      }

      if (!_subject.isVisible(defnBase)) {
        reporter().error(astBase->location()) << "Type '" << formatted(genericBase) <<
            "' is not visible from here.";
      }

      // Circularity check
      if (compBase->inheritsFrom(cls)) {
        reporter().error(astBase->location()) << "Cannot inherit from type '" <<
            formatted(genericBase) << "' because it is a subtype of '" << formatted(cls) << "'.";
        break;
      }
    }

    auto allowedBaseMetas = [](Type* cls) -> StringRef {
      switch (cls->kind()) {
        case Type::Kind::CLASS:
        case Type::Kind::EXTENSION:
          return "a class or interface";
        case Type::Kind::STRUCT:
          return "a struct or interface";
        case Type::Kind::INTERFACE:
          return "an interface";
        case Type::Kind::ENUM:
          return "an enum type";
        default:
          assert(false);
      }
    };

    if (cls->kind() == Type::Kind::CLASS ||
        cls->kind() == Type::Kind::STRUCT ||
        cls->kind() == Type::Kind::ENUM) {
      if (genericBase->kind() == cls->kind()) {
        if (primaryBase != nullptr) {
          reporter().error(astBase->location()) << "Type cannot inherit from '" <<
              formatted(genericBase) << "' because it already inherits from '" <<
              formatted(semgraph::types::raw(primaryBase)) << "'.";
        } else {
          primaryBase = base;
        }
      } else if (genericBase->kind() != Type::Kind::INTERFACE) {
        reporter().error(astBase->location()) << "Cannot inherit from " << formatted(genericBase) <<
            " because it is not " << allowedBaseMetas(cls) << ".";
      }
    } else if (cls->kind() == Type::Kind::INTERFACE) {
      if (genericBase->kind() != Type::Kind::INTERFACE) {
        reporter().error(astBase->location()) << "Cannot inherit from " << formatted(genericBase) <<
            " because it is not " << allowedBaseMetas(cls) << ".";
      }
    } else if (cls->kind() == Type::Kind::EXTENSION) {
      assert(false && "Implement extension.");
    }

  }

//     # Redundancy check
//     ancestors = set()
//     for base in baseTypes:
//       if isinstance(base, graph.InstantiateType):
//         base = base.getBase()
//       for base2 in types.getBaseTypes(base):
//         self.getAncestors(base2, ancestors)
//
//     for base in baseTypes:
//       if isinstance(base, graph.InstantiateType):
//         base = base.getBase()
//       if cls in ancestors:
//         self.errorAtFmt(
//             td, "Type '{0]' is inherited both directly and indirectly.", base.getDefn().getName())
//         break
//

  if (!primaryBase) {
    Composite* objectType = _context->essentials()->get(types::Essentials::Type::OBJECT);
    Composite* enumType = _context->essentials()->get(types::Essentials::Type::ENUM);
    if (cls->kind() == Type::Kind::CLASS && cls != objectType) {
      primaryBase = objectType;
    } else if (cls->kind() == Type::Kind::ENUM && cls != enumType) {
      resolveClassBasesOutOfBand(enumType);
      primaryBase = enumType;
    }
  }

  if (primaryBase) {
    cls->setSuperType(primaryBase);
    cls->defn()->inheritedMemberScope()->addScope(createScope(primaryBase));
  }

  support::ArrayBuilder<Type*> builder(*_arena);
  for (Type* base : baseTypes) {
    if (base != primaryBase) {
      builder.append(base);
      cls->defn()->inheritedMemberScope()->addScope(createScope(base));
    }
  }
  cls->setInterfaces(builder.build());
}

void NameResolutionPass::resolveClassBasesOutOfBand(Composite* cls) {
  TypeDefn* defnBase = cls->defn();
  // TODO: Not if class Object
  if (!cls->supertypesResolved()) {
    cls->setSupertypesResolved(true);
    std::auto_ptr<scope::ScopeStack> savedScopeStack = _scopeStack;
    _scopeStack.reset(new scope::ScopeStack());
    *_scopeStack = *_globalScopes;
    pushAncestorScopes(defnBase->definedIn());
    _scopeStack->push(defnBase->typeParamScope());
    Defn* prevSubject = _subject.set(defnBase);
    resolveClassBases(cls);
    _subject.set(prevSubject);
    _scopeStack = savedScopeStack;
  }
}

Expr* NameResolutionPass::resolveExpr(const ast::Node* ast) {
  names::ResolveExprs re(
      _context->reporter(), _subject, _scopeStack.get(), _context->typeStore(), *_arena);
  return re.exec(ast);
}

Type* NameResolutionPass::resolveType(const ast::Node* ast) {
  names::ResolveExprs re(
      _context->reporter(), _subject, _scopeStack.get(), _context->typeStore(), *_arena);
  Expr* expr = re.exec(ast);
  if (Expr::isError(expr)) {
    return &Type::ERROR;
  }
  names::ResolveTypes rt(_context->reporter(), _context->typeStore(), *_arena);
  return rt.exec(expr);
}

void NameResolutionPass::pushAncestorScopes(Member* m) {
  switch (m->kind()) {
    case Defn::Kind::TYPE: {
      auto td = static_cast<TypeDefn*>(m);
      if (td->definedIn() != nullptr) {
        pushAncestorScopes(td->definedIn());
      }
      _scopeStack->push(td->typeParamScope());
      _scopeStack->push(td->memberScope());
      break;
    }
    case Defn::Kind::MODULE: {
      auto mod = static_cast<Module*>(m);
      assert(mod->definedIn()->kind() == Member::Kind::PACKAGE);
      _scopeStack->push(static_cast<Package*>(mod->definedIn())->memberScope()); // This module's pkg.
      _scopeStack->push(mod->importScope()); // This module's imports
      _scopeStack->push(mod->memberScope()); // This module's top-level defines
      break;
    }
    default:
      break;
  }
}

scope::SymbolScope* NameResolutionPass::createScope(Type* t) {
  // TODO: Type alias
  // TODO: env extractor.
  if (auto spec = dyn_cast<SpecializedType*>(t)) {
    return new scope::SpecializedScope(
        createScope(spec->generic()), spec->env(), _context->typeStore());
  } else if (auto comp = dyn_cast<Composite*>(t)) {
    return comp->defn()->inheritedMemberScope();
  } else {
    assert(false && "Invalid base type.");
  }
}

void NameResolutionPass::findAbsoluteSymbol(const ast::Node* node, std::vector<Member*> &result) {
  if (node->kind() == ast::Kind::MEMBER) {
    auto memberRef = static_cast<const ast::MemberRef*>(node);
    std::vector<Member*> members;
    findAbsoluteSymbol(memberRef->base(), members);
    for (Member* m : members) {
      if (m->kind() == Member::Kind::PACKAGE) {
        static_cast<const Package*>(m)->memberScope()->lookupName(memberRef->name(), result);
      } else if (m->kind() == Member::Kind::MODULE) {
        static_cast<const Module*>(m)->memberScope()->lookupName(memberRef->name(), result);
      } else if (m->kind() == Member::Kind::TYPE) {
        static_cast<const TypeDefn*>(m)->memberScope()->lookupName(memberRef->name(), result);
      } else {
        reporter().error(node->location()) << "Invalid lookup context.";
      }
    }
  } else if (node->kind() == ast::Kind::IDENT) {
    _context->modulePathScope()->lookupName(static_cast<const ast::Ident*>(node)->name(), result);
  } else {
    reporter().fatal(node->location()) << "Invalid node kind: " << node->kind();
  }
}

Package* NameResolutionPass::findPackage(const StringRef& qname) {
  std::vector<Member*> members;
  int pos = 0;
  int end = 0;
  while (pos < qname.size()) {
    end = qname.find('.', pos);
    if (end < 0) {
      end = qname.size();
    }
    if (pos == 0) {
      _context->modulePathScope()->lookupName(qname.substr(pos, end), members);
    } else {
      std::vector<Member*> nextMembers;
      for (Member* m : members) {
        assert(m->kind() == Member::Kind::PACKAGE);
        static_cast<const Package*>(m)->memberScope()->lookupName(
            qname.substr(pos, end),
            nextMembers);
      }
      members.swap(nextMembers);
    }
    if (members.empty()) {
      // TODO: Report an error and return.
    }

    pos = end + 1;
  }

  if (members.size() == 1) {
    Member* m = members.front();
    assert(m->kind() == Member::Kind::PACKAGE);
    return static_cast<Package*>(m);
  }
  return nullptr;
}

}}}
