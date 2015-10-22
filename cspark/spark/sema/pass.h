// ============================================================================
// sema/semapass.h: Base class for semantic passes.
// ============================================================================

#ifndef SPARK_SEMA_PASS_H
#define SPARK_SEMA_PASS_H 1

#ifndef SPARK_COMPILER_CONTEXT_H
  #include "spark/compiler/context.h"
#endif

#ifndef SPARK_SEMA_PASSES_PASSID_H
  #include "spark/sema/passes/passid.h"
#endif

namespace spark {
namespace semgraph {
class Module;
}
namespace sema {
using spark::error::Reporter;

/** Base class for semantic passes. */
class Pass {
public:
  Pass(compiler::Context* context) : _context(context) {}
  virtual ~Pass() {}

  /** Return a unique identifier for this pass. This is used to keep track of which passes
      have processed a given module. */
  virtual passes::PassId id() const = 0;

  /** Method called to indicate that we want to run this pass on this module. */
  virtual void run(semgraph::Module* mod) = 0;

  /** Called when we are done processing a batch of modules. */
  virtual void finish() {}

  Reporter& reporter() const { return _context->reporter(); }

protected:
  compiler::Context const* _context;
};

}}

#endif
