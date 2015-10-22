// ============================================================================
// scope/namelookuputil.h.
// ============================================================================

#ifndef SPARK_SCOPE_NAMELOOKUPUTIL_H
#define SPARK_SCOPE_NAMELOOKUPUTIL_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

namespace spark {
namespace error {
class Reporter;
}
namespace semgraph {
class Defn;
}
namespace scope {
using collections::StringRef;
using error::Reporter;
using semgraph::Member;

class NameLookupContext {
public:
  NameLookupContext(Reporter& reporter) : _reporter(reporter), _subject(NULL) {}

  /** The 'subject' is the definition that contains the name being looked up. The subject
      determines whether a given symbol is visible or not. */
  Defn* subject() const { return _subject; }
  Defn* setSubject(Defn* subject) {
    Defn* prevSubject = _subject;
    _subject = subject;
    return prevSubject;
  }
private:
  Reporter& _reporter;
  Defn* _subject;
};

}}

#endif
