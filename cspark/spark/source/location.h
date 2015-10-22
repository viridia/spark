// ============================================================================
// location.h: Data structures for recording source line information.
// ============================================================================

#ifndef SPARK_SOURCE_LOCATION_H
#define SPARK_SOURCE_LOCATION_H 1

#include <spark/config.h>

#if SPARK_HAVE_ALGORITHM
#include <algorithm>
#endif

namespace spark {
namespace source {

class ProgramSource;

struct Location {
  ProgramSource* source;
  int32_t startLine;
  int16_t startCol;
  int32_t endLine;
  int16_t endCol;

  Location()
    : source(NULL)
    , startLine(0)
    , startCol(0)
    , endLine(0)
    , endCol(0)
  {
  }

  Location(
    ProgramSource* src,
    int32_t startLn,
    int16_t startCl,
    int32_t endLn,
    int16_t endCl)
    : source(src)
    , startLine(startLn)
    , startCol(startCl)
    , endLine(endLn)
    , endCol(endCl)
  {
  }

  Location(const Location& src)
    : source(src.source)
    , startLine(src.startLine)
    , startCol(src.startCol)
    , endLine(src.endLine)
    , endCol(src.endCol)
  {
  }

  inline friend Location operator|(const Location& left, const Location& right) {
    if (left.source != right.source) {
      if (left.source == NULL) {
        return right;
      } else {
        return left;
      }
    } else {
      Location result;
      result.source = left.source;
      if (left.startLine < right.startLine) {
        result.startLine = left.startLine;
        result.startCol = left.startCol;
      } else if (left.startLine > right.startLine) {
        result.startLine = right.startLine;
        result.startCol = right.startCol;
      } else {
        result.startLine = left.startLine;
        result.startCol = std::min(left.startCol, right.startCol);
      }
      if (left.endLine > right.endLine) {
        result.endLine = left.endLine;
        result.endCol = left.endCol;
      } else if (left.endLine < right.endLine) {
        result.endLine = right.endLine;
        result.endCol = right.endCol;
      } else {
        result.endLine = left.endLine;
        result.endCol = std::max(left.endCol, right.endCol);
      }
      return result;
    }
  }

  Location& operator|=(const Location& right) {
    *this = *this | right;
    return *this;
  }
};

}}

#endif
