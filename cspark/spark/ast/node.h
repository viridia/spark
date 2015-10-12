// ============================================================================
// node.h: AST nodes
// ============================================================================

#ifndef SPARK_AST_NODE_H
#define SPARK_AST_NODE_H 1

#ifndef SPARK_SOURCE_LOCATION_H
#include <spark/source/location.h>
#endif

#ifndef SPARK_COLLECTIONS_ARRAYREF_H
#include <spark/collections/arrayref.h>
#endif

namespace spark {
namespace support {
class Arena;
}
namespace ast {
using spark::source::Location;

class Node {
public:
  enum Kind {
    #define NODE_KIND(x) KIND_##x,
    #include "nodetype.txt"
    #undef NODE_KIND
  };

  /** Construct an AST node. */
  Node(Kind kind, const Location& location) : _kind(kind), _location(location) {}

  /** What kind of node this is. */
  Kind kind() const { return _kind; }

  /** The source location where this node was parsed. */
  const source::Location& location() const { return _location; }

  static inline bool isError(const Node* node) {
    return node == NULL || node->kind() == Node::KIND_ERROR;
  }

  static inline bool isPresent(const Node* node) {
    return node != NULL && node->kind() != Node::KIND_ABSENT;
  }

  static Node ERROR; // Sentinel node that indicate errors.
  static Node ABSENT; // Sentinel node that indicate lack of a node.

private:
  const Kind _kind;
  const source::Location _location;
};

/** Typedef for a list of nodes. */
typedef collections::ArrayRef<const Node*> NodeList;

}}

#endif
