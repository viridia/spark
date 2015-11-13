// ============================================================================
// sema/types/ordering.h: Class for creating and managing derived types.
// ============================================================================

#ifndef SPARK_SEMA_TYPES_ORDERING_H
#define SPARK_SEMA_TYPES_ORDERING_H 1

#ifndef SPARK_SEMGRAPH_EXPR_H
  #include "spark/semgraph/expr.h"
#endif

#ifndef SPARK_SEMGRAPH_DEFN_H
  #include "spark/semgraph/defn.h"
#endif

#ifndef SPARK_SEMGRAPH_TYPE_H
  #include "spark/semgraph/type.h"
#endif

#ifndef SPARK_SEMGRAPH_PRIMITIVETYPE_H
  #include "spark/semgraph/primitivetype.h"
#endif

#ifndef SPARK_SUPPORT_ARENA_H
  #include "spark/support/arena.h"
#endif

#if SPARK_HAVE_ALGORITHM
  #include <algorithm>
#endif

namespace spark {
namespace sema {
namespace types {
using semgraph::Env;
using semgraph::Type;
using semgraph::IntegerType;
using semgraph::FloatType;
using semgraph::Composite;
using semgraph::FunctionType;
using semgraph::UnionType;
using semgraph::TupleType;
using semgraph::SpecializedType;
using semgraph::TypeArray;
using semgraph::Member;
using collections::ArrayRef;

/** Defines a total ordering for all Spark type expressions. */
struct TypeOrdering {
  /** Compare two types. */
  bool operator()(const Type* t0, const Type* t1) {
    return compare(t0, t1) < 0;
  }

  /** Compare two type arrays. */
  bool operator()(const TypeArray& a0, const TypeArray& a1) {
    return compare(a0, a1) < 0;
  }

  /** Compare two type bindings. */
  static int compare(const Env::Binding& b0, const Env::Binding& b1) {
    int c = compare(b0.first, b1.second);
    if (c != 0) {
      return c;
    }
    return compare(b0.first, b1.second);
  }

  /** Compare two arrays of type bindings. */
  bool operator()(const ArrayRef<Env::Binding>& a0, const ArrayRef<Env::Binding>& a1) {
    return compare(a0, a1) < 0;
  }

