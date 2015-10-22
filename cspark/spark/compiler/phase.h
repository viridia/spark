// ============================================================================
// phase.h.
// ============================================================================

#ifndef SPARK_COMPILER_PHASE_H
#define SPARK_COMPILER_PHASE_H 1

#ifndef SPARK_COMPILER_COMPILER_H
  #include "spark/compiler/compiler.h"
#endif

namespace spark {
namespace compiler {
using spark::collections::StringRef;
using spark::error::Reporter;

class Context;

/** A phase is a sequence of one or more passes that are run together. */
class Phase {
public:
  Phase(Context* context, const StringRef& name, ModuleList& input)
    : _context(context)
    , _name(name)
    , _input(input)
    , _output(false)
    , _handlingException(false)
  {}

  ~Phase();

  /** Add a compilation pass to the list of passes. */
  void addPass(sema::Pass* pass) { _passes.push_back(pass); }

  /** The name of this phase. */
  const StringRef& name() const { return _name; }

  /** Set whether this is an output pass, i.e. should only run if an output location has been
      specified. */
  bool output() const { return _output; }
  void setOutput(bool output) { _output = output; }

  /** Run this phase. */
  void run();

  /** Error reporter. */
  error::Reporter& reporter();

private:
  Context* _context;
  StringRef _name;
  ModuleList& _input;
  std::vector<bool> _finished;
  std::vector<sema::Pass*> _passes;
  bool _output;
  bool _handlingException;
};

}}

#endif
