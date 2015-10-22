// ============================================================================
// module.h: AST representing a module
// ============================================================================

#ifndef SPARK_AST_MODULE_H
#define SPARK_AST_MODULE_H 1

#ifndef SPARK_AST_NODE_H
  #include "spark/ast/node.h"
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

namespace spark {
namespace ast {
using spark::collections::StringRef;

/** AST node for a module. */
class Module : public Node {
public:
  Module(const Location& location)
    : Node(Kind::MODULE, location)
  {}

  /** List of members of this module. */
  NodeList members() const { return _members; }
  void setMembers(const NodeList& members) { _members = members; }

  /** List of imports. */
  NodeList imports() const { return _imports; }
  void setImports(const NodeList& imports) { _imports = imports; }

private:
  NodeList _members;
  NodeList _imports;
};

/** Node type representing an import. */
class Import : public Node {
public:

  /** Construct an Ident node. */
  Import(const Location& location, const Node* path, StringRef alias)
    : Node(Kind::IMPORT, location)
    , _path(path)
    , _alias(alias)
  {}

  /** The fully-qualified name of the symbol. */
  const Node* path() const { return _path; }

  /** Short aliased name for the import. */
  const StringRef alias() const { return _alias; }

private:
  const Node* _path;
  const StringRef _alias;
};

}}

#endif
