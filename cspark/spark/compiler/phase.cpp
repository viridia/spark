#include "spark/compiler/context.h"
#include "spark/compiler/phase.h"
#include "spark/sema/pass.h"

namespace spark {
namespace compiler {

error::Reporter& Phase::reporter() {
  return _context->reporter();
}

Phase::~Phase() {
  for (sema::Pass* pass : _passes) {
    delete pass;
  }
}

void Phase::run() {
  reporter().status() << "Running phase: " << _name;
  size_t inputSize = _input.size();

  // Make sure there's a finished bit for every input module.
  _finished.resize(_input.size());

  // See if there are any modules that still need this phase.
  bool unfinished = false;
  for (int i = 0; i < inputSize; ++i) {
    if (!_finished[i]) {
      unfinished = true;
      break;
    }
  }

  // TODO: Handle exceptions here?
  if (unfinished) {
    reporter().status() << "Unfinished modules: " << _name;
    for (sema::Pass* pass : _passes) {
      // Note that we don't use iterators here because they may be invalidated if additional
      // modules get added to the list during processing.
      for (int i = 0; i < inputSize; ++i) {
        if (!_finished[i]) {
          semgraph::Module* module = _input[i];
          pass->run(module);
        }
      }
    }
  }

  // Mark modules as finished.
  for (int i = 0; i < inputSize; ++i) {
    if (!_finished[i]) {
      _finished[i] = true;
    }
  }

  (void)_handlingException;
}

//   StringRef _name;
//   ModuleSet _finished;
//   std::vector<sema::Pass*> _passes;

//   def __init__(self, compiler, name, moduleSets, passes, output = False):
//     super().__init__(compiler)
//     self.name = name
//     self.moduleSets = moduleSets
//     self.passes = passes
//     self.modulesFinished = set()
//     self.output = output
//     self.handlingException = False
//
//   def run(self):
//     if self.output and not self.compiler.outputDir:
//       return
//     if self.errorReporter.getErrorCount() > 0:
//       return
//
//     # Compute the set of modules we need to operate on.
//     modules = OrderedDict()
//     for ms in self.moduleSets:
//       for mod in ms:
//         if mod not in self.modulesFinished:
//           modules[mod] = None
//     if not modules:
//       return
//
// #     self.infoFmt('pass group: {0}', self.name)
// #     self.infoFmt('  modules: {0}', ', '.join(m.getName() for m in modules))
//
//     for passToRun in self.passes:
//       if self.errorReporter.getErrorCount() > 0:
//         return
// #       self.infoFmt("  pass: {0}", passToRun.__class__.__name__)
//       for mod in modules:
//         for prereq in passToRun.PREREQS:
//           if prereq not in self.compiler.passesRun[mod]:
//             self.errorReporter.error(
//                 'Cannot run compiler pass {0} because it requires pass {1} to run first.'.format(
//                     passToRun.__class__.__name__, prereq))
//             return
//         try:
//           passToRun.run(mod)
//         except Exception:
//           if not self.handlingException:
//             self.handlingException = True
//             self.errorReporter.error("Exception encountered while compiling module: " + mod.getName())
//           raise
//         self.modulesFinished.add(mod)
//         self.compiler.passesRun[mod].add(passToRun.__class__.__name__)
//       # Doesn't mean we're finished forever, only for this cycle.
//       passToRun.finish()

}}
