#include "spark/sema/types/typestore.h"
#include "spark/semgraph/type.h"

namespace spark {
namespace sema {
namespace types {
using semgraph::Type;
using semgraph::TypeDefn;
using semgraph::TypeParameter;
using semgraph::ValueDefn;

Type* TypeStore::memberType(Member* m) {
  switch (m->kind()) {
    case Member::Kind::MODULE:
    case Member::Kind::PACKAGE:
      return &Type::ERROR;
    case Member::Kind::TYPE:
      assert(static_cast<TypeDefn*>(m)->type() != nullptr);
      return static_cast<TypeDefn*>(m)->type();
    case Member::Kind::FUNCTION:
      assert(false && "Unsupported kind: function.");
    case Member::Kind::PROPERTY:
      assert(false && "Unsupported kind: property.");
    case Member::Kind::LET:
    case Member::Kind::VAR:
    case Member::Kind::ENUM_VAL:
    case Member::Kind::PARAM:
    case Member::Kind::TUPLE_MEMBER:
      assert(static_cast<ValueDefn*>(m)->type() != nullptr);
      return static_cast<ValueDefn*>(m)->type();
    case Member::Kind::TYPE_PARAM:
      return static_cast<TypeParameter*>(m)->typeVar();
    case Member::Kind::SPECIALIZED:
      assert(false && "Unsupported kind: specialized.");
    default:
      assert(false && "Unsupported kind.");
  }
  assert(false);
}

Env TypeStore::createEnv(const semgraph::EnvMap& env) {
  // TODO: Definitely going to need unit tests for this.
  auto it = _envs.find(env);
  if (it != _envs.end()) {
    return Env(Env::Bindings(it->data(), it->size()));
  }

  // Make a copy of the data on the arena
  auto data = reinterpret_cast<Env::Bindings::element_type*>(_arena.allocate(
      env.size() * sizeof(Env::Bindings::element_type)));
  std::uninitialized_copy(env.begin(), env.end(), data);

  // Return a new environment object.
  return Env(Env::Bindings(data, env.size()));
}

UnionType* TypeStore::createUnionType(const TypeArray& members) {
  std::vector<Type*> sortedMembers(members.begin(), members.end());
  std::sort(sortedMembers.begin(), sortedMembers.end(), TypeOrdering());
  TypeKey key = TypeKey(members);
  auto it = _unionTypes.find(key);
  if (it != _unionTypes.end()) {
    return it->second;
  }
  auto ut = new (_arena) UnionType(_arena.copyOf(sortedMembers));
  _unionTypes[TypeKey(ut->members())] = ut;
  return ut;
}

TupleType* TypeStore::createTupleType(const TypeArray& members) {
  TypeKey key = TypeKey(members);
  auto it = _tupleTypes.find(key);
  if (it != _tupleTypes.end()) {
    return it->second;
  }
  auto tt = new (_arena) TupleType(_arena.copyOf(members));
  _tupleTypes[TypeKey(tt->members())] = tt;
  return tt;
}

ConstType* TypeStore::createConstType(Type* base, bool provisional) {
  auto key = std::pair<Type*, bool>(base, provisional);
  auto it = _constTypes.find(key);
  if (it != _constTypes.end()) {
    return it->second;
  }
  auto ct = new (_arena) ConstType(base, provisional);
  _constTypes[key] = ct;
  return ct;
}

FunctionType* TypeStore::createFunctionType(Type* returnType, const TypeArray& paramTypes) {
  std::vector<Type*> signature;
  signature.reserve(paramTypes.size() + 1);
  signature.push_back(returnType);
  signature.insert(signature.end(), paramTypes.begin(), paramTypes.end());
  TypeKey key = TypeKey(signature);
  auto it = _functionTypes.find(key);
  if (it != _functionTypes.end()) {
    return it->second;
  }
  auto ft = new (_arena) FunctionType(returnType, _arena.copyOf(paramTypes), false);
  _functionTypes[TypeKey(_arena.copyOf(signature))] = ft;
  return ft;
}

FunctionType* TypeStore::createFunctionType(Type* returnType, const ArrayRef<Parameter*>& params) {
  std::vector<Type*> paramTypes;
  paramTypes.reserve(params.size());
  for (auto param : params) {
    paramTypes.push_back(param->type());
  }
  return createFunctionType(returnType, paramTypes);
}

}}}