  static int compare(const Type* t0, const Type* t1) {
    if (t0 == t1) {
      return 0;
    }

    if (t0->kind() != t1->kind()) {
      return t0->kind() < t1->kind() ? -1 : 1;
    }

    switch (t0->kind()) {
      case Type::Kind::INVALID:
      case Type::Kind::IGNORED:
      case Type::Kind::NO_RETURN:
      case Type::Kind::VOID:
      case Type::Kind::NULLPTR:
      case Type::Kind::BOOLEAN:
        // These types are all singletons.
        return 0;

      case Type::Kind::INTEGER: {
        auto i0 = static_cast<const IntegerType*>(t0);
        auto i1 = static_cast<const IntegerType*>(t1);
        if (i0->bits() != i1->bits()) {
          return i0->bits() < i1->bits() ? -1 : 1;
        }
        if (i0->isUnsigned() != i1->isUnsigned()) {
          return i1->isUnsigned() ? -1 : 1;
        }
        if (i0->isPositive() != i1->isPositive()) {
          return i1->isPositive() ? -1 : 1;
        }
        // Integer types should never be the same value unless they are the same object.
        assert(false && "Non-referential integer type equality.");
      }

      case Type::Kind::FLOAT: {
        auto f0 = static_cast<const FloatType*>(t0);
        auto f1 = static_cast<const FloatType*>(t1);
        if (f0->bits() != f1->bits()) {
          return f0->bits() < f1->bits() ? -1 : 1;
        }
        // Float types should never be the same value unless they are the same object.
        assert(false && "Non-referential float type equality.");
      }
      case Type::Kind::CLASS:
      case Type::Kind::STRUCT:
      case Type::Kind::INTERFACE:
      case Type::Kind::EXTENSION:
      case Type::Kind::ENUM: {
        auto c0 = static_cast<const Composite*>(t0);
        auto c1 = static_cast<const Composite*>(t1);
        auto d0 = c0->defn();
        auto d1 = c1->defn();
        // See if names are different
        int c = d0->name().compare(d1->name());
        if (c != 0) {
          return c;
        }
        // See if parents are different
        c = compare(d0->definedIn(), d1->definedIn());
        if (c != 0) {
          return c;
        }
        // Only thing left is to compare overload index.
        assert(false && "check overloading index.");
      }

      case Type::Kind::UNION: {
        auto u0 = static_cast<const UnionType*>(t0);
        auto u1 = static_cast<const UnionType*>(t1);
        return compare(u0->members(), u1->members());
      }

      case Type::Kind::TUPLE: {
        auto d0 = static_cast<const TupleType*>(t0);
        auto d1 = static_cast<const TupleType*>(t1);
        return compare(d0->members(), d1->members());
      }

      case Type::Kind::FUNCTION: {
        auto f0 = static_cast<const FunctionType*>(t0);
        auto f1 = static_cast<const FunctionType*>(t1);
        int c = compare(f0->returnType(), f1->returnType());
        if (c != 0) {
          return c;
        }
        c = compare(f0->paramTypes(), f1->paramTypes());
        if (c != 0) {
          return c;
        }
        assert(false && "check overloading index.");
      }

      case Type::Kind::SPECIALIZED: {
        auto s0 = static_cast<const SpecializedType*>(t0);
        auto s1 = static_cast<const SpecializedType*>(t1);
        int c = compare(s0->generic(), s1->generic());
        if (c != 0) {
          return c;
        }
        auto i0 = s0->env().begin(), i0End = s0->env().end();
        auto i1 = s1->env().begin(), i1End = s1->env().end();
        for (; i0 != i0End && i1 != i1End; ++i0, ++i1) {
          c = compare(i0->first, i1->first);
          if (c != 0) {
            return c;
          }
          c = compare(i0->second, i1->second);
          if (c != 0) {
            return c;
          }
        }
        if (i0 != i0End) {
          return 1;
        } else if (i1 != i1End) {
          return -1;
        } else {
          // Q: Should we enforce referential equality here?
          return 0;
        }
      }

      case Type::Kind::TYPE_VAR:
      case Type::Kind::ALIAS:
      case Type::Kind::CONST:
      case Type::Kind::VALUE_PARAM:
      case Type::Kind::TYPESET:
        assert(false && "Implement type comparison.");

//   if isinstance(lt, graph.TypeAlias):
//     return compareLexical(lt.getValue(), rt)
//
//   if isinstance(rt, graph.TypeAlias):
//     return compareLexical(lt, rt.getValue())

//   if isinstance(lt, graph.TypeVar):
//     return -1
//     assert False
// #     return compareLexical(lEnv[lt], lConst, lEnv, rt, rConst, rEnv)
//
//   if isinstance(rt, graph.TypeVar):
//     return 1
//     assert False
// #     return compareLexicalExt(lt, lConst, lEnv, rEnv[rt], rConst, rEnv)

//   if isinstance(lt, graph.ModifiedType):
//     if isinstance(rt, graph.ModifiedType):
//       result = compareLexical(lt.getBase(), rt.getBase())
//       if result != 0:
//         return result
//       if isConst(lt) != isConst(rt):
//         return 1 if isConst(lt) else -1
//       if isReference(lt) != isReference(rt):
//         return 1 if lt.isRef() else -1
//       if isVariadic(lt) != isVariadic(rt):
//         return 1 if lt.isVariadic() else -1
//       return 0
//     else:
//       result = compareLexical(lt.getBase(), rt)
//       return result or -1

//   if isinstance(rt, graph.ModifiedType):
//     result = compareLexical(lt, rt.getBase())
//     return result or 1
//
//   if lt.typeId() < rt.typeId():
//     return -1
//   elif lt.typeId() > rt.typeId():
//     return 1
    }
  }

  static int compare(const TypeArray& a0, const TypeArray& a1) {
    size_t size = std::min(a0.size(), a1.size());
    for (int i = 0; i < size; ++i) {
      int c = compare(a0[i], a1[i]);
      if (c != 0) {
        return c;
      }
    }
    if (a0.size() < a1.size()) {
      return -1;
    } else if (a1.size() < a0.size()) {
      return 1;
    } else {
      return 0;
    }
  }

  static int compare(const ArrayRef<Env::Binding>& a0, const ArrayRef<Env::Binding>& a1) {
    size_t size = std::min(a0.size(), a1.size());
    for (int i = 0; i < size; ++i) {
      int c = compare(a0[i], a1[i]);
      if (c != 0) {
        return c;
      }
    }
    if (a0.size() < a1.size()) {
      return -1;
    } else if (a1.size() < a0.size()) {
      return 1;
    } else {
      return 0;
    }
  }

  static int compare(const Member* m0, const Member* m1) {
    if (m0 == m1) {
      return 0;
    }
    int c = m0->name().compare(m1->name());
    if (c != 0) {
      return c;
    }
    if (m0->definedIn() == NULL) {
      return m1->definedIn() == NULL ? 0 : -1;
    } else if (m1->definedIn() == NULL) {
      return 1;
    }
    return compare(m0->definedIn(), m1->definedIn());
  }
};

}}}

#endif
