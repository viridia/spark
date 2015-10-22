// ============================================================================
// cspark.cpp: Main entry point.
// ============================================================================

#include "spark/collections/stringref.h"
#include "spark/compiler/compiler.h"
#include <iostream>
#include <vector>

using spark::collections::StringRef;
using spark::compiler::Compiler;

void usage() {
  std::cerr << "cspark [options...] sources...\n";
  std::cerr << "  --version              Print version number and exit.\n";
  std::cerr << "  --out, -o DIR          Specify root output directory.\n";
  std::cerr << "  --modulepath, -m PATH  Add path to module search path.\n";
  std::cerr << "  --sourceroot, -s PATH  Root directory for input sources.\n";
  exit(-1);
}

class App {
public:
  App(int argc, char **argv)
    : _argCount(argc)
    , _args(argv)
    , _compiler(_reporter)
  {}

  void parseArgs() {
    for (int i = 1; i < _argCount; ++i) {
      StringRef arg(_args[i]);
      if (arg.empty()) {
        std::cerr << "Invalid argument: ''\n";
      } else if (arg.startsWith("--")) {
        StringRef opt = arg.substr(2);
        if (opt == "version") {
          std::cerr << "cspark version 0.1.\n";
          exit(0);
        } else if (opt == "out") {
          setOutputDir(nextArg(i));
        } else if (opt == "modulepath") {
          _compiler.addModulePath(nextArg(i));
        } else if (opt == "sourceroot") {
          setSourceRoot(nextArg(i));
        } else {
          std::cerr << "Unknown option: " << arg << "\n";
          usage();
        }
      } else if (arg.startsWith("-")) {
        StringRef opt = arg.substr(1);
        if (opt == "v") {
          std::cerr << "cspark version 0.1.\n";
          exit(0);
        } else if (opt == "m") {
          _compiler.addModulePath(nextArg(i));
        } else if (opt == "s") {
          setSourceRoot(nextArg(i));
        } else if (opt == "o") {
          setOutputDir(nextArg(i));
        } else {
          std::cerr << "Unknown option: " << arg << "\n";
          usage();
        }
      } else {
        _compiler.addSource(arg);
      }
    }
  }

  int run() {
    if (_compiler.sources().empty()) {
      std::cerr << "No input sources specified.\n";
      usage();
    }

    _compiler.compile();

    if (_reporter.errorCount() > 0) {
      std::cerr << _reporter.errorCount() << " total errors.\n";
      return 2;
    }
    return 0;
  }
private:
  void setOutputDir(StringRef dir) {
    if (dir.empty()) {
      std::cerr << "Invalid output directory ''.\n";
      usage();
    }
    if (!_compiler.outputDir().empty()) {
      std::cerr << "Output directory already specified.\n";
      usage();
    }
    _compiler.setOutputDir(dir);
  }

  void setSourceRoot(StringRef dir) {
    if (dir.empty()) {
      std::cerr << "Invalid output directory ''.\n";
      usage();
    }
    if (!_compiler.sourceRoot().empty()) {
      std::cerr << "Source root directory already specified.\n";
      usage();
    }
    _compiler.setSourceRoot(dir);
  }

  StringRef nextArg(int& i) {
    ++i;
    if (i < _argCount) {
      return _args[i];
    } else {
      std::cerr << "Option value expected\n";
      usage();
      return StringRef();
    }
  }

  int _argCount;
  char** _args;
  spark::error::ConsoleReporter _reporter;
  spark::compiler::Compiler _compiler;
};

int main(int argc, char **argv) {
  App app(argc, argv);
  app.parseArgs();
  return app.run();
}
