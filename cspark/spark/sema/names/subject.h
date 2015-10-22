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
}
namespace sema {
namespace names {
using semgraph::Defn;

/** The 'subject' is the definition that contains the name being looked up. The subject
    determines whether a given symbol is visible or not. */
class Subject {
public:
  Subject() : _value(NULL) {}

  /** The current subject. */
  Defn* get() const { return _value; }
  Defn* set(Defn* value) {
    Defn* prev = _value;
    _value = value;
    return prev;
  }
private:
  Defn* _value;
};

}}}

#endif
