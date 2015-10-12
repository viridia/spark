// ============================================================================
// Fileesystem paths - implementation.
// ============================================================================

#include "spark/support/path.h"

#if SPARK_HAVE_IOSTREAM
  #include <iostream>
#endif

namespace spark {
namespace support {

bool Path::exists() {
  ensureStat();
  return _status == ST_OK;
}

bool Path::isDir() {
  ensureStat();
  return _status == ST_OK && S_ISDIR(_mode);
}

bool Path::isFile() {
  ensureStat();
  return _status == ST_OK && S_ISREG(_mode);
}

// bool Path::isReadable() {
//   return _status == ST_OK && S_ISDIR(_mode);
// }

void Path::ensureStat() {
  if (_status == ST_UNSET) {
    struct ::stat st;
    if (::stat(_path.c_str(), &st) != 0) {
      _mode = 0;
      if (errno == ENOENT) {
        _status = ST_NOENT;
      } else {
        std::cerr << _path << ": " << ::strerror(errno) << "\n";
        _status = ST_ERROR;
      }
    } else {
      _status = ST_OK;
      _mode = st.st_mode;
    }
  }
}

PathIterator Path::iterate() const {
  return PathIterator(*this);
}

bool Path::makeRelative(const Path& base) {
  if (base._path.size() >= _path.size()) {
    return false;
  }
  size_t split = base.size();
  while (split > 0 && base._path[split - 1] == '/') {
    --split;
  }
  if (base.substr(0, split) != substr(0, split)) {
    return false;
  }
  while (split < size() && _path[split] == '/') {
    ++split;
  }
  _path.erase(0, split);
  return true;
}

bool Path::nextComponent(size_t& index, size_t& size) {
  index += size;
  while (index < _path.size() && _path[index] == '/') {
    ++index;
  }
  size = 0;
  while (index + size < _path.size() && _path[index + size] != '/') {
    ++size;
  }
  return index < _path.size();
}

bool Path::prevComponent(size_t& index, size_t& size) {
  while (index > 0 && _path[index - 1] == '/') {
    --index;
  }
  size_t end = index;
  while (index > 0 && _path[index - 1] != '/') {
    --index;
  }
  size = end - index;
  return size > 0;
}

PathIterator::PathIterator(const Path& path) {
  _dir = ::opendir(path._path.c_str());
  if (_dir == NULL) {
    int err = errno;
    std::cerr << "Unable to list contents of directory: " << path._path << "\n";
    std::cerr << ::strerror(err) << "\n";
  }
}

PathIterator::~PathIterator() {
  if (_dir) {
    ::closedir(_dir);
  }
}

bool PathIterator::next(StringRef& path) {
  if (_dir) {
    struct dirent *dp = ::readdir(_dir);
    if (dp) {
      path = &dp->d_name[0];
      return true;
    }
  }
  return false;
}
  
}}
