#include "spark/scope/stdscope.h"
#include "spark/semgraph/defn.h"
#include "spark/semgraph/primitivetype.h"

namespace spark {
namespace semgraph {

/** Void. */
VoidType VoidType::VOID;

/** Boolean. */
BooleanType BooleanType::BOOL;

/** Character. */
IntegerType IntegerType::CHAR("char", 8, true, false);

/** Signed integers. */
IntegerType IntegerType::I8("i8", 8, false, false);
IntegerType IntegerType::I16("i16", 16, false, false);
IntegerType IntegerType::I32("i32", 32, false, false);
IntegerType IntegerType::I64("i64", 64, false, false);

/** Unsigned integers. */
IntegerType IntegerType::U8("u8", 8, true, false);
IntegerType IntegerType::U16("u16", 16, true, false);
IntegerType IntegerType::U32("u32", 32, true, false);
IntegerType IntegerType::U64("u64", 64, true, false);

/** Positive signed integers. */
IntegerType IntegerType::P8("literal i8", 8, false, true);
IntegerType IntegerType::P16("literal i16", 16, false, true);
IntegerType IntegerType::P32("literal i32", 32, false, true);
IntegerType IntegerType::P64("literal i64", 64, false, true);

/** Float types. */
FloatType FloatType::F32("f32", 32);
FloatType FloatType::F64("f64", 64);

/** Null pointer. */
NullPtrType NullPtrType::NULLPTR;

// scope::SymbolScope* _scope = nullptr;
//
// scope::SymbolScope* PrimitiveType::scope() {
//   if (_scope != nullptr) {
//     return _scope;
//   }
//
//   _scope = new scope::StandardScope(scope::SymbolScope::DEFAULT, "primitive types");
//   _scope->addMember(IntegerType::I8.defn());
// //   StandardScope(ScopeType st, const StringRef& description)
// //     : _scopeType(st)
// //     , _description(description.begin(), description.end())
// //     , _owner(NULL)
// //   {}
//   return _scope;
// }

}}
