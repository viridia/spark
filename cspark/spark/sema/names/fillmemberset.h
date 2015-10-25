// ============================================================================
// sema/names/fillmemberset.h.
// ============================================================================

#ifndef SPARK_SEMA_NAMES_FILLMEMBERSET_H
#define SPARK_SEMA_NAMES_FILLMEMBERSET_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#if SPARK_HAVE_VECTOR
  #include <vector>
#endif

namespace spark {
namespace error { class Reporter; }
namespace semgraph { class Member; class MemberSet; }
namespace support { class Arena; }
namespace sema {
namespace names {
using error::Reporter;
using semgraph::MemberSet;
using semgraph::Member;
class Subject;

/** Name resolver specialized for resolving types. */
class FillMemberSet {
public:
  FillMemberSet(Reporter& reporter, Subject& subject, support::Arena& arena)
    : _reporter(reporter)
    , _subject(subject)
    , _arena(arena)
  {}

  /** Populate this member set with members, and infer the genus of the set. Also reporter
      an error if the members are ambiguous or not visible. */
  bool fill(MemberSet* mset, std::vector<Member*> members);

private:
  Reporter& _reporter;
  Subject& _subject;
  support::Arena& _arena;
};

}}}

#endif
