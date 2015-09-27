// ============================================================================
// node.h: AST nodes
// ============================================================================

#ifndef SPARK_AST_NODE_H
#define SPARK_AST_NODE_H 1

#ifndef SPARK_SOURCE_LOCATION_H
#include <spark/source/location.h>
#endif

namespace spark {
namespace ast {
  
class Node {
public:
  enum Kind {
    #define NODE_KIND(x) KIND_##x,
    #include "nodetype.txt"
    #undef NODE_KIND
  };
  
  /** Construct an AST node. */
  Node(Kind kind, Location& location) : _kind(kind), _location(location) {}

  /** What kind of node this is. */
  Kind kind() const { return _kind; }
  
  /** The source location where this node was parsed. */
  const source::Location& location() const { return _location; }
  
private:
  const Kind _kind;
  const source::Location _location;
};

}}

#endif
