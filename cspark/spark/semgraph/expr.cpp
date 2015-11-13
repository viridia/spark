#include "spark/semgraph/expr.h"

namespace spark {
namespace semgraph {

Expr Expr::ERROR(Expr::Kind::INVALID);
Expr Expr::IGNORED(Expr::Kind::IGNORED);

}}
