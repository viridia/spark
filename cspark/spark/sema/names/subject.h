// ============================================================================
// sema/names/subject.h.
// ============================================================================

#ifndef SPARK_SEMA_NAMES_SUBJECT_H
#define SPARK_SEMA_NAMES_SUBJECT_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

namespace spark {
namespace semgraph {
class Defn;
class Member;
}
namespace sema {
namespace names {
using semgraph::Defn;
using semgraph::Member;

/** The 'subject' is the definition that contains the name being looked up. The subject
    determines whether a given symbol is visible or not. */
class Subject {
public:
  Subject() : _value(nullptr) {}

  /** The current subject. */
  Defn* get() const { return _value; }
  Defn* set(Defn* value) {
    Defn* prev = _value;
    _value = value;
    return prev;
  }

  bool isVisible(Member* target);
private:
  // Returns true if the subject is enclosed within the scope of container.
  bool containsSubject(Member* container);

  Defn* _value;
};

}}}

#endif
