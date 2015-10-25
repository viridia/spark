// ============================================================================
// programsource.h: Abstract class representing a source code file.
// ============================================================================

#ifndef SPARK_SOURCE_PROGRAMSOURCE_H
#define SPARK_SOURCE_PROGRAMSOURCE_H 1

#ifndef SPARK_CONFIG_H
  #include "spark/config.h"
#endif

#if SPARK_HAVE_ISTREAM
  #include <istream>
#endif

#if SPARK_HAVE_SSTREAM
  #include <sstream>
#endif

#if SPARK_HAVE_FSTREAM
  #include <fstream>
#endif

#if SPARK_HAVE_STRING
  #include <string>
#endif

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

#ifndef SPARK_SUPPORT_PATH_H
  #include "spark/support/path.h"
#endif

namespace spark {
namespace source {
using spark::collections::StringRef;

/** Interface for reading source code. */
class ProgramSource {
public:
  /** Open the source file stream for reading. */
  virtual std::istream& open() = 0;
  
  /** Close the stream. */
  virtual void close() = 0;

  /** The path of this file, used for error reporting. */
  virtual StringRef path() const = 0;

  /** Returns true if the stream is good for reading. */
  virtual bool valid() const = 0;

  /** Read a segment from the stream (for error reporting) */
  virtual bool getLine(uint32_t index, std::string& result) = 0;
};

/** Implements shared logic for ProgramSource implementations. */
class AbstractProgramSource : public ProgramSource {
public:
  AbstractProgramSource(StringRef path) : _path(path.begin(), path.end()) {}

  StringRef path() const { return _path; }

  bool getLine(uint32_t index, std::string& result) {
    if (_lines.size() == 0) {
      readLines(_lines);
    }
    if (index <= _lines.size()) {
      result = _lines[index];
      return true;
    } else {
      result.clear();
      return false;
    }
  }
protected:
  std::vector<std::string> _lines;
  std::string _path;

  virtual void readLines(std::vector<std::string>& lines) = 0;
};

class StringSource : public AbstractProgramSource {
public:
  StringSource(StringRef path, StringRef source)
    : AbstractProgramSource(path)
    , _source(source.begin(), source.end())
    , _strm(_source)
  {}
  
  std::istream& open() { return _strm; }
  void close() {}
  bool valid() const { return true; }
  
private:
  void readLines(std::vector<std::string>& lines) {
    std::istringstream strm(_source);
    std::string line;
    while (std::getline(strm, line)) {
      _lines.push_back(line);
    }
  }
  std::string _source;
  std::istringstream _strm;
};

class FileSource : public AbstractProgramSource {
public:
  FileSource(support::Path fullPath, StringRef path)
    : AbstractProgramSource(path)
    , _fullPath(fullPath)
    , _strm(_fullPath.c_str())
  {}
  
  std::istream& open() { return _strm; }
  void close() { _strm.close(); }
  bool valid() const { return _strm.good(); }
  
private:
  void readLines(std::vector<std::string>& lines) {
    std::ifstream strm(_fullPath.c_str());
    std::string line;
    while (std::getline(strm, line)) {
      _lines.push_back(line);
    }
  }
  support::Path _fullPath;
  std::ifstream _strm;
};

}}

#endif
