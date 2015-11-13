#include "spark/ast/node.h"
#include "spark/ast/oper.h"
#include "spark/ast/ident.h"
#include "spark/collections/smallset.h"
#include "spark/error/reporter.h"
#include "spark/scope/closematch.h"
#include "spark/scope/scopestack.h"
#include "spark/scope/stdscope.h"
#include "spark/sema/names/fillmemberset.h"
#include "spark/sema/names/memberlookup.h"
#include "spark/sema/names/resolveexprs.h"
#include "spark/sema/names/resolverequirements.h"
#include "spark/sema/names/resolvetypes.h"
#include "spark/sema/names/subject.h"
#include "spark/semgraph/requirement.h"
#include "spark/support/casting.h"

#if SPARK_HAVE_ALGORITHM
  #include <algorithm>
#endif

namespace spark {
namespace sema {
namespace names {
using namespace semgraph;
using support::dyn_cast;

bool ResolveRequirements::exec(const ast::Node* node) {
  (void)_arena;
  switch (node->kind()) {
    case ast::Kind::CALL_REQUIRED:
    case ast::Kind::CALL_REQUIRED_STATIC:
      return visitCallRequired(static_cast<const ast::Oper*>(node));
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
    default:
      _reporter.fatal() << "Invalid requirement AST: " << node->kind();
      return false;
  }
}

bool ResolveRequirements::visitCallRequired(const ast::Oper* ast) {
  const ast::Node* callable = ast->op();
  StringRef funcName;

  collections::SmallSet<Member*, 8> lookupContexts;
  if (callable->kind() == ast::Kind::IDENT) {
    funcName = static_cast<const ast::Ident*>(callable)->name();
  } else if (callable->kind() == ast::Kind::MEMBER) {
    auto mref = static_cast<const ast::MemberRef*>(callable);
    funcName = mref->name();
    if (!resolveLookupContexts(mref->base(), lookupContexts)) {
      return false;
    }
  }

  std::vector<Parameter*> params;

  auto reqFunc = new Function(ast->location(), funcName, _subject.get());
  if (ast->kind() == ast::Kind::CALL_REQUIRED_STATIC) {
    reqFunc->setStatic(true);
  }
  reqFunc->setRequirement(true);
  Type* t = resolveType(ast->operands()[0]);
  if (Type::isError(t)) {
    return false;
  }
  auto ft = dyn_cast<FunctionType*>(t);
  assert(ft != nullptr);
  getFunctionParams(ft, reqFunc, reqFunc->params());

  if (reqFunc->name() == "new") {
    if (ft->returnType() != &Type::IGNORED) {
      _reporter.error(ast->location()) << "Requirement for 'new' should not declare a return type";
      return false;
    } else if (lookupContexts.empty()) {
      _reporter.error(ast->location()) <<
          "Requirement 'new' must be of the form typename.new(argtypes...)";
      return false;
    } else if (lookupContexts.size() > 1) {
      _reporter.error(ast->location()) << "Ambiguous type name '" <<
          (*lookupContexts.begin())->name() << "'.";
      return false;
    }
    reqFunc->setConstructor(true);
    auto member = *lookupContexts.begin();
    if (auto tp = dyn_cast<TypeParameter*>(member)) {
      ft = _typeStore->createFunctionType(tp->typeVar(), ft->paramTypes());
    } else {
      _reporter.error(ast->location()) << "Expected scope of 'new' to be a type parameter.";
    }
  } else if (ft->returnType() == nullptr) {
    assert(false && "Missing return type.");
  }

  reqFunc->setType(ft);
  auto req = new (_arena) RequiredFunction(reqFunc);
  support::ArrayBuilder<Member*> builder(_arena, lookupContexts);
  req->setLookupContexts(builder.build());

  if (!lookupContexts.empty()) {
    MemberSet::Genus genus = FillMemberSet::genusOf(lookupContexts);
    if (genus == MemberSet::Genus::NAMESPACE) {
      reqFunc->setStatic(true);
    }

    if (auto pg = dyn_cast<PossiblyGenericDefn*>(_subject.get())) {
      pg->requiredFunctions().push_back(req);
      for (auto m : lookupContexts) {
        scope::StandardScope* s;
        auto it = pg->interceptScopes().find(m);
        if (it == pg->interceptScopes().end()) {
          s = new scope::StandardScope(scope::SymbolScope::INTERCEPT, pg);
          pg->interceptScopes()[m] = s;
        } else {
          s = it->second;
        }
        s->addMember(reqFunc);
      }
    }
  } else {
    addTargetRequirement(req);
  }
  return true;
}

bool ResolveRequirements::visitEqual(const ast::Oper* ast) {
  assert(ast->operands().size() == 2);
  relationalOp(ast->location(), "==", ast->operands()[0], ast->operands()[1]);
  return true;
}

bool ResolveRequirements::visitRefEqual(const ast::Oper* ast) {
  assert(ast->operands().size() == 2);
  _reporter.error(ast->location()) << "Invalid 'where' condition (reference equality).";
  return false;
}

bool ResolveRequirements::visitNotEqual(const ast::Oper* ast) {
  assert(ast->operands().size() == 2);
  relationalOp(ast->location(), "!=", ast->operands()[0], ast->operands()[1]);
  return true;
}

bool ResolveRequirements::visitLessThan(const ast::Oper* ast) {
  assert(ast->operands().size() == 2);
  relationalOp(ast->location(), "<", ast->operands()[0], ast->operands()[1]);
  return true;
}

bool ResolveRequirements::visitGreaterThan(const ast::Oper* ast) {
  assert(ast->operands().size() == 2);
  relationalOp(ast->location(), ">", ast->operands()[0], ast->operands()[1]);
  return true;
}

bool ResolveRequirements::visitLessThanOrEqual(const ast::Oper* ast) {
  assert(ast->operands().size() == 2);
  relationalOp(ast->location(), "<=", ast->operands()[0], ast->operands()[1]);
  return true;
}

bool ResolveRequirements::visitGreaterThanOrEqual(const ast::Oper* ast) {
  assert(ast->operands().size() == 2);
  relationalOp(ast->location(), ">=", ast->operands()[0], ast->operands()[1]);
  return true;
}

void ResolveRequirements::relationalOp(
    const source::Location& loc,
    const StringRef& name,
    const ast::Node* left,
    const ast::Node* right) {
  Function* reqFunc = new Function(loc, name, _subject.get());
  reqFunc->setRequirement(true);
  reqFunc->setStatic(true);

  auto leftType = resolveType(left);
  auto rightType = resolveType(right);

  auto p = new Parameter(loc, "left", reqFunc);
  p->setType(leftType);
  reqFunc->params().push_back(p);

  p = new Parameter(loc, "right", reqFunc);
  p->setType(rightType);
  reqFunc->params().push_back(p);

  reqFunc->setType(_typeStore->createFunctionType(&BooleanType::BOOL, { leftType, rightType }));
  addTargetRequirement(new (_arena) RequiredFunction(reqFunc));
}

void ResolveRequirements::addTargetRequirement(RequiredFunction* req) {
  if (auto pg = dyn_cast<PossiblyGenericDefn*>(_subject.get())) {
    pg->requiredFunctions().push_back(req);
    pg->requiredMethodScope()->addMember(req->method());
  } else {
    assert(false);
  }
}

bool ResolveRequirements::resolveLookupContexts(
    const ast::Node* node,
    SmallSetBase<Member*>& result) {
  if (node->kind() == ast::Kind::IDENT) {
    auto ident = static_cast<const ast::Ident*>(node);
    scope::NameLookupResult lookupResult;
    _scopeStack->find(ident->name(), lookupResult);
    if (lookupResult.members.empty()) {
      scope::CloseMatchFinder matcher(ident->name());
      _scopeStack->forAllNames(matcher);
      if (matcher.closest().empty()) {
        _reporter.error(ident->location()) << "Name '" << ident->name() << "' not found.";
      } else {
        _reporter.error(ident->location()) << "Name '" << ident->name() <<
            "' not found, did you mean '" << matcher.closest() << "'?";
      }
      return false;
    }
    for (auto m : lookupResult.members) {
      result.insert(m);
    }
  } else if (node->kind() == ast::Kind::MEMBER) {
    auto mref = static_cast<const ast::MemberRef*>(node);
    collections::SmallSet<Member*, 8> baseMembers;
    if (!resolveLookupContexts(mref->base(), baseMembers)) {
      return false;
    }

    collections::SmallSet<Member*, 8> members;
    MemberLookup memberLookup(_reporter, _typeStore);
    memberLookup.lookup(mref->name(), baseMembers, true, result);

    if (result.empty()) {
      scope::CloseMatchFinder matcher(mref->name());
      memberLookup.forAllNames(baseMembers, matcher);
      if (matcher.closest().empty()) {
        _reporter.error(mref->location()) << "Name '" << mref->name() << "' not found.";
      } else {
        _reporter.error(mref->location()) << "Name '" << mref->name() <<
            "' not found, did you mean '" << matcher.closest() << "'?";
      }
      return false;
    }
  } else {
    _reporter.error(node->location()) << "Invalid requirement form.";
    return false;
  }

  // These need to be types or namespaces.
  for (auto m : result) {
    if (m->kind() == Member::Kind::SPECIALIZED) {
      m = static_cast<SpecializedMember*>(m)->generic();
    }
    if (m->kind() != Member::Kind::TYPE &&
        m->kind() != Member::Kind::PACKAGE &&
        m->kind() != Member::Kind::MODULE &&
        m->kind() != Member::Kind::TYPE_PARAM) {
      _reporter.error(node->location()) << "Invalid lookup context for required function.";
      return false;
    }
  }

  return true;
}

bool ResolveRequirements::getFunctionParams(
    const ast::NodeList& astParams,
    Defn* definedIn,
    std::vector<Parameter*>& result) {
  for (auto param : astParams) {
    std::stringstream paramName;
    paramName << "_" << result.size();
    auto paramType = resolveType(param);
    auto p = new Parameter(param->location(), paramName.str(), definedIn);
    p->setType(paramType);
    result.push_back(p);
  }
  return true;
}

void ResolveRequirements::getFunctionParams(
    FunctionType* ft,
    Defn* definedIn,
    std::vector<Parameter*>& result) {
  for (auto paramType : ft->paramTypes()) {
    std::stringstream paramName;
    paramName << "_" << result.size();
    auto p = new Parameter(definedIn->location(), paramName.str(), definedIn);
    p->setType(paramType);
    result.push_back(p);
  }
}

Type* ResolveRequirements::resolveType(const ast::Node* node) {
  ResolveExprs re(_reporter, _subject, _scopeStack, _typeStore, _arena);
  Expr* expr = re.exec(node);
  if (Expr::isError(expr)) {
    return &Type::ERROR;
  }
  ResolveTypes rt(_reporter, _typeStore, _arena);
  return rt.exec(expr);
}

}}}
