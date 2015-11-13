// ============================================================================
// sema/names/memberlookup.h.
// ============================================================================

#ifndef SPARK_SEMA_NAMES_MEMBERLOOKUP_H
#define SPARK_SEMA_NAMES_MEMBERLOOKUP_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_SEMA_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

#ifndef SPARK_SEMA_COLLECTIONS_SMALLSET_H
  #include "spark/collections/smallset.h"
#endif

#ifndef SPARK_SEMA_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

#ifndef SPARK_SEMA_TYPES_APPLYENV_H
  #include "spark/sema/types/applyenv.h"
#endif

#ifndef SPARK_SEMA_TYPES_TYPESTORE_H
  #include "spark/sema/types/typestore.h"
#endif

namespace spark {
namespace error { class Reporter; }
namespace semgraph { class Member; class Type; }
namespace support { class Arena; }
namespace sema {
namespace names {
using collections::ArrayRef;
using collections::SmallSetBase;
using collections::StringRef;
using error::Reporter;
using semgraph::Member;
using semgraph::Type;

/** Name resolver specialized for resolving types. */
class MemberLookup {
public:
  MemberLookup(Reporter& reporter, types::TypeStore* typeStore)
    : _reporter(reporter)
    , _arena(typeStore->arena())
    , _apply(typeStore)
  {}

  /** Given a list of members to look in, find members with the specified name. */
  void lookup(
      const StringRef& name,
      const ArrayRef<Member*>& stem,
      bool fromStatic,
      SmallSetBase<Member*>& result);

  /** Given a list of types to look in, find members with the specified name. */
  void lookup(
      const StringRef& name,
      const ArrayRef<Type*>& stem,
      bool fromStatic,
      SmallSetBase<Member*>& result);

  /** Given a member to look in, find members with the specified name. */
  void lookup(
      const StringRef& name,
      Member* stem,
      bool fromStatic,
      SmallSetBase<Member*>& result);

  /** Given a type to look in, find members with the specified name. */
  void lookup(
      const collections::StringRef& name,
      Type* stem,
      bool fromStatic,
      SmallSetBase<Member*>& result);

  /** Iterate through all names. */
  void forAllNames(const ArrayRef<Member*>& stem, scope::NameFunctor& nameFn);
  void forAllNames(Member* stem, scope::NameFunctor& nameFn);

private:
  Reporter& _reporter;
  support::Arena& _arena;
  types::ApplyEnv _apply;
};

}}}

#endif
