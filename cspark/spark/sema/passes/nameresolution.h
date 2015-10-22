// ============================================================================
// scopecreation.h:
// ============================================================================

#ifndef SPARK_SEMA_PASSES_NAMERESOLUTION_H
#define SPARK_SEMA_PASSES_NAMERESOLUTION_H 1

#ifndef SPARK_SEMA_PASS_H
  #include "spark/sema/pass.h"
#endif

#ifndef SPARK_SEMA_PASSES_PASSID_H
  #include "spark/sema/passes/passid.h"
#endif

#ifndef SPARK_SEMA_NAMES_SUBJECT_H
  #include "spark/sema/names/subject.h"
#endif

#ifndef SPARK_SEMGRAPH_DEFN_H
  #include "spark/semgraph/defn.h"
#endif

#ifndef SPARK_SEMGRAPH_DEFNVISITOR_H
  #include "spark/semgraph/defnvisitor.h"
#endif

namespace spark {
namespace scope {
class ScopeStack;
}
namespace sema {
namespace names {
class ResolveTypes;
}
namespace passes {

/** Pass that resolves all symbol names. */
class NameResolutionPass : public Pass, semgraph::DefnVisitor<void> {
public:
  NameResolutionPass(compiler::Context* context);

  PassId id() const { return NAME_RESOLUTION; }
  void run(semgraph::Module* mod);

  void visitValueDefn(semgraph::ValueDefn* v);
  void visitTypeDefn(semgraph::TypeDefn* t);
  void visitTypeParameter(semgraph::TypeParameter* t);
  void visitParameter(semgraph::Parameter* p);
  void visitFunction(semgraph::Function* f);
  void visitProperty(semgraph::Property* p);

private:
  void resolveImports(semgraph::Module* mod);
  void findAbsoluteSymbol(const ast::Node* node, std::vector<semgraph::Member*> &result);

  support::Arena* _arena;
  sema::names::Subject _subject;
  std::auto_ptr<scope::ScopeStack> _scopeStack;
  std::auto_ptr<names::ResolveTypes> _resolveTypes;

  support::Arena& arena();
};

}}}

#endif
