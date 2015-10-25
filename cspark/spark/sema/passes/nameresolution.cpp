#include "spark/ast/node.h"
#include "spark/ast/defn.h"
#include "spark/ast/ident.h"
#include "spark/ast/module.h"
#include "spark/scope/modulepathscope.h"
#include "spark/scope/scopestack.h"
#include "spark/sema/names/resolveexprs.h"
#include "spark/sema/names/resolvetypes.h"
#include "spark/sema/passes/nameresolution.h"
#include "spark/semgraph/expr.h"
#include "spark/semgraph/module.h"
#include "spark/semgraph/package.h"
#include "spark/semgraph/type.h"
#include "spark/support/casting.h"

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace sema {
namespace passes {
using namespace semgraph;
using support::dyn_cast;

NameResolutionPass::NameResolutionPass(compiler::Context* context)
  : Pass(context)
  , _arena(nullptr)
  , _scopeStack(new scope::ScopeStack())
{
}

void NameResolutionPass::run(Module* mod) {
  assert(mod->ast() != nullptr);
  _arena = &mod->sgArena();

//   if self.compiler.packageMgr:
//     self.loadEssentialTypes(scopeStack)

  // Bottom scope is builtins and core libraries
  assert(_scopeStack->size() == 0);
  Package* sparkCore = findPackage("spark.core");
  assert(sparkCore != nullptr);
#if 0
  scopeStack.push(primitivetypes.MODULE.getMemberScope())
  if self.compiler.packageMgr:
    scopeStack.pushPackage(self.SPARK_CORE)
#endif
  _scopeStack->push(sparkCore->memberScope()); // spark.core
  _scopeStack->push(_context->modulePathScope()); // Module search path
  assert(mod->definedIn()->kind() == Member::Kind::PACKAGE);
  _scopeStack->push(static_cast<Package*>(mod->definedIn())->memberScope()); // This module's pkg.
  _scopeStack->push(mod->importScope()); // This module's imports
  _scopeStack->push(mod->memberScope()); // This module's top-level defines

//  const ast::Module* ast = static_cast<const ast::Module*>(mod->ast());
//  assert(ast->kind() == ast::Kind::MODULE);
//  createMembers(ast->members(), mod, mod->members(), mod->memberScope());
//   for (const ast::Node* node : ast->imports()) {
//     reporter().fatal(node->location()) << "Implement import.";
//     assert(false && node);
//   }
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
//   reporter().debug() << "value " << v->name();
  auto vAst = static_cast<const ast::ValueDefn*>(v->ast());
  if (vAst->type() != nullptr) {
    v->setType(resolveType(vAst->type()));
//     assert type(vdef.getType()) != graph.Type
  }
  if (vAst->init() != nullptr) {
    if (v->type() && v->type()->kind() == Type::Kind::ENUM) {
      _scopeStack->push(static_cast<Composite*>(v->type())->defn()->inheritedMemberScope());
//      vdef.setInit(self.resolveExprs.visit(vdef.astInit))
      _scopeStack->pop();
    } else {
//       init = self.resolveExprs.visit(vdef.astInit)
//       if not self.isErrorExpr(init):
//         vdef.setInit(init)
    }
  }
//   self.processAttributes(defn)
}

void NameResolutionPass::visitTypeDefn(TypeDefn* tdef) {
  Defn* prevSubject = _subject.set(tdef);
  _scopeStack->push(tdef->typeParamScope());

//   resolverequirements.ResolveRequirements(
//       self.errorReporter, self.resolveExprs, self.resolveTypes, typeDefn).run()
//   typeDefn.setFriends(self.visitList(typeDefn.getFriends()))
  if (tdef->type()->kind() == Type::Kind::CLASS ||
      tdef->type()->kind() == Type::Kind::STRUCT ||
      tdef->type()->kind() == Type::Kind::INTERFACE ||
      tdef->type()->kind() == Type::Kind::ENUM) {
    resolveClassBases(static_cast<Composite*>(tdef->type()));
    // TODO: Add base/stem
    _scopeStack->push(tdef->inheritedMemberScope());
  } else {
    _scopeStack->push(tdef->memberScope());
  }

  exec(tdef->members());
//   for item in defn.getTypeParams():
//     self.visit(item)
//   self.processAttributes(defn)

  _scopeStack->pop();
  _scopeStack->pop();
  _subject.set(prevSubject);
}

void NameResolutionPass::visitTypeParameter(TypeParameter* t) {
  assert(false && "Implement NameResolutionPass::visitTypeParameter");
}

void NameResolutionPass::visitParameter(Parameter* p) {
  assert(false && "Implement NameResolutionPass::visitParameter");
}

void NameResolutionPass::visitFunction(Function* f) {
  Defn* prevSubject = _subject.set(f);
  _subject.set(prevSubject);
}

void NameResolutionPass::visitProperty(Property* p) {
  Defn* prevSubject = _subject.set(p);
  _subject.set(prevSubject);
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

    Type* unspecBase = base;
    if (SpecializedType* st = dyn_cast<SpecializedType*>(base)) {
      unspecBase = st->generic();
    }

    (void)primaryBase;

//
//       if (plainBase not in self.baseClassesResolved and
//           plainBase is not objectType and
//           not plainBase.hasSuperType()):
//         savedSubject = self.nameLookup.getSubject()
//         savedScopeStack = self.nameLookup.scopeStack
//         self.nameLookup.scopeStack = self.getScopeStackForDefn(plainBase.getDefn())
//         self.nameLookup.setSubject(plainBase.getDefn())
//         self.resolveClassBases(plainBase)
//         self.nameLookup.setSubject(savedSubject)
//         self.nameLookup.scopeStack = savedScopeStack
//         assert plainBase in self.baseClassesResolved

  }
}

Type* NameResolutionPass::resolveType(const ast::Node* ast) {
  names::ResolveExprs re(_context->reporter(), _subject, _scopeStack.get(), _tempArena);
  Expr* expr = re.exec(ast);
  if (Expr::isError(expr)) {
    return &Type::ERROR;
  }
  names::ResolveTypes rt(_context->reporter(), _context->typeStore(), *_arena);
  Type* type = rt.exec(expr);
  _tempArena.clear();
  return type;
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
