// ============================================================================
// node.h: AST nodes
// ============================================================================

#ifndef SPARK_AST_BUILDER_H
#define SPARK_AST_BUILDER_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#ifndef SPARK_AST_NODE_H
  #include "spark/ast/node.h"
#endif

#ifndef SPARK_COLLECTIONS_ARRAYREF_H
  #include "spark/collections/arrayref.h"
#endif

#ifndef SPARK_SUPPORT_ARENA_H
  #include "spark/support/arena.h"
#endif

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

namespace spark {
namespace ast {
using spark::source::Location;
using spark::collections::ArrayRef;

/** Helper for building node lists in an arena. */
class NodeListBuilder {
public:
  NodeListBuilder(support::Arena& arena) : _arena(arena) {}
  NodeListBuilder& append(Node* n) {
    _nodes.push_back(n);
    return *this;
  }

  size_t size() const {
    return _nodes.size();
  }

  /** Return a location which spans all of the nodes. */
  Location location() const {
    Location loc;
    for (Node* n : _nodes) {
      loc |= n->location();
    }
    return loc;
  }

  Node* operator[](int index) const {
    return _nodes[index];
  }

  ArrayRef<const Node*> build() const {
    if (_nodes.empty()) {
      return ArrayRef<const Node*>();
    }
    Node** data = reinterpret_cast<Node**>(_arena.allocate(sizeof(Node*) * _nodes.size()));
    ArrayRef<const Node*> result(data, _nodes.size());
    std::copy(_nodes.begin(), _nodes.end(), data);
    return result;
  }
private:
  support::Arena& _arena;
  std::vector<Node*> _nodes;
};

}}

#endif
