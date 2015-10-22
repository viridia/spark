// ============================================================================
// scopecreation.h:
// ============================================================================

#ifndef SPARK_SEMA_PASSES_BUILDGRAPH_H
#define SPARK_SEMA_PASSES_BUILDGRAPH_H 1

#ifndef SPARK_SEMA_PASS_H
  #include "spark/sema/pass.h"
#endif

#ifndef SPARK_SEMA_PASSES_PASSID_H
  #include "spark/sema/passes/passid.h"
#endif

#ifndef SPARK_SEMGRAPH_DENF_H
  #include "spark/semgraph/defn.h"
#endif

namespace spark {
namespace sema {
namespace passes {

/** Pass that builds the semantic graph. */
class BuildGraphPass : public Pass {
public:
  BuildGraphPass(compiler::Context* context) : Pass(context), _arena(NULL) {}

  PassId id() const { return BUILD_GRAPH; }
  void run(semgraph::Module* mod);

private:
  support::Arena* _arena;

  void createMembers(
      const ast::NodeList& memberAsts,
      semgraph::Member* parent,
      semgraph::MemberList& memberList,
      scope::StandardScope* memberScope);

  semgraph::Defn* createDefn(const ast::Node * node, semgraph::Member* parent);

  void createParamList(
      const ast::NodeList& memberAsts,
      semgraph::Member* parent,
      std::vector<semgraph::Parameter*>& paramList,
      scope::StandardScope* paramScope);

  void createTypeParamList(
      const ast::NodeList& paramAsts,
      semgraph::Member* parent,
      std::vector<semgraph::TypeParameter*>& paramList,
      scope::StandardScope* paramScope);

  support::Arena& arena();
  semgraph::Visibility astVisibility(const ast::Defn* d);
};

}}}

#endif
