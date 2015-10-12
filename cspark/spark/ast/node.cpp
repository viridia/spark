// ============================================================================
// node.cpp
// ============================================================================

#include "spark/ast/node.h"

namespace spark {
namespace ast {

/** Singleton error node. */
Node Node::ERROR(Node::KIND_ERROR, source::Location());

/** Sentinel node to indicate lack of a node. */
Node Node::ABSENT(Node::KIND_ABSENT, source::Location());

}}

