#include "spark/ast/node.h"
#include "spark/ast/defn.h"
#include "spark/ast/ident.h"
#include "spark/ast/module.h"
#include "spark/scope/modulepathscope.h"
#include "spark/scope/scopestack.h"
#include "spark/sema/names/resolvetypes.h"
#include "spark/sema/passes/nameresolution.h"
#include "spark/semgraph/module.h"
#include "spark/semgraph/package.h"
#include "spark/semgraph/type.h"

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace sema {
namespace passes {
using namespace semgraph;

NameResolutionPass::NameResolutionPass(compiler::Context* context)
  : Pass(context)
  , _arena(NULL)
  , _scopeStack(new scope::ScopeStack())
  , _resolveTypes(new names::ResolveTypes(reporter(), context->typeStore(), _scopeStack.get()))
{
}

void NameResolutionPass::run(Module* mod) {
  assert(mod->ast() != NULL);
  _arena = &mod->sgArena();

//   if self.compiler.packageMgr:
//     self.loadEssentialTypes(scopeStack)

  // Bottom scope is builtins and core libraries
  assert(_scopeStack->size() == 0);
#if 0
  scopeStack.push(primitivetypes.MODULE.getMemberScope())
  if self.compiler.packageMgr:
    scopeStack.pushPackage(self.SPARK_CORE)
    # Next is the module search path
    scopeStack.push(self.compiler.packageMgr)
    # Next is the same package as the module being compiled
    packageParts = self.getModulePackageParts(mod)
    if packageParts != self.SPARK_CORE:
      scopeStack.pushPackage(packageParts)
#endif
//   assert mod.hasImportScope(), mod.getName()
  // Then this module's imports
  _scopeStack->push(mod->importScope());
  // And finally this module's top-level defines
  _scopeStack->push(mod->memberScope());

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
  reporter().debug() << "value " << v->name();
}

void NameResolutionPass::visitTypeDefn(TypeDefn* tdef) {
  reporter().debug() << "type " << tdef->name();
  reporter().indent();

  Defn* prevSubject = _subject.set(tdef);
  _scopeStack->push(tdef->typeParamScope());

//   resolverequirements.ResolveRequirements(
//       self.errorReporter, self.resolveExprs, self.resolveTypes, typeDefn).run()
//   typeDefn.setFriends(self.visitList(typeDefn.getFriends()))
  if (tdef->type()->kind() == Type::Kind::CLASS ||
      tdef->type()->kind() == Type::Kind::STRUCT ||
      tdef->type()->kind() == Type::Kind::INTERFACE ||
      tdef->type()->kind() == Type::Kind::ENUM) {
    //self.resolveClassBases(typeDefn.getType())
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

  reporter().unindent();
}

void NameResolutionPass::visitTypeParameter(TypeParameter* t) {
  reporter().debug() << "type param " << t->name();
}

void NameResolutionPass::visitParameter(Parameter* p) {
  reporter().debug() << "param " << p->name();
}

void NameResolutionPass::visitFunction(Function* f) {
  Defn* prevSubject = _subject.set(f);
  reporter().debug() << "func " << f->name();
  _subject.set(prevSubject);
}

void NameResolutionPass::visitProperty(Property* p) {
  Defn* prevSubject = _subject.set(p);
  reporter().debug() << "prop " << p->name();
  _subject.set(prevSubject);
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

}}}
