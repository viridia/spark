#include "spark/error/reporter.h"
#include "spark/sema/names/memberlookup.h"
#include "spark/semgraph/defn.h"
#include "spark/semgraph/expr.h"
#include "spark/semgraph/module.h"
#include "spark/semgraph/package.h"
#include "spark/support/arraybuilder.h"
#include "spark/support/casting.h"

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace sema {
namespace names {
using namespace semgraph;
using support::isa;
using support::dyn_cast;

void MemberLookup::lookup(
    const StringRef& name,
    const ArrayRef<Member*>& stem,
    bool fromStatic,
    SmallSetBase<Member*>& result) {
  (void)_reporter;
  for (auto m : stem) {
    lookup(name, m, fromStatic, result);
  }
}

void MemberLookup::lookup(
    const StringRef& name,
    const ArrayRef<Type*>& stem,
    bool fromStatic,
    SmallSetBase<Member*>& result) {
  for (auto t : stem) {
    lookup(name, t, fromStatic, result);
  }
}

void MemberLookup::lookup(
    const collections::StringRef& name,
    Member* stem,
    bool fromStatic,
    SmallSetBase<Member*>& result) {

  std::vector<Member*> members;
  switch (stem->kind()) {
    case Member::Kind::PACKAGE:
      static_cast<Package*>(stem)->memberScope()->lookupName(name, members);
      break;
    case Member::Kind::MODULE:
      static_cast<Module*>(stem)->memberScope()->lookupName(name, members);
      break;
    case Member::Kind::TYPE: {
      auto td = static_cast<TypeDefn*>(stem);
      if (isa<Composite*>(td->type())) {
        td->inheritedMemberScope()->lookupName(name, members);
      } else {
        td->memberScope()->lookupName(name, members);
      }
      break;
    }
    case Member::Kind::TYPE_PARAM: {
      auto tp = static_cast<TypeParameter*>(stem);
      for (auto st : tp->subtypeConstraints()) {
        lookup(name, st, fromStatic, result);
      }
      break;
    }
    case Member::Kind::SPECIALIZED:
      assert(false && "implement.");
      break;
    case Member::Kind::LET:
    case Member::Kind::VAR:
    case Member::Kind::PARAM:
    case Member::Kind::ENUM_VAL:
    case Member::Kind::TUPLE_MEMBER:
    case Member::Kind::FUNCTION:
    case Member::Kind::PROPERTY:
      assert(false && "value defs do not have scopes");
      break;
  }

  for (auto m : members) {
    result.insert(m);
  }
}

void MemberLookup::forAllNames(const ArrayRef<Member*>& stem, scope::NameFunctor& nameFn) {
  for (auto m : stem) {
    forAllNames(m, nameFn);
  }
}

void MemberLookup::forAllNames(Member* stem, scope::NameFunctor& nameFn) {
  std::vector<Member*> members;
  switch (stem->kind()) {
    case Member::Kind::PACKAGE:
      static_cast<Package*>(stem)->memberScope()->forAllNames(nameFn);
      break;
    case Member::Kind::MODULE:
      static_cast<Module*>(stem)->memberScope()->forAllNames(nameFn);
      break;
    case Member::Kind::TYPE: {
      auto td = static_cast<TypeDefn*>(stem);
      if (!isa<Composite*>(td->type())) {
        td->inheritedMemberScope()->forAllNames(nameFn);
      } else {
        td->memberScope()->forAllNames(nameFn);
      }
      break;
    }
    case Member::Kind::TYPE_PARAM: {
      auto tp = static_cast<TypeParameter*>(stem);
      for (auto st : tp->subtypeConstraints()) {
        if (auto comp = dyn_cast<Composite*>(st)) {
          forAllNames(comp->defn(), nameFn);
        }
      }
      break;
    }
    case Member::Kind::SPECIALIZED:
      assert(false && "implement.");
      break;
    case Member::Kind::LET:
    case Member::Kind::VAR:
    case Member::Kind::PARAM:
    case Member::Kind::ENUM_VAL:
    case Member::Kind::TUPLE_MEMBER:
    case Member::Kind::FUNCTION:
    case Member::Kind::PROPERTY:
      break;
  }
}

void MemberLookup::lookup(
    const collections::StringRef& name,
    Type* stem,
    bool fromStatic,
    SmallSetBase<Member*>& result) {
  if (auto comp = dyn_cast<Composite*>(stem)) {
    lookup(name, comp->defn(), fromStatic, result);
  } else {
    assert(false && "Implement lookup in type");
  }
}

}}}
