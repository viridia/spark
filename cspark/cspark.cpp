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
  std::cerr << "cspark [options...] source...\n";
  std::cerr << "  --version              Print version number and exit.\n";
  std::cerr << "  --out, -o DIR          Specify root output directory.\n";
  std::cerr << "  --modulepath, -mp PATH Add path to module search path.\n";
  exit(-1);
}

StringRef nextArg(int& i, int argc, char**argv) {
  ++i;
  if (i < argc) {
    return argv[i];
  } else {
    std::cerr << "Option value expected\n";
    usage();
    return StringRef();
  }
}

int main(int argc, char **argv) {
  spark::error::ConsoleReporter reporter;
  spark::compiler::Compiler compiler(reporter);
  for (int i = 1; i < argc; ++i) {
    StringRef arg(argv[i]);
    if (arg.empty()) {
    } else if (arg.startsWith("--")) {
      StringRef opt = arg.substr(2);
      if (opt == "version") {
        std::cerr << "cspark version 0.1.\n";
        return 0;
      } else if (opt == "out") {
        if (!compiler.outputDir().empty()) {
          std::cerr << "Output directory already specified.\n";
          usage();
        }
        compiler.setOutputDir(nextArg(i, argc, argv));
        if (compiler.outputDir().empty()) {
          std::cerr << "Invalid output directory ''.\n";
          usage();
        }
      } else if (opt == "mp" || opt == "modulePath") {
        compiler.addModulePath(nextArg(i, argc, argv));
      } else {
        std::cerr << "Unknown option: " << arg << "\n";
        usage();
      }
    } else if (arg.startsWith("-")) {
      StringRef opt = arg.substr(1);
      if (opt == "v") {
      } else if (opt == "o") {
        if (!compiler.outputDir().empty()) {
          std::cerr << "Output directory already specified.\n";
          usage();
        }
        compiler.setOutputDir(nextArg(i, argc, argv));
        if (compiler.outputDir().empty()) {
          std::cerr << "Invalid output directory ''.\n";
          usage();
        }
      } else {
        std::cerr << "Unknown option: " << arg << "\n";
        usage();
      }
    } else {
      compiler.addSourcePath(arg);
    }
  }

  if (compiler.sourcePaths().empty()) {
    std::cerr << "No input sources specified.\n";
    usage();
  }

  compiler.compile();

  if (reporter.errorCount() > 0) {
    std::cerr << reporter.errorCount() << " total errors.\n";
    return 2;
  }
  return 0;
}
