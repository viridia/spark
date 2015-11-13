#include "spark/error/reporter.h"
#include "spark/sema/names/fillmemberset.h"
#include "spark/sema/names/subject.h"
#include "spark/semgraph/defn.h"
#include "spark/semgraph/expr.h"
#include "spark/support/arraybuilder.h"
#include "spark/support/casting.h"

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace sema {
namespace names {
using namespace semgraph;
using support::dyn_cast;

bool FillMemberSet::fill(MemberSet* mset, ArrayRef<Member*> members) {
  assert(members.size() > 0);
  assert(!mset->name().empty());

  std::vector<Member*> visible;
  std::vector<Member*> hidden;

  for (auto m : members) {
    if (_subject.isVisible(m)) {
      visible.push_back(m);
    } else {
      hidden.push_back(m);
    }
  }

  const ArrayRef<Member*>& all = visible.empty() ? members : visible;
  MemberSet::Genus genus = genusOf(all);

  // Report an error if no symbols are visible.
  if (!hidden.empty()) {
    switch (genus) {
      case MemberSet::Genus::VARIABLE:
        _reporter.error(mset->location()) << "Variable '" << mset->name() << "' is not visible.";
        break;
      case MemberSet::Genus::FUNCTION:
        _reporter.error(mset->location()) << "Method '" << mset->name() << "' is not visible.";
        break;
      case MemberSet::Genus::TYPE:
        _reporter.error(mset->location()) << "Type '" << mset->name() << "' is not visible.";
        break;
      default:
        _reporter.error(mset->location()) << "Member '" << mset->name() << "' is not visible.";
        break;
    }

    for (auto m : members) {
      while (m->kind() == Member::Kind::SPECIALIZED) {
        m = static_cast<SpecializedMember*>(m)->generic();
      }
      if (m->kind() == Member::Kind::PACKAGE || m->kind() == Member::Kind::MODULE) {
        _reporter.info() << m->qualifiedName();
      } else {
        _reporter.info(static_cast<Defn*>(m)->location()) << "Defined here.";
      }
    }
    return false;
  } else if (genus == MemberSet::Genus::INCONSISTENT) {

//     if listType == graph.MemberListType.VARIABLE and len(members) > 1:
//       # Try removing properties and see if that helps
//       nonProperties = []
//       for m in members:
//         if m.typeId() != graph.MemberKind.PROPERTY:
//           nonProperties.append(m)
//       if len(nonProperties) != 1:
//         ambiguous = True
//       else:
//         members = nonProperties
//         assert len(members) > 0
    _reporter.error(mset->location()) << "Ambiguous reference to '" << mset->name() <<
        "', could be:";
    for (auto m : members) {
      while (m->kind() == Member::Kind::SPECIALIZED) {
        m = static_cast<SpecializedMember*>(m)->generic();
      }
      _reporter.info() << m->qualifiedName();
    }
    return false;
  } else if (genus == MemberSet::Genus::VARIABLE) {
    if (auto vdef = dyn_cast<ValueDefn*>(all[0])) {
      if (!vdef->isDefined()) {
        _reporter.error(mset->location()) << "Reference to variable '" <<
            mset->name() << "' before it has been assigned a value.";
      }
    }
  }

  support::ArrayBuilder<Member*> builder(_arena, visible);
  mset->setGenus(genus);
  mset->setMembers(builder.build());
  return true;
}

MemberSet::Genus FillMemberSet::genusOf(ArrayRef<Member*> members) {
  MemberSet::Genus genus = MemberSet::Genus::INCOMPLETE;
  assert(members.size() > 0);

  // Make sure members are similar kinds
  for (auto m : members) {
    while (m->kind() == Member::Kind::SPECIALIZED) {
      m = static_cast<SpecializedMember*>(m)->generic();
    }
    MemberSet::Genus memberGenus;
    switch (m->kind()) {
      case Member::Kind::PACKAGE:
      case Member::Kind::MODULE:
        memberGenus = MemberSet::Genus::NAMESPACE;
        break;
      case Member::Kind::TYPE:
        memberGenus = MemberSet::Genus::TYPE;
        break;
      case Member::Kind::TYPE_PARAM: {
        TypeParameter* tp = static_cast<TypeParameter*>(m);
        memberGenus = tp->valueType() != nullptr ?
          MemberSet::Genus::VARIABLE : MemberSet::Genus::TYPE;
        break;
      }
      case Member::Kind::FUNCTION:
        memberGenus = MemberSet::Genus::FUNCTION;
        break;
      case Member::Kind::PROPERTY: {
        Property* p = static_cast<Property*>(m);
        memberGenus = p->params().empty() ? MemberSet::Genus::VARIABLE : MemberSet::Genus::FUNCTION;
        break;
      }
      case Member::Kind::VAR:
      case Member::Kind::LET:
      case Member::Kind::PARAM:
      case Member::Kind::ENUM_VAL:
      case Member::Kind::TUPLE_MEMBER:
        memberGenus = MemberSet::Genus::VARIABLE;
        break;
      case Member::Kind::SPECIALIZED:
        assert(false);
    }

    if (genus == MemberSet::Genus::INCOMPLETE) {
      genus = memberGenus;
    } else if (genus != memberGenus) {
      genus = MemberSet::Genus::INCONSISTENT;
    }
  }

  return genus;
}

}}}
