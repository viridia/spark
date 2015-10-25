// ============================================================================
// scope/closematch.h: A standard scope.
// ============================================================================

#ifndef SPARK_SCOPE_CLOSEMATCH_H
#define SPARK_SCOPE_CLOSEMATCH_H 1

#ifndef SPARK_SCOPE_SCOPE_H
  #include "spark/scope/scope.h"
#endif

namespace spark {
namespace semgraph {
class Member;
}
namespace scope {
using collections::StringRef;
using semgraph::Member;

/** Class that looks for the closest name (in terms of edit distance) to a given target name. */
class CloseMatchFinder : public NameFunctor {
public:
  CloseMatchFinder(const StringRef& target) : _target(target), _distance(target.size() * 2 / 3) {}

  /** Called for each candidate name. */
  void operator()(const StringRef& name);

  /** Return the closes matching string. Returns an empty string if there were no matches. */
  StringRef closest() const { return _closest; }

private:
  std::size_t editDistance(const StringRef& s1, const StringRef& s2);

  StringRef _target;
  std::string _closest;
  std::size_t _distance;
};

}}

#endif
