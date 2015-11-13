#include "spark/ast/node.h"
#include "spark/ast/defn.h"
#include "spark/ast/module.h"
#include "spark/sema/passes/buildgraph.h"
#include "spark/semgraph/module.h"
#include "spark/semgraph/type.h"

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace sema {
namespace passes {

void BuildGraphPass::run(semgraph::Module* mod) {
  assert(mod->ast() != nullptr);
  _arena = &mod->sgArena();
  const ast::Module* ast = static_cast<const ast::Module*>(mod->ast());
  assert(ast->kind() == ast::Kind::MODULE);
  createMembers(ast->members(), mod, mod->members(), mod->memberScope());
//   for (const ast::Node* node : ast->imports()) {
//     reporter().fatal(node->location()) << "Implement import.";
//     assert(false && node);
//   }
}

void BuildGraphPass::createMembers(
    const ast::NodeList& memberAsts,
    semgraph::Member* parent,
    semgraph::MemberList& memberList,
    scope::StandardScope* memberScope) {

  memberList.reserve(memberAsts.size());
  for (const ast::Node* node : memberAsts) {
    auto ast = static_cast<const ast::Defn*>(node);
    semgraph::Defn* d = createDefn(node, parent);
    d->setAst(ast);
    d->setVisibility(astVisibility(ast));
    d->setAbstract(ast->isAbstract());
    d->setFinal(ast->isFinal());
    d->setOverride(ast->isOverride());
    d->setUndef(ast->isUndef());
    d->setStatic(ast->isStatic());
    memberList.push_back(d);
    memberScope->addMember(d);

    if (node->kind() == ast::Kind::OBJECT_DEFN) {
      semgraph::ValueDefn* singleton = new semgraph::ValueDefn(
          semgraph::Member::Kind::LET, ast->location(), ast->name(), parent);
      singleton->setType(static_cast<semgraph::TypeDefn*>(d)->type());
      singleton->setVisibility(astVisibility(ast));
      memberList.push_back(singleton);
      memberScope->addMember(singleton);
    }

//     typeRef = graph.MemberList().setLocation(decl.getLocation())
//     typeRef.setListType(graph.MemberListType.TYPE)
//     typeRef.getMutableMembers().append(decl)
//
//     initCtor = graph.MemberList().setLocation(decl.getLocation())
//     initCtor.setName('new')
//     initCtor.setBase(typeRef)
//     initCtor.setListType(graph.MemberListType.INCOMPLETE)
//
//     initCall = graph.Call().setLocation(decl.getLocation())
//     initCall.getMutableArgs().append(initCtor)
//     singleton.setInit(initCall)
//     return singleton

//       instanceScope = StandardScope(decl, 'member', scopeType=ScopeType.INSTANCE)
//       if isinstance(decl.getType(), graph.Class) and decl.getType().isObject():
//         singleton = self.buildSingletonObject(decl)
//         singleton.setDefinedIn(enclosure)
// #         memberScope.addMember(singleton)
//         enclosure.getMembers().append(singleton)
//       decl.setMemberScope(instanceScope)
//       decl.setTypeParamScope(StandardScope(decl, 'type param'))
//       decl.setRequiredMethodScope(StandardScope(decl, 'constraint'))
//       for member in decl.getMembers(): # @type member: spark.ast.Defn
//         self.buildDefn(member, decl, decl.getMemberScope())

  }
}

semgraph::Defn* BuildGraphPass::createDefn(const ast::Node * node, semgraph::Member* parent) {
  switch (node->kind()) {
    case ast::Kind::LET:
    case ast::Kind::VAR: {
      const ast::ValueDefn* ast = static_cast<const ast::ValueDefn*>(node);
      semgraph::Member::Kind kind = node->kind() == ast::Kind::LET ?
          semgraph::Member::Kind::LET : semgraph::Member::Kind::VAR;
      semgraph::ValueDefn* vd = new semgraph::ValueDefn(
          kind, ast->location(), ast->name(), parent);
      vd->setAst(ast);
      return vd;
    }
    case ast::Kind::ENUM_VALUE: {
      const ast::ValueDefn* ast = static_cast<const ast::ValueDefn*>(node);
      semgraph::ValueDefn* vd = new semgraph::ValueDefn(
          semgraph::Member::Kind::ENUM_VAL, ast->location(), ast->name(), parent);
      vd->setAst(ast);
      return vd;
    }
    case ast::Kind::CLASS_DEFN:
    case ast::Kind::STRUCT_DEFN:
    case ast::Kind::INTERFACE_DEFN:
    case ast::Kind::OBJECT_DEFN: {
      const ast::TypeDefn* ast = static_cast<const ast::TypeDefn*>(node);
      std::string typeName(ast->name().begin(), ast->name().end());
      if (node->kind() == ast::Kind::OBJECT_DEFN) {
        typeName.append("#Class");
      }
      semgraph::TypeDefn* td = new semgraph::TypeDefn(
          semgraph::Member::Kind::TYPE, ast->location(), typeName, parent);

      semgraph::Type::Kind tk = semgraph::Type::Kind::CLASS;
      if (node->kind() == ast::Kind::STRUCT_DEFN) {
        tk = semgraph::Type::Kind::STRUCT;
      } else if (node->kind() == ast::Kind::INTERFACE_DEFN) {
        tk = semgraph::Type::Kind::INTERFACE;
      }
      semgraph::Composite* cls = new (arena()) semgraph::Composite(tk);
      td->setType(cls);
      cls->setDefn(td);

      createMembers(ast->members(), td, td->members(), td->memberScope());
      createTypeParamList(ast->typeParams(), td, td->typeParams(), td->typeParamScope());
//       decl.setRequiredMethodScope(StandardScope(decl, 'constraint'))
      return td;
    }
    case ast::Kind::ENUM_DEFN: {
      const ast::TypeDefn* ast = static_cast<const ast::TypeDefn*>(node);
      semgraph::TypeDefn* td = new semgraph::TypeDefn(
          semgraph::Member::Kind::TYPE, ast->location(), ast->name(), parent);

      semgraph::Composite* cls = new (arena()) semgraph::Composite(semgraph::Type::Kind::ENUM);
      td->setType(cls);
      cls->setDefn(td);

      createMembers(ast->members(), td, td->members(), td->memberScope());
      return td;
    }
    case ast::Kind::FUNCTION: {
      const ast::Function* ast = static_cast<const ast::Function*>(node);
      semgraph::Function* f = new semgraph::Function(ast->location(), ast->name(), parent);
      createParamList(ast->params(), f, f->params(), f->paramScope());
      createTypeParamList(ast->typeParams(), f, f->typeParams(), f->typeParamScope());
      return f;
    }
    case ast::Kind::PROPERTY: {
      const ast::Property* ast = static_cast<const ast::Property*>(node);
      semgraph::Property* prop = new semgraph::Property(ast->location(), ast->name(), parent);
      createParamList(ast->params(), prop, prop->params(), prop->paramScope());
      createTypeParamList(ast->typeParams(), prop, prop->typeParams(), prop->typeParamScope());
      if (ast->getter() != nullptr) {
        semgraph::Function* getter = new semgraph::Function(
            ast->getter()->location(), ast->getter()->name(), prop);
        getter->setAst(ast->getter());
        prop->setGetter(getter);
      }
      if (ast->setter() != nullptr) {
        semgraph::Function* setter = new semgraph::Function(
            ast->setter()->location(), ast->setter()->name(), prop);
        setter->setAst(ast->setter());
        prop->setSetter(setter);
      }
      return prop;
    }
    default:
      reporter().fatal(node->location()) << "Invalid member node type: " << node->kind();
      return nullptr;
  }
}

void BuildGraphPass::createParamList(
    const ast::NodeList& paramAsts,
    semgraph::Member* parent,
    std::vector<semgraph::Parameter*>& paramList,
    scope::StandardScope* paramScope) {

  paramList.reserve(paramAsts.size());
  for (const ast::Node* node : paramAsts) {
    assert(node->kind() == ast::Kind::PARAMETER);
    const ast::Parameter* ast = static_cast<const ast::Parameter*>(node);
    semgraph::Parameter* param = new semgraph::Parameter(ast->location(), ast->name(), parent);
    param->setAst(ast);
    param->setKeywordOnly(ast->isKeywordOnly());
    param->setVariadic(ast->isVariadic());
    param->setSelfParam(ast->isSelfParam());
    param->setClassParam(ast->isClassParam());
    param->setExpansion(ast->isExpansion());

    paramList.push_back(param);
    paramScope->addMember(param);
  }
}

void BuildGraphPass::createTypeParamList(
    const ast::NodeList& paramAsts,
    semgraph::Member* parent,
    std::vector<semgraph::TypeParameter*>& paramList,
    scope::StandardScope* paramScope) {

  paramList.reserve(paramAsts.size());
  for (const ast::Node* node : paramAsts) {
    assert(node->kind() == ast::Kind::TYPE_PARAMETER);
    const ast::TypeParameter* ast = static_cast<const ast::TypeParameter*>(node);
    semgraph::TypeParameter* param = new semgraph::TypeParameter(
        ast->location(), ast->name(), parent);
    param->setAst(ast);
    param->setVariadic(ast->isVariadic());
    param->setTypeVar(new (arena()) semgraph::TypeVar(param));
    paramList.push_back(param);
    paramScope->addMember(param);
  }
}

semgraph::Visibility BuildGraphPass::astVisibility(const ast::Defn* d) {
  if (d->isPrivate()) {
    return semgraph::PRIVATE;
  } else if (d->isProtected()) {
    return semgraph::PROTECTED;
  } else {
    return semgraph::PUBLIC;
  }
}

support::Arena& BuildGraphPass::arena() {
  assert(_arena != nullptr);
  return *_arena;
}

}}}
