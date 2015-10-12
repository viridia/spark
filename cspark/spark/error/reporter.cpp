// ============================================================================
// reporter.cpp: Error reporter.
// ============================================================================

#include "spark/error/reporter.h"
#include "spark/source/programsource.h"

#if SPARK_HAVE_CASSERT
  #include <cassert>
#endif

#if SPARK_HAVE_CSIGNAL
  #include <csignal>
#endif

#if SPARK_HAVE_IOSTREAM
  #include<iostream>
#endif

#if SPARK_HAVE_STDIO_H
  #include <stdio.h>
#endif

#if SPARK_HAVE_UNISTD_H
  #include <unistd.h>
#endif

namespace spark {
namespace error {
using spark::source::Location;

#define BLACK       ";30"
#define RED         ";31"
#define GREEN       ";32"
#define YELLOW      ";33"
#define BLUE        ";34"
#define MAGENTA     ";35"
#define CYAN        ";36"
#define WHITE       ";37"

#define UNCHANGED   ""

static const char * severityNames[SEVERITY_LEVELS] = {
  "debug",
  "status",
  "info",
  "warning",
  "error",
  "fatal",
};

//   SAVEDCOLOR

MessageStream::~MessageStream() {
  flush();
  _reporter->report(_severity, _location, str());
}

void ConsoleReporter::writeSpaces(unsigned numSpaces) {
  static const char spaces[] = "                                                                ";
  while (numSpaces > sizeof(spaces) - 1) {
    std::cerr.write(spaces, sizeof(spaces) - 1);
    numSpaces -= sizeof(spaces) - 1;
  }

  std::cerr.write(spaces, numSpaces);
}

void ConsoleReporter::changeColor(StringRef color, bool bold) {
  std::cerr << "\033[" << (bold ? "1" : "0") << color << "m";
}

void ConsoleReporter::resetColor() {
  std::cerr << "\033[0m";
}

void ConsoleReporter::report(Severity sev, Location loc, StringRef msg) {
  assert(msg.size() > 0 && "Zero-length diagnostic message");

  StringRef severityName;
  _messageCountArray[(int)sev] += 1;

  bool colorChanged = false;
  #if SPARK_HAVE_UNISTD_H
    if (::isatty(STDERR_FILENO)) {
      if (sev >= ERROR) {
        changeColor(RED, true);
        colorChanged = true;
      } else if (sev == WARNING) {
        changeColor(YELLOW, true);
        colorChanged = true;
      } else if (sev == INFO) {
        changeColor(CYAN, true);
        colorChanged = true;
      } else if (sev == STATUS) {
        changeColor(CYAN, false);
        colorChanged = true;
      }
    }
  #endif

  bool showErrorLine = false;
  if (loc.source != NULL && !loc.source->path().empty()) {
    std::cerr << loc.source->path() << ":" << loc.startLine << ":" << loc.startCol << ": ";
    showErrorLine = true;
  }

  std::cerr << severityNames[(int)sev] << ": ";
  writeSpaces(_indentLevel * 2);
  std::cerr << msg;
  if (sev != STATUS) {
    std::cerr << "\n";
  }

  if (colorChanged) {
    resetColor();
  }

  if (showErrorLine) {
    #if SPARK_HAVE_UNISTD_H
      if (colorChanged) {
        changeColor(UNCHANGED, true);
      }
    #endif

    std::string line;
    if (loc.source->getLine(loc.startLine - 1, line)) {
      uint32_t beginCol = loc.startCol - 1;
      uint32_t endCol = loc.endCol - 1;
      if (loc.endLine > loc.startLine) {
        endCol = line.size();
      }
      std::cerr << line << "\n";
      #if SPARK_HAVE_UNISTD_H
        if (colorChanged) {
          resetColor();
        }
      #endif
      writeSpaces(beginCol);
      while (beginCol < endCol) {
        std::cerr << "^";
        ++beginCol;
      }
      std::cerr << "\n";
    } else if (colorChanged) {
      #if SPARK_HAVE_UNISTD_H
        resetColor();
      #endif
    }
  }


  std::cerr.flush();
  if (sev == FATAL) {
#if SPARK_HAVE_CSIGNAL
    std::raise(SIGINT);
#endif
  }
}

}}
