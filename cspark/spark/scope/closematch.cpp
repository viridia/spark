#include "spark/scope/closematch.h"

#if SPARK_HAVE_ALGORITHM
  #include <algorithm>
#endif

namespace spark {
namespace scope {
using spark::collections::StringRef;

void CloseMatchFinder::operator()(const StringRef& name) {
  uint32_t dist = editDistance(_target, name);
  if (dist < _distance) {
    _distance = dist;
    _closest.assign(name.begin(), name.end());
  }
}

std::size_t CloseMatchFinder::editDistance(const StringRef& s1, const StringRef& s2) {
  std::size_t s1len = s1.size();
  std::size_t s2len = s2.size();
  std::size_t column_start = 1;

  auto column = new std::size_t[s1len + 1];
  std::iota(column + column_start, column + s1len + 1, column_start);

  // TODO: This could be improved a lot.
  for (auto x = column_start; x <= s2len; x++) {
    column[0] = x;
    auto last_diagonal = x - column_start;
    for (auto y = column_start; y <= s1len; y++) {
      auto old_diagonal = column[y];
      auto possibilities = {
        column[y] + 1,
        column[y - 1] + 1,
        last_diagonal + (s1[y - 1] == s2[x - 1]? 0 : 1)
      };
      column[y] = std::min(possibilities);
      last_diagonal = old_diagonal;
    }
  }
  auto result = column[s1len];
  delete[] column;
  return result;
}

}}
