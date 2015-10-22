// ============================================================================
// node.cpp
// ============================================================================

#include "spark/ast/node.h"

namespace spark {
namespace ast {

/** Singleton error node. */
Node Node::ERROR(Kind::ERROR, source::Location());

/** Sentinel node to indicate lack of a node. */
Node Node::ABSENT(Kind::ABSENT, source::Location());

#define NODE_KIND(x) #x,

const char* KIND_NAMES[] = {
  #include "spark/ast/nodetype.txt"
};

const char* Node::KindName(Kind kind) {
  if (kind < Kind::LAST) {
    return KIND_NAMES[(uint32_t)kind];
  }

  return "<Invalid AST Kind>";
}

}}

